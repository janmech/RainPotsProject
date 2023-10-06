# RainPots Project

A modular physical controller system for Max/RNBO Patches running on a Raspberry Pi

**¡This Project is still under creation. More files and doc umentation will be added eventually!**



Dependencies:
pip insstall pythonosc


Configure Serial Port on the Raspberry Pi:

sudo raspi-config
 ```
	3 Interface Options

	Would you like a login shell to be accessible over serial?   
	<No>

	Would you like the serial port hardware to be enabled?
	<yes>

 ```

Link scripts for easy access:
sudo ln -s Path/to/rainpots_configure.py /usr/local/bin/rainpots-config
sudo ln -sPath/to//RainPots/main.py /usr/local/bin/rainpots


SystemD servevice
cd /lib/systemd/system

rainpots.service
 ```
[Unit]
  Description=RainPots Service
  After=multi-user.target
  StartLimitIntervalSec=500
  StartLimitBurst=5
  StartLimitInterval=0
  Wants=rnbooscquery.service
  After=rnbooscquery.service
  PartOf=rnbooscquery.service

[Service]
  Type=idle
  ExecStart=/usr/bin/rainpots
  KillSignal=SIGINT
  User=pi
  Group=audio
  Restart=on-failure
  RestartSec=5s
  StandardOutput=append:/home/pi/Documents/RainPots/log/rainpots.log
  StandardError=append:/home/pi/Documents/RainPots/log/rainpots-eror.log

[Install]
  WantedBy=multi-user.target
  Alias=rainpots

 ```
sudo systemctl daemon-reload

 sudo systemctl enable (startup at boot)
 
