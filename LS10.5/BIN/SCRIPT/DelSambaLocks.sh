find /usr/local/samba/var/locks \( -type f -name '*' \) -mtime +2 -exec rm -f {} \;
