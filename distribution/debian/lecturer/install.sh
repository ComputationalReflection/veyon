#!/bin/bash
cd app
sudo dpkg -i veyon_4.3.5_amd64.deb
sudo veyon-cli config clear
sudo veyon-cli config set Authentication/Method 1
sudo veyon-cli authkeys import tpp/public key.pem
sudo veyon-cli authkeys import tpp/private private_key.pem
sudo veyon-cli config upgrade
cd ..
