#!/bin/bash

# regenerate ssh keys
sudo rm /etc/ssh/ssh_host_* -f
sudo ssh-keygen -A

# enable password connection
sudo sed -i 's/PasswordAuthentication no/PasswordAuthentication yes/' /etc/ssh/sshd_config

# restart sshd service
sudo service ssh restart

# test localhost ssh connection
rm ~/.ssh/known_hosts -f
ssh "$USER@localhost" -o StrictHostKeyChecking=no
