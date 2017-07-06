#! /bin/bash

# Change scoreboard to registration where appropriate

LOCKFILE=/tmp/polictf-scoreboard.lock
SOCKFILE=/tmp/scoreboard-fcgi-socket

if [ $EUID -ne 0 ]; then
    echo "Run this script with sudo!"
    exit 1
fi 

if [ -e "$LOCKFILE" ]; then
    echo "Killing running instance..."
    while [ -e "$LOCKFILE" ]; do
        kill $(cat /tmp/polictf-scoreboard.lock)
        sleep 3;
    done
fi

echo "Starting instance..."

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:"/usr/local/lib"
/usr/local/bin/scoreboard -c /etc/polictf/scoreboard.conf
chown www-data:root "$SOCKFILE"
chmod 0660 "$SOCKFILE"

