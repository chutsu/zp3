# Enable SSH and Connect the Raspberry Pi Zero to Wifi

1. Create empty `ssh` file in the `boot` partition.

2. Create `wpa_supplicant.conf` in the `boot` partition. The
   `wpa_supplicant.conf` should look something like this:

		country=GB
		ctrl_interface=DIR=/var/run/wpa_supplicant GROUP=netdev
		update_config=1

		network={
				ssid="your_real_wifi_ssid"
				scan_ssid=1
				psk="your_real_password"
				key_mgmt=WPA-PSK
		}

    Replace the above with your relevant details. You can obtain your Wifi's SSID
    with the command `iwlist wlan0 scan` where `wlan0` is the wifi interface.
