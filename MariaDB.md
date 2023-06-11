# MariaDB


Das [bitname GIT](https://github.com/bitnami/charts/tree/master/bitnami/mariadb/#installing-the-chart) für MariaDB beschreibt die Einzelheiten.


```
$ helm install my-mariadb -f helm_values.yaml bitnami/mariadb
NAME: my-mariadb
LAST DEPLOYED: Sun May  2 13:25:46 2021
NAMESPACE: default
STATUS: deployed
REVISION: 1
TEST SUITE: None
NOTES:
Please be patient while the chart is being deployed

Tip:

  Watch the deployment status using the command: kubectl get pods -w --namespace default -l release=my-mariadb

Services:

  echo Primary: my-mariadb.default.svc.cluster.local:3306

Administrator credentials:

  Username: root
  Password : $(kubectl get secret --namespace default my-mariadb -o jsonpath="{.data.mariadb-root-password}" | base64 --decode)

To connect to your database:

  1. Run a pod that you can use as a client:

      kubectl run my-mariadb-client --rm --tty -i --restart='Never' --image  docker.io/bitnami/mariadb:10.5.9-debian-10-r56 --namespace default --command -- bash

  2. To connect to primary service (read/write):

      mysql -h my-mariadb.default.svc.cluster.local -uroot -p my_database

To upgrade this helm chart:

  1. Obtain the password as described on the 'Administrator credentials' section and set the 'auth.rootPassword' parameter as shown below:

      ROOT_PASSWORD=$(kubectl get secret --namespace default my-mariadb -o jsonpath="{.data.mariadb-root-password}" | base64 --decode)
      helm upgrade my-mariadb bitnami/mariadb --set auth.rootPassword=$ROOT_PASSWORD
```

# PhpMyAdmin
Das [bitname GIT](https://github.com/bitnami/charts/tree/master/bitnami/phpmyadmin/#installing-the-chart) für PhpMyAdmin beschreibt die Installation.

Aus der Installation von MariaDB gehen die relevanten Daten hervor, die für die Chart-Installation in der **phpmyadm_helm_values.yaml** eingetragen werden:

```
  ## Database port
  ##
  port: 3306

  ## If you are deploying phpMyAdmin as part of a release and the database is part
  ## of the release, you can pass a suffix that will be used to find the database
  ## in releasename-dbSuffix. Please note that this setting precedes db.host
  ##
  # chartName: mariadb

  ## Database Hostname. Ignored when db.chartName is set.
  ##
  host: my-mariadb.default.svc.cluster.local
```

Zur Nutzung wird noch **INGRES** enabled:

```
## Ingress configuration
##
ingress:
  ## Set to true to enable ingress record generation
  ##
  enabled: true

  ## Set this to true in order to add the corresponding annotations for cert-manager
  ##
  certManager: false

  ## When the ingress is enabled, a host pointing to this will be created
  ##
  hostname: phpmyadmin.local

```

Die Installation ist dann per Helm wie folgt:

```
$ helm install my-phpmyadmin -f phpmyadmin_helm_values.yaml bitnami/phpmyadmin
NAME: my-phpmyadmin
LAST DEPLOYED: Sun May  2 14:50:09 2021
NAMESPACE: default
STATUS: deployed
REVISION: 1
TEST SUITE: None
NOTES:
** Please be patient while the chart is being deployed **

1. Get the application URL by running these commands:

  You should be able to access your new phpMyAdmin installation through

  http://phpmyadmin.local/

2. How to log in

phpMyAdmin has been configured to connect to a database in my-mariadb.default.svc.cluster.localwith port 3306 
Please login using a database username and password.
```

Zur Nutzung von Ingres muss der Host "phpmyadmin.local" per DNS-Auflösung erreichbar sein. (z.B.: Eintrag in die Datei /etc/hosts/) und auf die IP-Adresse des "Ingress - Service" zeigen.

Beim Zugriff muss noch der Port angegeben werden, auf den der **Ingress-Controller** hört. Z.B.: http://phpmyadmin.local:31310/
