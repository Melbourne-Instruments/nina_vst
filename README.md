# README #

The **Melbourne Instruments Nina** VST3 plugin.

## What is this repository for? ##

* Quick summary
* Version

### How do I get set up? ###

To clone the Nina VST the --recurse-submodules flag must be used:

$ git clone --recurse-submodules https://github.com/Melbourne-Instruments/nina_vst.git

The Nina VST uses CMake as its build system.

### Building ###

To setup your make environment, create a build folder (e.g. 'build') and run cmake from that folder:

$ mkdir build
$ cd build
$ cmake ../


### Building And Running Unit Tests###

create a build folder for the native (machine) build (e.g. 'build-native) and run cmake from that folder.
ex:
$ mkdir build-native
$ cd build-native
$ cmake ../ 
$ make

during the build, the unit tests will run.

To build the VST simply go into the folder created above (in the example 'build'), and run make.

Note: The ELK SDK *must* be used when building for the Raspberry Pi. It can be downloaded from here:

https://github.com/elk-audio/elkpi-sdk

It is recommended to install the SDK in the /opt folder on your Ubuntu PC.
Once this has been done, source the environment script to set-up the build environment, for example:

$ source /opt/elk/1.0/environment-setup-aarch64-elk-linux

### Contribution guidelines ###

* Writing tests
* Code review
* Other guidelines

### Tempory GSL setup Guide ###

1. clone a gsl repo: git clone https://github.com/ampl/gsl.git
2. run next steps from a root terminal (needed or make install wont copy all files)
3. run elk environment setup script
3. run: ./autogen.sh
4. run: ./configure -host=aarch64-linux --prefix=/opt/elk/1.0/sysroots/aarch64-elk-linux/usr/
5. run: make
6. run: make install
7. from a standard terminal build synthia_vst normally

---
Copyright 2020-2022 Melbourne Instruments, Australia.
