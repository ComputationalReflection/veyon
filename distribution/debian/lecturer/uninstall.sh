#!/bin/bash
sudo veyon-cli authkeys delete tpp/public
sudo veyon-cli authkeys delete tpp/private
sudo veyon-cli config clear
sudo dpkg -r veyon
