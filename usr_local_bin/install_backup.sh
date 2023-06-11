#!/bin/bash
mkdir -p /mnt/backup
echo "synology-ds920.local:/volume1/Backups/BORGREPO/$HOSTNAME /mnt/backup nfs rw,nosuid,noexec 0 0" >>/etc/fstab
mount /mnt/backup
apt-get install -y borgbackup nfs-common
echo "Borg Init:"
borg init --encryption=repokey /mnt/backup
borg key export /mnt/backup /root/$HOSTNAME-borg_backup_repokey.txt
