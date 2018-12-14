#!/bin/sh

echo "Installing python3 deps ..." \
  && \
  sudo apt-get install -y -qqq \
  python3-dev \
  python3-pip \
  libfreetype6-dev \
  libjpeg-dev \
  build-essential \
  libopenjp2-*

echo "Installing luma.oled ..." && sudo -H pip3 install --upgrade -q luma.oled
echo "Installing libtag ..." && sudo apt-get install libtag1-dev -y -qq
echo "Installing python-vlc ..." && pip3 install -q python-vlc
echo "Installing click ..." && pip3 install -q click
echo "Installing gpiozero ..." && pip3 install -q gpiozero
echo "Installing pytaglib ..." && pip3 install -q pytaglib
