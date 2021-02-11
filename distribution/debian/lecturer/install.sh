#!/bin/bash
cd app
sudo dpkg -i veyon_1.0.0_amd64.deb
sudo veyon-cli config clear
sudo veyon-cli config import default.config
sudo veyon-cli config set Authentication/Method 1
sudo veyon-cli config set Master/ConfirmUnsafeActions true
sudo veyon-cli authkeys import tpp/public key.pem
sudo veyon-cli authkeys import tpp/private private_key.pem
sudo veyon-cli config upgrade
cd ..
