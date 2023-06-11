# ZooKeeper
Kafka benötigte bis zu einer bestimmten Version noch ZooKeeper als Basis, was mich wiederum auf Zookeeper neugierig gemacht hat.

Also fasste ich den Entschluss auf meinem Kubernetes-Cluster ein Zookeeper-Ensemble zu installieren.

Es gibt mehrere Anleitungen zur Installation und Konfiguration von ZooKeeper, direkt bei [Apache](https://zookeeper.apache.org/doc/current/zookeeperStarted.html) oder bei [DigitalOcean](https://zookeeper.apache.org/doc/current/zookeeperStarted.html).
Aber auch die Standard-Ubuntu Repositories bringen ZooKeeper mit.
Die Installation per Kombination von Ansible und APT war relativ einfach.

## Ansible Vorbereitung
Neben den bisherigen Einträgen für das Kubernetes-Cluster habe ich den Eintrag **zookeeper** in **/etc/ansible/hosts** ergänzt.
Da die Rechner über avahi im MDNS bekannt gemacht werden, verzichtete ich auf Einträge in **/etc/hosts**

```
# This is the default ansible 'hosts' file.
#
# It should live in /etc/ansible/hosts
#
#   - Comments begin with the '#' character
#   - Blank lines are ignored
#   - Groups of hosts are delimited by [header] elements
#   - You can enter hostnames or ip addresses
#   - A hostname/ip can be a member of multiple groups
# If you have multiple hosts following a pattern you can specify
# them like this:

#www[001:006].example.com


lubuntu1.local
lubuntu2.local ansible_connection=local
vaio1.local
vaio2.local

[kubernetes]
lubuntu[1:2].local
vaio[1:2].local

[master]
lubuntu2.local

[slaves]
lubuntu1.local
vaio[1:2].local

[zookeeper]
lubuntu2.local
vaio[1:2].local
```


## ZooKeeper Installation

### Systemvorbereitung

Um ZooKeeper als Service starten zu können ist folgende Systemd-Unit auf **allen** Rechnern des Ensembles zu erzeugen:
/etc/systemd/system/zk.service:

```
[Unit]
Description=Zookeeper Daemon
Documentation=http://zookeeper.apache.org
Requires=network.target
After=network.target

[Service]    
Type=forking
WorkingDirectory=/usr/share/zookeeper
User=zookeeper
Group=zookeeper
ExecStart=/usr/share/zookeeper/bin/zkServer.sh start /etc/zookeeper/conf/zoo.cfg
ExecStop=/usr/share/zookeeper/bin/zkServer.sh stop /etc/zookeeper/conf/zoo.cfg
ExecReload=/usr/share/zookeeper/bin/zkServer.sh restart /etc/zookeeper/conf/zoo.cfg
TimeoutSec=30
Restart=on-failure

[Install]
WantedBy=default.target
```
### Installation

```
ansible zookeeper -a "apt-get update"
ansible zookeeper -a "apt-get install -y zookeeper"
ansible zookeeper -a "chown -R zookeeper:zookeeper /var/lib/zookeeper"
```

### Vorbereitung des Ensembles
Damit sich das Ensemble gegenseitig kennt sind folgende Einträge auf **allen** Rechnern des Ensembles in /etc/zookeeper/conf/zoo.cfg zu ergänzen.
Hier das Beispiel für die bereits genannten drei Rechner in der Ansible Host Datei.
ACHTUNG: gemäß https://stackoverflow.com/questions/30940981/zookeeper-error-cannot-open-channel-to-x-at-election-address muss auf dem jeweiligen Server der Hostname gegen die Adresse: **0.0.0.0** ersetzt werden.
Generisches Beispiel:

```
# specify all zookeeper servers
# The fist port is used by followers to connect to the leader
# The second one is used for leader election
#server.1=zookeeper1:2888:3888
#server.2=zookeeper2:2888:3888
#server.3=zookeeper3:2888:3888
server.1=lubuntu2.local:2888:3888
server.2=vaio1.local:2888:3888
server.3=vaio2.local:2888:3888

```

Aber auf server.1 des Ensembles (lubuntu2):
```
server.1=0.0.0.0:2888:3888
server.2=vaio1.local:2888:3888
server.3=vaio2.local:2888:3888
```


Desweiteren ist auf ***allen** Rechnern des Ensembles die Datei /etc/zookeeper/conf/myid mit einer eindeutigen ID (1..<Anzahl der Rechner im Ensemble>) zu beschreiben.
In meinem Beispiel:

```
# ansible zookeeper -a "cat /etc/zookeeper/conf/myid"
lubuntu2.local | SUCCESS | rc=0 >>
1

vaio1.local | SUCCESS | rc=0 >>
2

vaio2.local | SUCCESS | rc=0 >>
3
```
 
### Start des Ensembles

Mit folgenden Befehlen wird das Ensemble dann gestartet:

```
ansible zookeeper -a "systemctl daemon-reload"
ansible zookeeper -a "systemctl start zk"
```

Anmerkung: Für einen Automatischen Start nach einem Reboot:

```
ansible zookeeper -a "systemctl enable zk"
```

##Test von ZooKeeper

Gemäß dem Beispiel von Digital Ocean:

'''
root@vaio1:/etc/zookeeper/conf# sudo -u zookeeper bash
zookeeper@vaio1:/etc/zookeeper/conf_example$ cd /usr/share/zookeeper/
zookeeper@vaio1:/usr/share/zookeeper$ bin/zkCli.sh -server vaio2.local
Connecting to vaio2.local
Welcome to ZooKeeper!
JLine support is enabled
[zk: vaio2.local(CONNECTING) 0] 
WATCHER::

WatchedEvent state:SyncConnected type:None path:null

[zk: vaio2.local(CONNECTED) 0] create /zk_znode_1 sample_data
Created /zk_znode_1
[zk: vaio2.local(CONNECTED) 1] ls /
[zk_znode_1, zookeeper]
[zk: vaio2.local(CONNECTED) 2] get /zk_node_1
Node does not exist: /zk_node_1
[zk: vaio2.local(CONNECTED) 3] get /zk_znode_1
sample_data
cZxid = 0x200000002
ctime = Fri Jul 10 20:00:36 CEST 2020
mZxid = 0x200000002
mtime = Fri Jul 10 20:00:36 CEST 2020
pZxid = 0x200000002
cversion = 0
dataVersion = 0
aclVersion = 0
ephemeralOwner = 0x0
dataLength = 11
numChildren = 0
[zk: vaio2.local(CONNECTED) 4] quit
Quitting...
```

