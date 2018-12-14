all: deps tests

deps:
	@echo "Installing libtag" && sudo apt-get install libtag1-dev -y -qq
	@echo "Installing python-vlc" && pip3 install -q python-vlc
	@echo "Installing click" && pip3 install -q click
	@echo "Installing gpiozero" && pip3 install -q gpiozero
	@echo "Installing pytaglib" && pip3 install -q pytaglib

tests:
	@cd zp3 && python3 -m unittest -v -b zp3.py
