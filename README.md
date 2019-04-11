# node-red-contrib-kbus

## These nodes make it easy to read and control the Kbus on the WAGO PFC200 Gen2 controller

 A BETA open source community project to read and write simple analog, digital, and termpetature sensor modules directly to the Kbus on WAGO PFC200 Generation 2 controllers running Node-RED.

 The WAGO board support package source files are included as the Kbus driver is currently beta with no expressed or implied support.

## Install

Run the following command in the root directory of your Node-RED install

    npm install node-red-contrib-kbus

Run the following command for global install

    npm install -g node-red-contrib-kbus

## How to use

1. Navigate to Web based management on the controller, go to General Configuration, and select : None.
![General Configutation Example](http://i68.tinypic.com/2wduck4.jpg)

2. Requires [IPK FILES][2] for controller.  Install the necceasary .ipk files to the PFC by copying them to your controller, opening an SSH terminal on the PFC, and executing:

		root@PFC200V3-43089A:~ opkg install json-c_0.12.1+20160607_armhf.ipk

		root@PFC200V3-43089A:~ opkg install kbusproc_0.0.4_armhf.ipk

		root@PFC200V3-43089A:~ opkg install mosquitto_1.4.14_armhf.ipk

	After the packages are installed, you can run the kbus driver with:

		root@PFC200V3-43089A:~ kbusproc &


Add an MQTT subscribe node to your flow and sibscribe to topic : "pd_in" and wire this to your inputs.  Add an MQTT publish node with topic: "pd_out" and wire this to your outputs.  All MQTT Kbus traffic transmits over port 1883.

See the example flow:

![Flow Example](http://i66.tinypic.com/wuqcms.jpg)

## Authors

[Jesse Cox][3]

[1]:https://nodered.org
[2]:https://github.com/jessejamescox/node-red-contrib-kbus/tree/master/ipk
[3]:https://www.youtube.com/channel/UCXEwdiyGgzVDJD48f7rWOAw
