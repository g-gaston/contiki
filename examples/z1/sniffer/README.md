Contiki 802.15.4 Sniffer for Zolertia Z1
========================================

1. Compile and upload the program to the mote
    `make sniffer.upload`

2. Run the pyhton script (select proper usb) (https://github.com/g-oikonomou/sensniff):
    `python sensniff.py -d /dev/ttyUSBX`

3. Run Wireshark

4. Select the pipe in Wireshark
    Capture -> options -> Manage Interfaces -> New (under Pipes) -> type /tmp/sensniff and Save.

(The first time your run this, you will need to open Wireshark's preferences and select 'TI CC24xx FCS format' under Protocols -> IEEE 802.15.4.)
