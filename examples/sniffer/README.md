Generic Sniffer Example
=======================

This project offers a platform-independent abstraction which helps us implement
IEEE 802.15.4 sniffers in Contiki.

The firmware built with this example communicates with a host PC using
(sensniff)[]:

How to Use
==========

1. Compile and upload the program to the mote (possibles targets: cc2530dk, sensinode, z1):
    `make sniffer.upload`

2. Run the pyhton script (select proper usb) (https://github.com/g-oikonomou/sensniff):
    `python sensniff.py -d /dev/ttyUSBX`

3. Run Wireshark

4. Select the pipe in Wireshark
    Capture -> options -> Manage Interfaces -> New (under Pipes) -> type /tmp/sensniff and Save.

(The first time your run this, you will need to open Wireshark's preferences and select 'TI CC24xx FCS format' under Protocols -> IEEE 802.15.4.)

Platforms
=========

In order to make this example work for a platform, the following steps need to
be undertaken:
 * sniffer_arch_get_rf_channel() and sniffer_arch_rf_set_channel() must be implemented
 * The platform's RF driver must configure the hardware to disable frame
   filtering.

The following RF drivers are known to work:
 * cc2530
 * cc2430
 * cc2420

There may be other ones working but not listed here.

Configurable Output
===================
By default, the project will output captured frames over UART1.

To change this behaviour: ...
