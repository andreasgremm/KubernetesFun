# Ubuntu Server
## Initiale Instalation
Installiere Ubuntu Server 20.04.x LTS auf dem Zielrechner.

Zusätzlich das SNAP-Package für **docker**  installieren.

## Zeitzone
Im System muss noch die richtige Zeitzone eingesellt werden.

```
timedatectl set-timezone Europe/Berlin
```

## Finalisierung
Abschliessend ein Full-Upgrade durchführen.

```
apt update
apt full-upgrade -y
```

Nach fertiggestellter Installation für die private Nutzung ohne installierten DNS-Server noch den **avahi-daemon** nachinstallieren.
Bei Laptops ist es günstig in der Datei **/etc/systemd/logind.conf** den Eintrag ***HandleLidSwitch*** auf "ignore" zu setzen. Damit kann der Deckel geschlossen werden und das System läuft weiter.

## weitere Vorbereitungen 
### SSH Verbindung 
Damit eine schnelle SSH Verbindung möglich ist, muss der Eintrag für den SSH-Key in /root/.ssh/authorized_keys hinterlegt werden.

Hierzu auf dem steuernden Rechner:

```
ssh-copy-id -i ~/.ssh/<private key> <user>@<neuer Server>
```

Folgende Dateien für den neuen Rechner anpassen

* ~/.ssh/config

### Ansible Verbindung 
Folgende Dateien für den neuen Rechner anpassen

* /etc/ansible/hosts 

Auf dem steuernden Rechner muss der Public-SSH-Key des Benutzers **root** zu der Datei ~/.ssh/authorized_keys auf dem Zielrechner hinzugefügt werden.

```
scp ~/.ssh/<key.pub> <user>@<neuer Server>:/tmp/xx
```

Auf dem Zielrechner:

```
# Login ...
sudo cat /tmp/xx >>/root/.ssh/authorized_keys
rm -f /tmp/xx
```

Testen auf dem Steuer-Rechner:

```
ansible <neuer Server> -a "lsmem"
```

