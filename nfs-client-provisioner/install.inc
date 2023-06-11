###
#
# https://github.com/kubernetes-sigs/nfs-subdir-external-provisioner
#
#  Please set the right values
export NFSServer=<Hostname/IP of NFS Server>
export NFSPath=<Exported Path of NFS Server>

helm repo add nfs-subdir-external-provisioner https://kubernetes-sigs.github.io/nfs-subdir-external-provisioner/
helm install nfs-subdir-external-provisioner nfs-subdir-external-provisioner/nfs-subdir-external-provisioner \
    --set nfs.server=$NFSServer \
    --set nfs.path=$NFSPath

