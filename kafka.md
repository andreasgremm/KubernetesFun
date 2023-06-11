# Kafka Standalone ....

[Hier](https://www.digitalocean.com/community/tutorials/how-to-install-apache-kafka-on-ubuntu-20-04) befindet sich die Installationsanleitung für Kafka und KafkaT.

Und etwas "Futter" zur [internen Architektur](https://www.instaclustr.com/apache-kafka-architecture/).

KafkaT ist ein Tool zur einfacheren Verwaltung von Kafka.

```
kafka@vaio1:~$ kafkat
[DEPRECATION] The trollop gem has been renamed to optimist and will no longer be supported. Please switch to optimist as soon as possible.
/var/lib/gems/2.7.0/gems/json-1.8.6/lib/json/common.rb:155: warning: Using the last argument as keyword parameters is deprecated
kafkat 0.3.0: Simplified command-line administration for Kafka brokers
usage: kafkat [command] [options]

Here's a list of supported commands:

  brokers                                                             Print available brokers from Zookeeper.
  clean-indexes                                                       Delete untruncated Kafka log indexes from the filesystem.
  cluster-restart help                                                Determine the server restart sequence for kafka
  controller                                                          Print the current controller.
  drain <broker id> [--topic <t>] [--brokers <ids>]                   Reassign partitions from a specific broker to destination brokers.
  elect-leaders [topic]                                               Begin election of the preferred leaders.
  partitions [topic]                                                  Print partitions by topic.
  partitions [topic] --under-replicated                               Print partitions by topic (only under-replicated).
  partitions [topic] --unavailable                                    Print partitions by topic (only unavailable).
  reassign [topics] [--brokers <ids>] [--replicas <n>]                Begin reassignment of partitions.
  resign-rewrite <broker id>                                          Forcibly rewrite leaderships to exclude a broker.
  resign-rewrite <broker id> --force                                  Same as above but proceed if there are no available ISRs.
  set-replication-factor [topic] [--newrf <n>] [--brokers id[,id]]    Set the replication factor of
  shutdown <broker id>                                                Gracefully remove leaderships from a broker (requires JMX).
  topics                                                              Print all topics.
  verify-replicas  [--topics] [--broker <id>] [--print-details] [--print-summary]Check if all partitions in a topic have same number of replicas.

```

# Beispiele

## Producer

```
echo "Hello, World" | ~/kafka/bin/kafka-console-producer.sh --broker-list localhost:9092 --topic TutorialTopic > /dev/null
```


## Consumer

```
/home/kafka/kafka/bin/kafka-console-consumer.sh --bootstrap-server localhost:9092 --topic TutorialTopic --from-beginning --property "print.timestamp=true" --property "print.key=true"
```

## Python Producer & Consumer
Das Python Modul [kafka-python](https://kafka-python.readthedocs.io/en/master/usage.html) lässt keine Wünsche zur Nutzung von Kafka aus Python übrig.

Requirements.txt:

```
rc32c==2.2
kafka-python==2.0.2
lz4==3.1.3
```

# Error Handling
Kafka scheint beim Starten neue Cluster ID's zu generieren:
[Fehlerbeseitigung](https://stackoverflow.com/questions/59592518/kafka-broker-doesnt-find-cluster-id-and-creates-new-one-after-docker-restart/59832914#59832914)

# Kafka auf Kubernetes

[Hier](https://betterprogramming.pub/how-to-run-highly-available-kafka-on-kubernetes-a1824db8a3e2) findet sich eine Beschreibung zur Implementierung von Kafka auf Kubernetes.

# Kafka on Kubernetes per Helm
[Hier](https://bitnami.com/stack/kafka/helm) gibt es weitere Informationen zu dem
[kafka helm chart](https://github.com/bitnami/charts/tree/master/bitnami/kafka/#installing-the-chart).

```
andreas@lubuntu2:~$ helm install my-kafka --set replicaCount=2,global.storageClass=<storage class> bitnami/kafka 
NAME: my-kafka
LAST DEPLOYED: Fri Apr  2 10:37:22 2021
NAMESPACE: default
STATUS: deployed
REVISION: 1
TEST SUITE: None
NOTES:
** Please be patient while the chart is being deployed **

Kafka can be accessed by consumers via port 9092 on the following DNS name from within your cluster:

    my-kafka.default.svc.cluster.local

Each Kafka broker can be accessed by producers via port 9092 on the following DNS name(s) from within your cluster:

    my-kafka-0.my-kafka-headless.default.svc.cluster.local:9092
    my-kafka-1.my-kafka-headless.default.svc.cluster.local:9092

To create a pod that you can use as a Kafka client run the following commands:

    kubectl run my-kafka-client --restart='Never' --image docker.io/bitnami/kafka:2.7.0-debian-10-r68 --namespace default --command -- sleep infinity
    kubectl exec --tty -i my-kafka-client --namespace default -- bash

    PRODUCER:
        kafka-console-producer.sh \
            --broker-list my-kafka-0.my-kafka-headless.default.svc.cluster.local:9092,my-kafka-1.my-kafka-headless.default.svc.cluster.local:9092 \
            --topic test

    CONSUMER:
        kafka-console-consumer.sh \
            --bootstrap-server my-kafka.default.svc.cluster.local:9092 \
            --topic test \
            --from-beginning
```

Kafka Consumer, weitere Beispiele:

```
kafka-console-consumer.sh --bootstrap-server my-kafka.default.svc.cluster.local:9092 --topic test --offset '2' --partition 0
```


Kafka Management kann im Kafka-Client erfolgen (Pfad: /opt/bitnami/kafka/bin). Beispiele:

```
kafka-topics.sh --bootstrap-server my-kafka-0.my-kafka-headless.default.svc.cluster.local:9092 --list
kafka-topics.sh --bootstrap-server my-kafka-0.my-kafka-headless.default.svc.cluster.local:9092 --describe test
kafka-topics.sh --bootstrap-server my-kafka-0.my-kafka-headless.default.svc.cluster.local:9092 --create --replication-factor 1 --partitions 1 --topic TutorialTopic
```
