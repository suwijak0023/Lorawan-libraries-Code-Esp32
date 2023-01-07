*Original from Rakwireless Repository. Thank for sharing this very useful script.

This script will install and config ChirpStack Server on Raspbian. Test with Raspberry Pi 4B

sudo apt update

sudo apt upgrade

git clone https://github.com/m2mlorawan/ChirpStack_on_Raspbian

cd ChirpStack_on_Raspbian/

chmod +x install.sh

sudo ./install.sh

sudo reboot

Start using http://"Your PI ip":8080
User: admin Password: admin

