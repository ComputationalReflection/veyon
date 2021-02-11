#!/bin/bash
cd app
sudo dpkg -i veyon_1.1.0_amd64_NO_MASTER.deb
sudo veyon-cli config clear
sudo veyon-cli config import default.config
sudo veyon-cli config set Service/Autostart false
sudo veyon-cli config set Authentication/Method 1
sudo veyon-cli config set Master/ConfirmUnsafeActions true
sudo veyon-cli authkeys import tpp/public key.pem
sudo veyon-cli config upgrade
sudo veyon-cli service restart
sudo veyon-cli service status
cd ..
