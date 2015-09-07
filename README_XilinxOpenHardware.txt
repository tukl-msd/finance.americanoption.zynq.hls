Team number:	XIL-83437
Project name:	Pushing the Limits in Financial Acceleration: Multidimensional American Option Pricing on Xilinx Zynq
Date:			30/06/2015
Version of uploaded archive: 1

University name:	University of Kaiserslautern
Supervisor name:	Prof. Norbert Wehn
Supervisor e-mail:	wehn@eit.uni-kl.de
Participant(s):		Javier Alejandro Varela
Email:				varela@eit.uni-kl.de

Board used:			ZC702 
Vivado Version (preferably 2014.4): 2014.3
Brief description of project:
In the financial industry, pricing Multidimensional American Options is a computationally intensive task, where MonteCarlo simulation is required.
The interest behind options resides in two key factors. First, they are one of the most traded derivative products in the derivatives market, 
which has an annual volume worldwide of U$D 700 trillion. Second, they are normally part of trading portfolios to hedge risk.
FPGAs provide the adequate means to obtain efficient acceleration in terms of bandwidth and energy.
Furthermore, hybrid CPU/FPGA devices like the Xilinx Zynq provide an excellent platform to create self-contained acceleration systems.
This work shows the benefits of the Xilinx Zynq against CPU implementations by pricing a two-dimensional American max option (both call and put).
Runtime and energy consumption is given and compared, as well as a numerical comparison against a golden solution.

Description of archive (explain directory structure, documents and source files):
Folders: 
Ccode_and_bitstream: bitstream and drive C code to run in linux
HLS: contains all code and testbenches for each custom IP block (the projects had to be deleted due to size constraints)
IP:  contains all custom IP block's .zip files (to use in a Vivado Project)
Report: 
SDCard: contains BOOT.bin, uimage, devicetree.dtb files.
Vivado: contains the vivado project. Due to size constraints, the implementated design had to be removed (only Design and Synthesis remain).



// ----------------------------------------------------------------------------------------------------
Instructions to build and test project
Step 1:
prepare the SDCard following the link: http://www.wiki.xilinx.com/Prepare+Boot+Medium

Step 2: 
copy the files located in the folder SDCard into the SDCard/boot

Step 3:
Download the latest linaro server version from: https://releases.linaro.org/
(preferably: linaro-raring-server-20131021-529.tar)

Step 4:
Mount the SDCard. In console, execute the following two lines:
1) tar --strip-components=1 -C /media/root -xzpf <path-to-image-file>/linaro-raring-server-<version>.tar.gz
2) sync
(Umount SDCard)

Step 5:
Insert the SDCard in Zynq.
Connect the USB cable to USB-UART port and the external PC.
Connect the board to Ethernet.
Plug the power connection and switch on the Zynq board.

Step 6:
run putty (or similar) and connect via serial connection (usually COM3/4 @ 115200 bauds.

Step 7:
Once the Zynq board responds, type the following:
1) env default -a
2) (this is OPTIONAL) setenv ethaddr ....<new MAC address>
3) saveenv
4) run sdboot
wait until system boots ...
5) ip link
take note of the ethernet interface ethX (usually eth0)
6) vi /etc/network/interfaces 
and insert the following two lines:
  auto ethX
  iface ethX inet dhcp
7) ifup ethX
set date:
8) ntpdate
edit file
9) vi /etc/apt/sources.list
delete the given entries and replace them with 
   deb http://old-releases.ubuntu.com/ubuntu/ raring main universe
   deb-src http://old-releases.ubuntu.com/ubuntu/ raring main universe

Step 8:
update system:
1) apt-get update
2) apt-get dist-upgrade
and install some useful tools:
3) apt-get install ntp ssh vim git build-essential screen
set root password:
4) passwd

Step 9:
make a directory of your choice and copy all files located in project folder: Ccode_and_bitstream
as a suggestion, use WinSCP for such task.

Step 10:
access the new folder in console, and type:
1) source ./run.sh
The main file is called main.cpp
It will compile the source code, run the main.exe file.
main.exe will configure the FPGA, run some tests, price the option in FPGA and via two golden versions in software.
At the end it will display numerical results, as well as runtime and percentages.
It is also possible to run the configuration several times for power measurements.

// ----------------------------------------------------------------------------------------------------


Link to YouTube Video(s):
https://youtu.be/EVGVIQayf1o