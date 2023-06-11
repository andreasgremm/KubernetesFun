# Minio
Ein Rechner wird mit ausreichend Festplatten ausgestattet und dient dann als [MinIO](https://min.io/resources/docs/CPG-MinIO-implementation-guide.pdf) Server.

Die Festplatten gemäß obiger Anleitung partitionieren.
ACHTUNG: Beim Festplattentausch auf das Partition Label achten!!!
Nach dem Tausch: mc admin heal -r TARGET

```
$for i in a b c d e; do parted /dev/sd$i print; done

Model: ATA ST3200822A (scsi)
Disk /dev/sda: 200GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End    Size   File system  Name         Flags
 1      1049kB  200GB  200GB  xfs          minio-data3

Model: ATA Hitachi HCS5C103 (scsi)
Disk /dev/sdb: 320GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End    Size   File system  Name         Flags
 1      1049kB  320GB  320GB  xfs          minio-data1

Model: ATA SAMSUNG SP1614C (scsi)
Disk /dev/sdc: 160GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End    Size   File system  Name         Flags
 1      1049kB  160GB  160GB  xfs          minio-data2

Model: ATA SAMSUNG HD103UJ (scsi)
Disk /dev/sdd: 1000GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system     Name         Flags
 1      1049kB  8591MB  8590MB  linux-swap(v1)               swap
 2      8591MB  331GB   322GB   ext4
 3      331GB   438GB   107GB   ext4
 4      438GB   1000GB  562GB   xfs             minio-data4

Model: ATA OCZ-VERTEX2 (scsi)
Disk /dev/sde: 60,0GB
Sector size (logical/physical): 512B/512B
Partition Table: gpt
Disk Flags: 

Number  Start   End     Size    File system  Name  Flags
 1      1049kB  2097kB  1049kB                     bios_grub
 2      2097kB  60,0GB  60,0GB  ext4

```

**Mountpunkte erzeugen:**

```
cd /mnt
for i in 1 2 3 4; do mkdir minio-data$i; done

mount /dev/sdb1 /mnt/minio-data1
mount /dev/sdc1 /mnt/minio-data2
mount /dev/sda1 /mnt/minio-data3
mount /dev/sdd4 /mnt/minio-data4
```

**/etc/fstab vorbereiten:**

```
blkid | grep UUID= | grep xfs
```

Beispielausgabe:

```
/dev/sda1: UUID="a91cd8dd-821e-4a14-bfcf-923f7164ebd5" TYPE="xfs" PARTLABEL="minio-data3" PARTUUID="72629793-697f-4911-9f09-829a8115cbaf"
/dev/sdb1: UUID="f5c33a72-0626-48ec-a69a-11c38fda8028" TYPE="xfs" PARTLABEL="minio-data1" PARTUUID="1622b617-9903-445d-b63d-480c82957e49"
/dev/sdd4: UUID="d3888c1d-54b9-4c77-85e0-493a1c6eaaf7" TYPE="xfs" PARTLABEL="minio-data4" PARTUUID="90e3b22e-372b-4bfc-9df1-3e682f21aeae"
/dev/sdc1: UUID="e88f0c04-d790-4c52-908f-4fc9af9ec99b" TYPE="xfs" PARTLABEL="minio-data2" PARTUUID="249bafb5-e603-5449-97f2-4361f3aefcd8"

# Beispiel für /etc/fstab auf Basis des obigen Ergebnis
# MinIO Disks
/dev/disk/by-uuid/f5c33a72-0626-48ec-a69a-11c38fda8028 /mnt/minio-data1 xfs defaults 0 0
/dev/disk/by-uuid/e88f0c04-d790-4c52-908f-4fc9af9ec99b /mnt/minio-data2 xfs defaults 0 0
/dev/disk/by-uuid/a91cd8dd-821e-4a14-bfcf-923f7164ebd5 /mnt/minio-data3 xfs defaults 0 0
/dev/disk/by-uuid/d3888c1d-54b9-4c77-85e0-493a1c6eaaf7 /mnt/minio-data4 xfs defaults 0 0
```

**MinIO installieren:**

```
export startuser=<user>

mkdir minio
cd minio
wget https://dl.minio.io/server/minio/release/linux-amd64/minio 
chmod +x minio

chown -R $startuser /mnt/minio-data1 /mnt/minio-data2 /mnt/minio-data3 /mnt/minio-data4
chmod u+rxw /mnt/minio-data1 /mnt/minio-data2 /mnt/minio-data3 /mnt/minio-data4
```

## Minio starten und testen
**Startup-Files erzeugen**

```
export MINIO_ACCESS_KEY=<access key>
export MINIO_SECRET_KEY=<secret key>
/home/andreas/minio/minio server http://chieftec.local/mnt/minio-data{1...4}
```

**MinIO starten:**

```
andreas@chieftec:~/minio$ ./start_minio.bash 
Formatting 1st pool, 1 set(s), 4 drives per set.
WARNING: Host chieftec.local has more than 2 drives of set. A host failure will result in data becoming unavailable.
Attempting encryption of all config, IAM users and policies on MinIO backend
Status:         4 Online, 0 Offline. 
Endpoint: http://192.168.1.20:9000  http://172.17.0.1:9000  http://127.0.0.1:9000     
RootUser: <access key> 
RootPass: <secret key>

Browser Access:
   http://192.168.1.20:9000  http://172.17.0.1:9000  http://127.0.0.1:9000    

Command-line Access: https://docs.min.io/docs/minio-client-quickstart-guide
   $ mc alias set myminio http://192.168.1.20:9000 <access key> <secret key>

Object API (Amazon S3 compatible):
   Go:         https://docs.min.io/docs/golang-client-quickstart-guide
   Java:       https://docs.min.io/docs/java-client-quickstart-guide
   Python:     https://docs.min.io/docs/python-client-quickstart-guide
   JavaScript: https://docs.min.io/docs/javascript-client-quickstart-guide
   .NET:       https://docs.min.io/docs/dotnet-client-quickstart-guide
IAM initialization complete
```

## Minio als Service implementieren

Diese [Installationsanleitung](https://github.com/minio/minio-service/tree/master/linux-systemd) gibt generell Auskunft wie MinIO als Service implementiert werden kann.

```
export MINIO_ACCESS_KEY=<minio root user>
export MINIO_SECRET_KEY=<minio root password>

cp /home/andreas/minio/minio /usr/local/bin/minio

cat <<EOT >> /etc/default/minio
# Volume to be used for MinIO server.
MINIO_VOLUMES="http://chieftec.local/mnt/minio-data{1...4}"
# Use if you want to run MinIO on a custom port.
# MINIO_OPTS="--address :9199"
# Root user for the server.
MINIO_ROOT_USER=$MINIO_ACCESS_KEY
# Root secret for the server.
MINIO_ROOT_PASSWORD=$MINIO_SECRET_KEY

EOT
```

## MinIO Client

**MinIO Client installieren:**
Die Installationsanleitung ist im [Quickstart Guide](https://docs.min.io/docs/minio-client-quickstart-guide) zu finden.

```
wget https://dl.min.io/client/mc/release/linux-amd64/mc
chmod +x mc
./mc --help
```
Nach dem ersten Start mit **mc ls** erfolgt folgende Ausgabe:

```
~/minio$ mc ls
mc: Configuration written to `/home/andreas/.mc/config.json`. Please update your access credentials.
mc: Successfully created `/home/andreas/.mc/share`.
mc: Initialized share uploads `/home/andreas/.mc/share/uploads.json` file.
mc: Initialized share downloads `/home/andreas/.mc/share/downloads.json` file.
```
In der Datei config.json müssen die Infos zum Access- und Secret Key hinterlegt werden.

Beispiele: S3 Name ist **local** laut **/home/andreas/.mc/config.json**

```
mc admin info local
mc admin user add local andreas
mc admin policy set local readwrite user=andreas

```

## MinIO File-System mounten
Die Beschreibung liegt [hier](https://github.com/s3fs-fuse/s3fs-fuse).

```
sudo ansible kubernetes -a "apt install s3fs -y"
sudo ansible kubernetes -a "mkdir /mnt/s3bucket"
sudo ansible kubernetes -a "chown <user:group> /mnt/s3bucket"

cat <Access Key>:<Secret Key> ~/.passwd_s3fs
chmod 0600 ~/.passwd_s3fs

s3fs <bucketname> /mnt/s3bucket -o url=http://<minio hostname>:9000/ -o passwd_file=.passwd_s3fs -o use_path_request_style
```

**Alternative mount beim booten** in /etc/fstab:

```
# mybucket /path/to/mountpoint fuse.s3fs _netdev,allow_other,use_path_request_style,url=https://url.to.s3/ 0 0

<bucketname> /mnt/s3bucket fuse.s3fs _netdev,allow_other,use_path_request_style,url=http://<minio hostname>:9000/ 0 0
```

Note: You may also want to create the global credential file first

```
echo ACCESS_KEY_ID:SECRET_ACCESS_KEY > /etc/passwd-s3fs
chmod 600 /etc/passwd-s3fs
```

## MinIO Operator in Kubernetes installieren
Die Installation und Nutzung des MinIO Operators in Kubernetes wird [hier](https://github.com/minio/operator) beschreiben.

```
kubectl krew install minio

# Ausgabe
Updated the local copy of plugin index.
Installing plugin: minio
Installed plugin: minio
\
 | Use this plugin:
 | 	kubectl minio
 | Documentation:
 | 	https://github.com/minio/operator/tree/master/kubectl-minio
 | Caveats:
 | \
 |  | * For resources that are not in default namespace, currently you must
 |  |   specify -n/--namespace explicitly (the current namespace setting is not
 |  |   yet used).
 | /
/
WARNING: You installed plugin "minio" from the krew-index plugin repository.
   These plugins are not audited for security by the Krew maintainers.
   Run them at your own risk.
```

**MinIO initialisieren**:

```
kubectl minio init
```

**Operator-Konsole starten**:

```
kubectl minio proxy -n minio-operator
```

