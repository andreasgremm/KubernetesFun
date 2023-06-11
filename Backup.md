# Backup mit Borg
Borg ist ein Backup-Programm mit dem Daten-Deduplizierung erreicht wird.

Die Beschreibung befindet sich [hier](https://borgbackup.readthedocs.io/en/stable/quickstart.html).

## Verzeichnis für Backup erstellen
Auf einem Dateiserver muss ein Pfad freigegeben werden, dieser wird per NFS/SMB o.ä. auf dem Client gemountet.

Beispieleintrag in /etc/fstab:

```
synology-ds920.local:/volume1/Backups/BORGREPO/<hostname> /mnt/backup nfs rw,nosuid,noexec 0 0
```

## Borg Nutzung

Beispiel für die Nutzung von Borg über nfs (nfs-common = nfs client):

```bash
#!/bin/bash
mkdir -p /mnt/backup
echo "synology-ds920.local:/volume1/Backups/BORGREPO/$HOSTNAME /mnt/backup nfs rw,nosuid,noexec 0 0" >>/etc/fstab
mount /mnt/backup
apt-get install -y borgbackup nfs-common
echo "Borg Init:"
borg init --encryption=repokey /mnt/backup
borg key export /mnt/backup /root/borg_backup_repokey.txt
```


## Automatisierte Backups
Die Automation von Backups wird [hier](https://borgbackup.readthedocs.io/en/stable/quickstart.html#automating-backups) beschrieben.

```sh
#!/bin/sh

backup_name=Borg_Backup-$HOSTNAME_$(date +"%Y-%m-%d_%H-%M")

mount /mnt/backup
if [ ! -d "/mnt/backup/data" ]; then
  echo "Backup Directory not found"
  umount /mnt/backup
  exit
fi

if [ ! -d "/mnt/backup/log" ]; then
  echo "Backup Log Directory not found .. creating"
  mkdir /mnt/backup/log
fi

# Setting this, so the repo does not need to be given on the commandline:
export BORG_REPO=/mnt/backup

# See the section "Passphrase notes" for more infos.
export BORG_PASSCOMMAND='gpg --decrypt /root/.borg.gpg'
# export BORG_PASSPHRASE='passphrase'

# some helpers and error handling:
info() { printf "\n%s %s\n\n" "$( date )" "$*" >&2; }
trap 'echo $( date ) Backup interrupted >&2; exit 2' INT TERM

info "Starting backup"

# Backup the most important directories into an archive named after
# the machine this script is currently running on:

borg create                         \
    --verbose                       \
    --filter AME                    \
    --list                          \
    --stats                         \
    --show-rc                       \
    --compression zstd,9            \
    --exclude-caches                \
    --exclude '/home/*/.cache/*'    \
    --exclude '/var/tmp/*'          \
    --exclude '/var/lib/docker/*'   \
                                    \
    ::'{hostname}-{now}'            \
    /etc                            \
    /home                           \
    /root                           \
    /var                            \
    /usr/local/etc                  \
    /usr/local/bin                  \
    2>/mnt/backup/log/$backup_name.log
    

backup_exit=$?

info "Pruning repository"

# Use the `prune` subcommand to maintain 7 daily, 4 weekly and 6 monthly
# archives of THIS machine. The '{hostname}-' prefix is very important to
# limit prune's operation to this machine's archives and not apply to
# other machines' archives also:

borg prune                          \
    --list                          \
    --prefix '{hostname}-'          \
    --show-rc                       \
    --keep-daily    7               \
    --keep-weekly   4               \
    --keep-monthly  6               \
    2>>/mnt/backup/log/$backup_name.log

prune_exit=$?

# actually free repo disk space by compacting segments

info "Compacting repository"

# Compact erst ab Version 1.2.x verfügbar
# borg compact                      \
#    2>>/mnt/backup/log/$backup_name.log

compact_exit=0

# use highest exit code as global exit code
global_exit=$(( backup_exit > prune_exit ? backup_exit : prune_exit ))
global_exit=$(( compact_exit > global_exit ? compact_exit : global_exit ))

if [ ${global_exit} -eq 0 ]; then
    info "Backup, Prune, and Compact finished successfully"
elif [ ${global_exit} -eq 1 ]; then
    info "Backup, Prune, and/or Compact finished with warnings"
else
    info "Backup, Prune, and/or Compact finished with errors"
fi

find /mnt/backup/log  -mtime +30 -delete
umount /mnt/backup

exit ${global_exit}
```

### GPG für automatisierte Backups
Die automatisierung im Batch wird [hier](https://www.gnupg.org/documentation/manuals/gnupg/Unattended-GPG-key-generation.html) beschrieben.

* gpg --gen-key  # no passphrase!
* vi /root/.bashrc | /root/.profile
	* export GPGKEY='<gpgkey>'
* echo <Borg Backup Passwort> | gpg --encrypt -o /root/.borg.gpg ----recipient $name$(used at key generation) --always-trust


