@echo off
"%programfiles%\Veyon\veyon-cli.exe" authkeys delete tpp/public
"%programfiles%\Veyon\veyon-cli.exe" authkeys delete tpp/private
"%programfiles%\Veyon\uninstall.exe" /ClearConfig /S