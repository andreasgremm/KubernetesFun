# KubernetesFun - K8S Fun
Some Tips and Hints / Experiences to use a Kubernetes (K8S) Cluster on xUbuntu

[kubectl Spickzettel](https://kubernetes.io/de/docs/reference/kubectl/cheatsheet/)

## Kubernetes Installation
Für die Kubernetes-Installation nutzte ich ausgemusterte Desktops und Laptops. Der Kubernetes Cluster wird am Ende 4 Knoten beinhalten, ein Master-Node und 3 normale Nodes.

Name: lubuntu1
Node:  Ubuntu 18.04.3 LTS
IP: 192.168.1.235
RAM: 2048 MB
CPU: Pentium 4 CPU 3.06 GHz: 1 Proc, 1 Core, 2 threads

Name: vaio1
Node:  Ubuntu 18.04.3 LTS
IP: 192.168.1.234
RAM: 4000 MB
CPU: Core i3-2350M CPU 2.30GHz: 4 Proc

Name: vaio2
Node:  Ubuntu 18.04.3 LTS
IP: 192.168.1.233
RAM: 4000 MB
CPU: Core i3-2350M CPU 2.30GHz: 4 Proc

Name: lubuntu2
Master: Ubuntu 18.04.3 LTS
IP: 192.168.1.236
RAM: 2577MB
CPU: Pentium D CPU, 2,80 GHz: 1 Proc, 2 cores, 2 threads

Diese [Installationsanleitung](https://vitux.com/install-and-deploy-kubernetes-on-ubuntu/) ist sehr zielgerichtet und besteht aus wenigen Schritten. 
Eine weitere [Installationsanleitung auf Kubernetes.io](https://kubernetes.io/docs/setup/production-environment/tools/kubeadm/install-kubeadm/) gibt weitere Informationen.


Mittels **sudo** folgendende Befehle auf allen Knoten ausführen.
Als Basis muss Docker installiert werden und parallel installieren wir das Hilfsprogramm ***curl***.

### Installation von Docker
Mit Einführung von docker.ce ist diese [Installationsanleitung (Beispiel Ubuntu)](https://docs.docker.com/install/linux/docker-ce/ubuntu/) zu beachten.

Installation von docker.io aus dem Standard-Repository:

```
apt-get update
apt-get install docker.io curl apt-transport-https openssh-server -y
docker --version
systemctl enable docker
```

### Installation von Kubernetes
Basierend auf Docker wir Kubernetes auf allen Knoten installiert. xUbuntu 18.04.x ist der Bionic Path, Kubernetes ist dort nicht verfügbar. Daher wird das Repository von Xenial genutzt. Hierfür wird das Repository auf den Systemen hinzugefügt (mittels **sudo**).

```
curl -s https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key add
apt-add-repository "deb https://apt.kubernetes.io/ kubernetes-xenial main"

apt-get update
apt-get install kubeadm -y
kubeadm version
```

### Aktivierung von Kubernetes
Swapping sollte für Kubernetes auf allen Nodes deaktiviert werden. Hierfür in **/etc/fstab** einen eventuell vorhandenen **swapfile** Eintrag auskommentieren ```#/swapfile none swap sw 0 0``` und den Befehl ```sudo swapoff -a``` ausführen.
Alle Rechner sollten eindeutige Hostnamen haben, dieser kann mit dem Befehl ```sudo hostnamectl set-hostname <hostname>``` gesetzt werden.

#### Master Node initialisieren

Auf dem Master Node wird Kubernetes initialisiert, die Ausgabe des Befehls beschreibt wie weitere Clusterknoten hinzugefügt werden können. 

Hier ist insbesondere das Zertifikat wichtig. Dieses Zertifikat läuft nach 24 Stunden aus. Für das Hinzufügen weiterer Knoten muss dann ein neues Zertifikat erzeugt werden. 

Als Pod-Netzwerk soll ***flannel*** verwendet werden, dafür wird der notwendige Parameter wie im folgenden Kommando gesetzt.

```
sudo kubeadm init --pod-network-cidr=10.244.0.0/16
```
Die Ausgabe von **kubeadm** bitte beachten und den Join-Befehl für weitere Knoten sicherheitshalber für die weitere Verwendung in einer Datei speichern.

Um den Cluster mit Hilfe des **kubectl** Programms nicht nur als Root-User managen zu können, muss die Konfiguration für den Benutzer zugänglich gemacht werden.

```
mkdir -p $HOME/.kube
sudo cp -i /etc/kubernetes/admin.conf $HOME/.kube/config
sudo chown $(id -u):$(id -g) $HOME/.kube/config
```

Jetzt kann das Pod-Netzwerk ([flannel](https://github.com/flannel-io/flannel) installiert werden:

```
kubectl apply -f https://raw.githubusercontent.com/coreos/flannel/master/Documentation/kube-flannel.yml
```

**Hinweis:** Es gab bei LTS 18.04.x und Kubernetes V1.16 ein Problem mit der YML-Datei. Eventuell muss der Parameter "cniVersion": "0.3.0" gesetzt sein. (In der beschriebenen Datei steht er aktuell auf 0.2.0). Im Zweifel die YML Datei herunterladen, ändern und ein neues **kubectl apply** mit der lokalen Datei durchführen. Danach die Knoten rebooten.


#### Slave Node hinzufügen
Um einen Slave Node hinzuzufügen müssen ebnefalls die entsprechenden Anwendungen installiert werden.

```
apt-get install apt-transport-https openssh-server curl -y
apt-get install docker.io -y
docker --version | tee /home/andreas/Dokumente/docker-version.txt

systemctl enable docker

curl -s https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key add
apt-add-repository "deb http://apt.kubernetes.io/ kubernetes-xenial main"

apt-get update
apt-get install kubeadm -y
kubeadm version | tee /home/andreas/Dokumente/kubeadm-version.txt

swapoff -a
```

Mittels **kubeadm** wird ein Cluster-Knoten (Slave) hinzugefügt. 
Das notwendige Join-Kommando wird bei der Initialisierung des Masters ausgegeben.

```
sudo kubeadm join <master-node>:6443 --token <token> --discovery-token-ca-cert-hash sha256:<discovery token> 
```

##### Join Token neu erzeugen
Wenn ein Slave-Node später als 24 Stunden hinzugefügt werden soll, muss auf dem Master ein neuer Token erzeugt werden.

```
sudo kubeadm token create --print-join-command
```

## Cluster upgraden
Auch wenn die Programme kubeadm, kubectl, kubelet etc. durch die Betriebssystemupgrades auf neuere Versionen gebracht werden, wird der Cluster nicht automatisch auf eine neue Version gehoben.

**ACHTUNG:** Um zu verhindern, dass die `kube` Kommandos automatisch durch Updates auf höhere Versionen gehoben werden und dann die Version der Clusterplane nicht mehr passt, sind diese möglichst zu locken. Dieses ist beispielsweise mit `apt-mark hold <package>` in Debian basierten Systemen möglich.

Der Cluster-Upgrade wird mit dem Programm **kubeadm** durchgeführt. Eine [Dokumentation](https://kubernetes.io/docs/tasks/administer-cluster/kubeadm/kubeadm-upgrade/) findet sich auf den Kubernetes-Seiten.

Im wesentlichen sind es die Kommandos (sudo):

```
kubeadm upgrade plan
kubeadm upgrade apply <version>
```

Es kann passieren, dass der Cluster veraltet und die letzte Version von **kubeadm** diese Control-Plane Version nicht updaten kann.
Beispiel: Cluster auf Version 1.14.1 und kubeadm auf Version 1.16.1.

In dem Fall muss **kubeadm** im Betriebssystem "downgegraded" werden und nach dem Update wieder auf die letzte Version gehoben werden.
Welche Versionen verfügbar sind erfährt man in Debian basierten Systemen über:

```
apt-cache policy kubeadm
```

In der Regel sollten die Versionen der Programme mit den Versionen des Clusters übereinstimmen. Eine Version 1.x kann im Regelfall den Cluster in der Version 1.(x-1) upgraden.

Für ein kontrolliertes Upgrade müssen die Programme kubeadm und kubelet in der richtigen Version vorliegen. Daher sollten diese von den normalen Update-Mechanismen ausgenommen werden. 
Dieses geschient mit dem "Hold" Mechanismus von apt.

Mittels ```apt-mark showhold``` können die gehaltenen Versionen angezeigt werden.

Eine spezifische Version kann mit ```apt-get install <package name>=<version>``` installiert werden.
Dieser Befehl führt ein UPGRADE aus, wenn das Package bereits installiert ist.
Danach muss das Package wieder auf hold gesetzt werden.

* Master-Nodes: ```apt-mark hold kubeadm```, ```apt-mark hold kubelet```
* Slave-Nodes: ```apt-mark hold kubelet```


# SSL Certificate checking
Auf [dieser](https://www.shellhacks.com/openssl-check-ssl-certificate-expiration-date/) Webseite werden diverse Zertifikat-Tests beschrieben (Date, Issuer ...).

```
openssl x509 -in /etc/kubernetes/pki/apiserver.crt -noout -text |grep ' Not '
```
## Fehlerbehandlung Zertifikate

Test der Zertifikate mit **kubeadm**

```
kubeadm certs check-expiration
```

Skript zum checken von Zertifikaten in einem Verzeichnis:

```
shopt -s nullglob
for f in *.0 *.pem *.key *.crt; do
    echo "## CERTIFICATE NAME -> $f ##"
     openssl x509 -noout -in $f   -issuer -subject -dates  
    echo ""
done
```

Test des Zertifikats des **kubelet**

```
echo | openssl s_client -showcerts --connect 192.168.1.234:10250 -servername kubelet 2>/dev/null | openssl x509 -noout -enddate
```

Im Verzeichnis /var/lib/kubelet/pki liegen Dateien **kubelet.crt** und **kubelet.key**.
Diese werden offensichtlich trotz editierter /etc/kubeadm/kubelet.conf angezogen und durch den Metrics-Server genutzt. Diese Dateien können gelöscht werden, beim restart des kubelet `systemctl restart kubelet` werden sie wieder erzeugt.

Test des Zertifikats des **api servers**

```
echo | openssl s_client -showcerts --connect 127.0.0.1:6443 -servername api 2>/dev/null | openssl x509 -noout -enddate
```


# HELM Installation
[Helm](https://helm.sh/docs/intro/install/) ist ein Installations-Tool für Anwendungen in K8S. Die Installationsanweisung wird in Helm **Charts** genannt. Die Helm-Charts liegen in Repositories.

Für meine Installation habe ich **snap** als Installationsbasis ausgesucht: 

```
apt install snap -y
snap install helm --classic
```

# NFS File Provisioner
Um in der K8S Installation auf NFS als persistenter Dateispeicher zurückgreifen zu können, gibt es mehrere Möglichkeiten.

* Mounten der NFS File-Systeme auf jeden K8S-Knoten
* Installation eines **Provisioners** um Physical Volume Claims (PVC) zu bedienen und Physical Volumes (PV) bereitzustellen. 

Die zweite Option lässt sich über den [nfs-subdir-external-provisioner](https://github.com/kubernetes-sigs/nfs-subdir-external-provisioner) realisieren.

Die Installation über Helm ist schnell gemacht:

```
helm repo add nfs-subdir-external-provisioner https://kubernetes-sigs.github.io/nfs-subdir-external-provisioner/
helm install nfs-subdir-external-provisioner nfs-subdir-external-provisioner/nfs-subdir-external-provisioner \
    --set nfs.server=x.x.x.x \
    --set nfs.path=/exported/path
```

In der aktuellen Version des Helm-Charts: nfs-subdir-external-provisioner-4.0.6 gibt es zwei Probleme.

1. der Provisioner muss **cluster.local/nfs-subdir-external-provisioner** lauten (die PVC's werden auf diesen Provision geclaimed).
2. die Cluster-Rolle **nfs-subdir-external-provisioner-runner** muss um Node-Berechtigungen [erweitert werden](https://github.com/kubernetes-sigs/nfs-subdir-external-provisioner/issues/85).

# Busybox in einem Pod starten.
Mit kubectl kann auch ein interaktiver Container gestartet werden, anhand von Busybox:

`kubectl run -i --tty busybox --image=busybox --restart=Never -- sh`

# KREW installieren
Krew ist ein Plugin-Manager für **kubectl**. [Hier](https://krew.sigs.k8s.io/docs/user-guide/setup/install/) gibt es die Installationsanleitung.

```
$ (
>   set -x; cd "$(mktemp -d)" &&
>   OS="$(uname | tr '[:upper:]' '[:lower:]')" &&
>   ARCH="$(uname -m | sed -e 's/x86_64/amd64/' -e 's/\(arm\)\(64\)\?.*/\1\2/' -e 's/aarch64$/arm64/')" &&
>   curl -fsSLO "https://github.com/kubernetes-sigs/krew/releases/latest/download/krew.tar.gz" &&
>   tar zxvf krew.tar.gz &&
>   KREW=./krew-"${OS}_${ARCH}" &&
>   "$KREW" install krew
> )

## Ausgabe:
++ mktemp -d
+ cd /tmp/tmp.O68ryWzwKH
++ uname
++ tr '[:upper:]' '[:lower:]'
+ OS=linux
++ sed -e s/x86_64/amd64/ -e 's/\(arm\)\(64\)\?.*/\1\2/' -e 's/aarch64$/arm64/'
++ uname -m
+ ARCH=amd64
+ curl -fsSLO https://github.com/kubernetes-sigs/krew/releases/latest/download/krew.tar.gz
+ tar zxvf krew.tar.gz
./LICENSE
./krew-darwin_amd64
./krew-darwin_arm64
./krew-linux_amd64
./krew-linux_arm
./krew-linux_arm64
./krew-windows_amd64.exe
+ KREW=./krew-linux_amd64
+ ./krew-linux_amd64 install krew
Adding "default" plugin index from https://github.com/kubernetes-sigs/krew-index.git.
Updated the local copy of plugin index.
Installing plugin: krew
Installed plugin: krew
\
 | Use this plugin:
 | 	kubectl krew
 | Documentation:
 | 	https://krew.sigs.k8s.io/
 | Caveats:
 | \
 |  | krew is now installed! To start using kubectl plugins, you need to add
 |  | krew's installation directory to your PATH:
 |  | 
 |  |   * macOS/Linux:
 |  |     - Add the following to your ~/.bashrc or ~/.zshrc:
 |  |         export PATH="${KREW_ROOT:-$HOME/.krew}/bin:$PATH"
 |  |     - Restart your shell.
 |  | 
 |  |   * Windows: Add %USERPROFILE%\.krew\bin to your PATH environment variable
 |  | 
 |  | To list krew commands and to get help, run:
 |  |   $ kubectl krew
 |  | For a full list of available plugins, run:
 |  |   $ kubectl krew search
 |  | 
 |  | You can find documentation at
 |  |   https://krew.sigs.k8s.io/docs/user-guide/quickstart/.
 | /
/

```

Interessante Krew-Module sind unter anderem:

* access-matrix
* ctx (Cluster Context ändern)
* ns (Namespace ändern)

# Ingress Controller
## Native Installation Nginx

Die besten Erfahrungen habe ich mit der nativen Ingress-Nginx [Installation](https://github.com/kubernetes/ingress-nginx/blob/master/docs/deploy/index.md) gemacht. 

## Helm Intallation Nginx
Der [Ingress Controller](https://docs.nginx.com/nginx-ingress-controller/installation/installation-with-helm/) ermöglicht den Zugang zu Services von ausserhalb des Clusters.

```
helm install ingress bitnami/nginx-ingress-controller
NAME: ingress
LAST DEPLOYED: Wed Apr  7 23:11:20 2021
NAMESPACE: default
STATUS: deployed
REVISION: 1
TEST SUITE: None
NOTES:
** Please be patient while the chart is being deployed **

The nginx-ingress controller has been installed.

Get the application URL by running these commands:

 NOTE: It may take a few minutes for the LoadBalancer IP to be available.
        You can watch its status by running 'kubectl get --namespace default svc -w ingress-nginx-ingress-controller'

    export SERVICE_IP=$(kubectl get svc --namespace default ingress-nginx-ingress-controller -o jsonpath='{.status.loadBalancer.ingress[0].ip}')
    echo "Visit http://${SERVICE_IP} to access your application via HTTP."
    echo "Visit https://${SERVICE_IP} to access your application via HTTPS."

An example Ingress that makes use of the controller:

  apiVersion: extensions/v1beta1
  kind: Ingress
  metadata:
    annotations:
      kubernetes.io/ingress.class: nginx
    name: example
    namespace: foo
  spec:
    rules:
      - host: www.example.com
        http:
          paths:
            - backend:
                serviceName: exampleService
                port: 80
              path: /
    # This section is only required if TLS is to be enabled for the Ingress
    tls:
        - hosts:
            - www.example.com
          secretName: example-tls

If TLS is enabled for the Ingress, a Secret containing the certificate and key must also be provided:

  apiVersion: v1
  kind: Secret
  metadata:
    name: example-tls
    namespace: foo
  data:
    tls.crt: <base64 encoded cert>
    tls.key: <base64 encoded key>
  type: kubernetes.io/tls
```

In dem "Bare-Metal-Kubernetes" Cluster kann **MetalLB** als Software-Loadbalancer implementiert werden.

# MetalLB
MetalLB ist ein Load-Balancer für Bare-Metal Kubernetes Installationen.
Die Installation von MetalLB wird [hier](https://metallb.universe.tf/installation/) beschrieben.

Mittels **helm** ist die Installation relativ simpel.

```
# helm repo add bitnami https://charts.bitnami.com/bitnami
# oder helm repo update
helm install metallb bitnami/metallb

## Ausgabe:
NAME: metallb
LAST DEPLOYED: Wed Apr  7 21:26:24 2021
NAMESPACE: default
STATUS: deployed
REVISION: 1
TEST SUITE: None
NOTES:
MetalLB is now running in the cluster

LoadBalancer Services in your cluster are now available on the IPs you
defined in MetalLB's configuration. To see IP assignments,

    kubectl get services -o wide --all-namespaces | grep --color=never -E 'LoadBalancer|NAMESPACE'

should be executed.

To see the currently configured configuration for metallb run

    kubectl get configmaps --namespace default metallb-config -o yaml

in your preferred shell.

```

Anschliessend muss noch die Config-Map angepasst werden, anbei ein Beispiel:

```
apiVersion: v1
data:
  config: |
    address-pools:
    - name: default
      protocol: layer2
      addresses:
      - 192.168.1.233-192.168.1.236
```


