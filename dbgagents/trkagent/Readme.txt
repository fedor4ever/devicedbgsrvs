//
// Trk
//

Trk provides target side application level debug support for Carbide C++
It comes preconfigured for certain reference platforms already supported by Symbian.
It also comes preconfigured for S60 and UIQ platforms. 

In order to build for different platforms, please follow the steps below:

1) Choose the appropriate UI platform folder, for ex: if you are working techview, then choose tv folder.
If you are working with S60, then choose s60 folder.
If you are working a text shell environment, then choose tshell folder.

2) Choose the type of TRK build, there are two options. One is OEM build which builds System TRK
and the other one is isv build for building Application TRK. 
For ex: If you want to build System TRK for S60 3.2 platform, 
then goto trk\group\s60 and run 
	bldmake bldfiles
	abld build
For Application TRK build, goto trk\group\s60\isv_build and run the same build commands as above.

3) You can either include TRK in rom or package TRK in a sis file.
For including trk in the rom image, please include trk.iby file 
into one of the iby files that are used to build the rom.
For packaging trk in a sis file, please goto the sis folder and run the appropriate batch file.


NOTE: Please note that group2 folder is in development and use this only
if you are working with TRK that uses new debug API
