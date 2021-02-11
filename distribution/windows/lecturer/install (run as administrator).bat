@echo off
cd %~dp0
cd app
veyon.exe /S /D=%programfiles%\Veyon
"%programfiles%\Veyon\veyon-cli.exe" config clear
"%programfiles%\Veyon\veyon-cli.exe" config import default.config
"%programfiles%\Veyon\veyon-cli.exe" config set Authentication/Method 1
"%programfiles%\Veyon\veyon-cli.exe" config set Master/ConfirmUnsafeActions true
"%programfiles%\Veyon\veyon-cli.exe" authkeys import tpp/public key.pem
"%programfiles%\Veyon\veyon-cli.exe" authkeys import tpp/private private_key.pem
"%programfiles%\Veyon\veyon-cli.exe" config upgrade
cd..
pause