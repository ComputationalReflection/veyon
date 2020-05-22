@echo off
cd %~dp0
cd app/vpn
vpncmd.exe /server 156.35.95.36 /password:e9Ep!SH5Zu*5 /in:get_tables.txt /out:temp.txt
type temp.txt | findstr /v "VPN Server" | findstr /v "SoftEther" | findstr /v "Compiled" | findstr /v "Version" | findstr /v "command" | findstr /v "SoftEther" > ../../input.txt
cd..
cd..
python parse.py
"%ProgramFiles%\Veyon\veyon-cli.exe" networkobjects clear
"%ProgramFiles%\Veyon\veyon-cli.exe" networkobjects import labs.txt format "%%name%%;%%host%%;%%location%%"
del input.txt