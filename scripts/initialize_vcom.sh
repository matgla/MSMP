#!/bin/bash
CURRENT_USER=${USER}
echo "${CURRENT_USER}"
sudo touch $1
sudo touch $2
sudo socat PTY,link=$1 PTY,link=$2 &
sleep 1
sudo stty -F $1 -onlcr -echo raw && sudo stty -F $2 -onlcr -echo -raw
sudo chown "${CURRENT_USER}" $1 && sudo chown "${CURRENT_USER}" $2