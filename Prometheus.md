# Prometheus
Die Prometheus Dokumentation findet sich [hier](https://prometheus.io/docs/introduction/overview/).

Die Installation wurde über **snap** bei der Installation des Betriebssystems (Ubuntu Server) gleich mit erledigt.

Die Konfigurationsdatei ist: `/var/snap/prometheus/32/prometheus.yml`
und enthält initial:

```
# my global config
global:
  scrape_interval:     15s # Set the scrape interval to every 15 seconds. Default is every 1 minute.
  evaluation_interval: 15s # Evaluate rules every 15 seconds. The default is every 1 minute.
  # scrape_timeout is set to the global default (10s).

# Alertmanager configuration
alerting:
  alertmanagers:
  - static_configs:
    - targets:
      # - alertmanager:9093

# Load rules once and periodically evaluate them according to the global 'evaluation_interval'.
rule_files:
  # - "first_rules.yml"
  # - "second_rules.yml"

# A scrape configuration containing exactly one endpoint to scrape:
# Here it's Prometheus itself.
scrape_configs:
  # The job name is added as a label `job=<job_name>` to any timeseries scraped from this config.
  - job_name: 'prometheus'

    # metrics_path defaults to '/metrics'
    # scheme defaults to 'http'.

    static_configs:
    - targets: ['localhost:9090']

```

## Node Exporter
Unter Ubuntu 20.x kann der Node Exporter per Standard installiert werden `apt install prometheus-node-exporter`

Hierbei wird der Service `prometheus-node-exporter.service`eingerichtet.

Mittels `ss -l` lässt sich überprüfen ob der Node-Exporter auf Port **9100** läuft und mittels `curl http://localhost:9100/metrics` können die Metriken abgerufen werden.

Die Konfigurationsdatei wird ergänzt um:

```
- job_name: 'node'
  static_configs:
  - targets: ['<hostname/ip>:9100', '<otherhost/otherip>:9100', ....]
```

**AKTUELL**: Die Namensauflösung geht **NICHT** per mDNS. Targets müssen per resolv.conf auflösbar sein.


Beispielkonfiguration: `--config.file=/var/snap/prometheus/32/prometheus.yml`

```
global:
  scrape_interval: 15s
  scrape_timeout: 10s
  evaluation_interval: 15s
alerting:
  alertmanagers:
  - static_configs:
    - targets: []
    scheme: http
    timeout: 10s
    api_version: v1
rule_files:
- /var/snap/prometheus/32/node_rules.yaml
- /var/snap/prometheus/32/node_alerts.yaml
scrape_configs:
- job_name: prometheus
  honor_timestamps: true
  scrape_interval: 15s
  scrape_timeout: 10s
  metrics_path: /metrics
  scheme: http
  static_configs:
  - targets:
    - latitude.local:9090
- job_name: node
  honor_timestamps: true
  scrape_interval: 15s
  scrape_timeout: 10s
  metrics_path: /metrics
  scheme: http
  static_configs:
  - targets:
    - latitude.local:9100
    - vaio3.local:9100
```

## Grafana Anbindung
Die Anbindung an Grafana ist [hier](https://grafana.com/oss/prometheus/exporters/node-exporter/) beschrieben.

# Apache Monitoring
##### Auch für den Apache Web Server gibt es einen [Prometheus Exporter](https://github.com/Lusitaniae/apache_exporter). Eine Installationsbeschreibung findet sich [hier](https://computingforgeeks.com/how-to-monitor-apache-web-server-with-prometheus-and-grafana-in-5-minutes/).

Der Promotheus Apache Exporter nutzt das Status Modul des Apache. Eine Beispielkonfiguration mit Benutzervalidierung wenn die Anfrage NICHT vom lokalen Rechner erfolgt ist:

```
<IfModule mod_status.c>
	ExtendedStatus On
	<Location /server-status/>
	    SetHandler server-status
	    Include conf.d/auth_admin.cnf
       <RequireAny>
	      Require valid-user
          Require local
       </RequireAny>
	</Location>
</IfModule>
```

Die gesammelten Installationsanweisungen:

```
mkdir $HOME/Prometheus_Apache_Exporter
cd $HOME/Prometheus_Apache_Exporter

curl -s https://api.github.com/repos/Lusitaniae/apache_exporter/releases/latest   | grep browser_download_url   | grep linux-amd64 | cut -d '"' -f 4 | wget -qi -

tar xvf apache_exporter-*.linux-amd64.tar.gz
sudo cp apache_exporter-*.linux-amd64/apache_exporter /usr/local/bin
sudo chmod +x /usr/local/bin/apache_exporter

sudo groupadd --system prometheus
sudo useradd -s /sbin/nologin --system -g prometheus prometheus

cat <<EOF >/etc/systemd/system/apache_exporter.service
[Unit]
Description=Prometheus
Documentation=https://github.com/Lusitaniae/apache_exporter
Wants=network-online.target
After=network-online.target

[Service]
Type=simple
User=prometheus
Group=prometheus
ExecReload=/bin/kill -HUP $MAINPID
ExecStart=/usr/local/bin/apache_exporter \
  --insecure \
  --scrape_uri=http://localhost/server-status/?auto \
  --telemetry.address=0.0.0.0:9117 \
  --telemetry.endpoint=/metrics

SyslogIdentifier=apache_exporter
Restart=always

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl start apache_exporter.service
sudo systemctl enable apache_exporter.service

sudo ss -tunelp | grep 9117
curl http://localhost:9117/metrics

```


#SNMP Gateway
Der Test wird mit einer Synology DS215j durchgeführt. Gemäß der Synology SNMP Einstellung kann folgendes SNMPWALK genutzt werden:

```
snmpwalk -v 3 -u <username> -a MD5 -A <passwort> -l authNoPriv synology-ds215j.local
```
Die Generation des snmp.yml wird [hier](https://github.com/prometheus/snmp_exporter/tree/main/generator) beschreiben. Voraussetzung ist die Installation von **go**.

```
sudo apt-get install unzip build-essential libsnmp-dev p7zip-full # Debian-based distros
apt install -y gccgo-go

go get github.com/prometheus/snmp_exporter/generator

cd $HOME/go/src/github.com/prometheus/snmp_exporter/generator/
go build
make mibs
```

In der Datei **generator.yml** im Verzeichnis kann beispielsweise für das Synology-Modul die Authentifizierung wie in der obigen Beschreibung für den Generator eingestellt werden. Beispiel:

```
  synology:
    walk:
      [...]

    version: 3
    auth:
            username: <username>
            security_level: authNoPriv
            password: <passwort>
            auth_protocol: MD5

```

Danach die Datei **snmp.yml** generieren:

```
export MIBDIRS=mibs
./generator generate
```

Die erzeugte **snmp.yml** Datei wird für die Konfiguration des snmp_exporters benötigt.

Die Installation des SNMP-Exporters wird [hier](https://github.com/prometheus/snmp_exporter) beschriben.

Nach der Installation und Konfiguration kann der SNMP-Exporter mit folgendem Beispielaufruf getestet werden:

```
curl "latitude.local:9116/snmp?target=synology-ds215j.local&module=synology"
```

Die prometheus.yml Datei wird um die Scrape-Config ergänzt:

```
 - job_name: 'snmp'
    static_configs:
      - targets:
        - synology-ds215j.local  # SNMP device.
    metrics_path: /snmp
    params:
      module: ['synology']
    relabel_configs:
      - source_labels: [__address__]
        target_label: __param_target
      - source_labels: [hostname]
        target_label: instance
      - target_label: __address__
        replacement: latitude.local:9116  # The SNMP exporter's real hostname:port.
```

## Kubernetes Metrics Server
Der [Kubernetes Metrics Server](https://github.com/kubernetes-sigs/metrics-server) ermöglicht die Anbindung der Kubernetes Metriken an Prometheus.

Bei Self-Signed-Certificates im Cluster muss der Metric-Server mit einem spezifischen Parameter `--kubelet-insecure-tls` gestartet werden.

Hierzu das **deployment** editieren oder die ursprüngliche **yaml** datei vor dem apply per `wget` herunterladen und editieren.

```
[...]
      - args:
        - --cert-dir=/tmp
        - --secure-port=4443
        - --kubelet-preferred-address-types=InternalIP,ExternalIP,Hostname
        - --kubelet-use-node-status-port
        - --kubelet-insecure-tls
 [...]
```

### Verifizierung des Metric-Servers

Durch `kubectl top pod|node` kann die Funktion des Metric-Servers getestet werden.

```
andreas@lubuntu2:~/Dokumente/k8sfiles/yaml/metrics-server$ kubectl top pod
NAME                                               CPU(cores)   MEMORY(bytes)   
my-kafka-0                                         19m          406Mi           
my-kafka-1                                         19m          406Mi           
my-kafka-zookeeper-0                               9m           184Mi           
my-nginx-5d998f947f-ddbkj                          0m           7Mi             
my-nginx-5d998f947f-kqzvs                          0m           4Mi             
nfs-subdir-external-provisioner-74577d4bcc-kshm6   4m           7Mi             
andreas@lubuntu2:~/Dokumente/k8sfiles/yaml/metrics-server$ kubectl top node
NAME       CPU(cores)   CPU%   MEMORY(bytes)   MEMORY%   
lubuntu1   97m          4%     798Mi           42%       
lubuntu2   321m         16%    1505Mi          63%       
vaio1      178m         4%     1614Mi          43%       
vaio2      174m         4%     1234Mi          32%       
andreas@lubuntu2:~/Dokumente/k8sfiles/yaml/metrics-server$ 

```

## Kube-State-Metrics
[Hier](https://sysdig.com/blog/kubernetes-monitoring-prometheus/#monitoringkubernetesclusterwithprometheusandkubestatemetrics) befindet sich die Beschreibung.

```
git clone https://github.com/kubernetes/kube-state-metrics.git
cd kube-state-metrics
kubectl apply -f examples/standard
```

# TextFile Collector Smartmon Fehler

In der Lubuntu Distribution tritt ein Fehler bei der Interpretation von Floating Point Werten in der Smartmon.prom auf.

Die Korrektur erfolgt durch den folgenden zusätzlichen Eintrag im systemd unit file prometheus-node-exporter-smartmon.service:

```
Environment=LC_NUMERIC=C
```
