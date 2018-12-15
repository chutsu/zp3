#!/bin/sh
set -e  # Exit on first error
APT_INSTALL='sudo apt-get install -y -qqq'
PIP_INSTALL='sudo pip3 install -q'

echo "Installing python3 deps ..." && $APT_INSTALL python3-dev python3-pip
echo "Installing ttf-mscorefonts ..." && $APT_INSTALL ttf-mscorefonts-installer 
echo "Installing libfreetype ..." && $APT_INSTALL libfreetype6-dev 
echo "Installing libjpeg-dev ..." && $APT_INSTALL libjpeg-dev 
echo "Installing libopenjp2 ..." && $APT_INSTALL libopenjp2-*
echo "Installing build essentials ..." && $APT_INSTALL build-essential 
echo "Installing luma.oled ..." && sudo -H pip3 install --upgrade -q luma.oled
echo "Installing libtag ..." && $APT_INSTALL libtag1-dev
echo "Installing python-vlc ..." && $PIP_INSTALL python-vlc
echo "Installing click ..." && $PIP_INSTALL click
echo "Installing gpiozero ..." && $PIP_INSTALL gpiozero
echo "Installing pytaglib ..." && $PIP_INSTALL pytaglib
