#!/bin/bash
cd app/vpn
./vpncmd /server 156.35.95.36 /password:'e9Ep!SH5Zu*5' /in:get_tables.txt /out:temp.txt
cat temp.txt | grep -v "VPN Server" | grep -v "SoftEther" | grep -v "Compiled" | grep -v "Version" | grep -v "command" | grep -v "Virtual Hub" > ../../input.txt
cd ..
cd ..
python3 parse.py
sudo veyon-cli networkobjects clear
sudo veyon-cli networkobjects import labs.txt format "%name%,%host%,%location%"
rm input.txt
