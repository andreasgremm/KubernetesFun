apt-get install apt-transport-https openssh-server curl -y
apt-get install docker.io -y
docker --version | tee /tmp/docker-version.txt

systemctl enable docker

curl -s https://packages.cloud.google.com/apt/doc/apt-key.gpg | apt-key add
apt-add-repository "deb http://apt.kubernetes.io/ kubernetes-xenial main"

apt-get update
apt-get install kubeadm -y
kubeadm version | tee /tmp/kubeadm-version.txt

swapoff -a

