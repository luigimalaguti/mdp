#!/bin/bash

set -e

mkdir -p /run/sshd
mkdir -p "/home/$USERNAME/.ssh"
echo "$SSH_PUBLIC_KEY" > "/home/$USERNAME/.ssh/authorized_keys"
chown -R "$USERNAME:$USERNAME" "/home/$USERNAME/.ssh"
chmod 700 "/home/$USERNAME/.ssh"
chmod 600 "/home/$USERNAME/.ssh/authorized_keys"

exec /usr/sbin/sshd -D
