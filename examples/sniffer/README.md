Generic Sniffer Example
=======================

This project offers a platform-independent abstraction which helps us implement
IEEE 802.15.4 sniffers in Contiki.

The firmware built with this example communicates with a host PC using
(sensniff)[]:

How to Use
==========
Build this example for your hardware and upload it to your device.
On your PC, follow the instructions in the sensniff page.

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

There may be other ones working but not listed here.

Configurable Output
===================
By default, the project will output captured frames over UART1.

To change this behaviour: ...
