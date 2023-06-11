###
#
# https://github.com/bitnami/charts/tree/master/bitnami/kafka
#
export STORAGEClass=managed-nfs-storage
# helm repo add bitnami https://charts.bitnami.com/bitnami
helm install my-kafka --set replicaCount=2,global.storageClass=$STORAGEClass bitnami/kafka

