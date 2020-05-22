@echo off
cd %~dp0
cd app
veyon.exe /S /NoMaster /D=%programfiles%\Veyon
"%programfiles%\Veyon\veyon-cli.exe" config clear
"%programfiles%\Veyon\veyon-cli.exe" config set Service/Autostart false
"%programfiles%\Veyon\veyon-cli.exe" config set Authentication/Method 1
"%programfiles%\Veyon\veyon-cli.exe" authkeys import tpp/public key.pem
"%programfiles%\Veyon\veyon-cli.exe" config upgrade
"%programfiles%\Veyon\veyon-cli.exe" service restart
"%programfiles%\Veyon\veyon-cli.exe" service status

pause