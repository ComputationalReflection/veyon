#!/bin/bash
cd app
sudo dpkg -i veyon_4.3.5_amd64_NO_MASTER.deb
sudo veyon-cli config clear
sudo veyon-cli config set Service/Autostart false
sudo veyon-cli config set Authentication/Method 1
sudo veyon-cli authkeys import tpp/public key.pem
sudo veyon-cli config upgrade
sudo veyon-cli service restart
sudo veyon-cli service status
cd ..
