#!/bin/bash
sudo veyon-cli authkeys delete tpp/public
sudo veyon-cli config clear
sudo dpkg -r veyon
