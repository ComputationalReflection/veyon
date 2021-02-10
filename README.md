# Veyon Fork to comply with the EU general data protection regulation and record remote lab sessions 

[![Latest stable release](https://img.shields.io/github/v/release/ComputationalReflection/veyon.svg?maxAge=3600)](https://github.com/ComputationalReflection/veyon/releases)
[![license](https://img.shields.io/badge/license-GPLv2-green.svg)](LICENSE)

## What is Veyon?

[Veyon (Virtual Eye On Networks)](https://veyon.io/) is a free and open-source software for computer monitoring and classroom
management supporting Windows and Linux. It enables instructors to view and control
computer labs and interact with students. Veyon is available in many different
languages and provides numerous features supporting instructors and administrators
at their daily work:

  * Overview: monitor all computers in one or multiple locations or classrooms
  * Remote access: view or control computers to watch and support users
  * Demo: broadcast the instructor's screen in realtime (fullscreen/window)
  * Screen lock: draw attention to what matters right now
  * Communication: send text messages to students
  * Start and end lessons: log in and log out users all at once
  * Screenshots: record learning progress and document infringements
  * Programs & websites: launch programs and open website URLs remotely
  * Teaching material: distribute and open documents, images and videos easily
  * Administration: power on/off and reboot computers remotely
  

## What is in this Veyon fork?

This fork of the [Veyon project](https://github.com/veyon/veyon) provides modifications to comply with the GDPR (EU regulation 2016/679). It also provides video recording of all the students attending the lab. Alternatively, it can store a sequence of screenshots instead of a video. To do this, a new "record" button has been added to the toolbar.

## Installation and configuration

Installation and configuration are done the same way as in the original project. Please, refer to the official [Veyon Administrator Manual](https://docs.veyon.io/en/latest/admin/index.html) for information about the installation and configuration of Veyon.

Additionally, this Veyon version is provided with two sets of [Windows and Linux scripts](https://github.com/ComputationalReflection/veyon/tree/master/distribution) that make it very easy for students and lecturers to (un)install, configure, start and stop the system.

## Usage

Please refer to the official [Veyon User Manual](https://docs.veyon.io/en/latest/user/index.html) for information about how to use Veyon.

### General Data Protection Regulation

This version of Veyon includes modifications regarding students' privacy concerns. First of all, configuration scripts enable access control policies to inform students when a remote user (instructor) is connected to their computer. It also shows a dialog where students are explicitly informed about the Veyon features (Remote View, Remote Control and Video Recording) related to their privacy and instructions on how to stop desktop sharing. Finally, it asks the students for explicit consent when an instructor wants to  control their computer or record their activity.

![Record Button](record_button.png)
This picture must be updated

### Video Recording Usage

This version also includes video recording as part of its functionality. When the record button of the toolbar is clicked, the system starts recording one video per lab attendant. By clicking the button again, the recording stops.
 
![Record Button](record_button.png)

It is possible to specify different video parameters, modifying the VeyonMaster.json configuration file.
* `video` (default true): recrods video (true) or sequence of screenshots (false).
* `Height` (default 720): frame height in pixels.
* `Width` (default 1280): frame width in pixels.
* `CaptureIntervalNum` and `CaptureIntervalDen` (default 1000/1000): set interval time (in millis) between consecutive frames for screenshots and video recording. By default 1 second (1000).
* `SavePath` (default %APPDATA%/Record): path where the video or screenshots are saved.

Full Example Configuration 

```shell
...
 "Plugin.Record": {
        "Video": true,
        "CaptureIntervalDen": 1000,
        "CaptureIntervalNum": 1000,
        "Height": 720,        
        "Width": 1280,
	"SavePath": "%APPDATA%/Record"
    }
...
```

## Veyon on Linux

### Downloading the sources

First, grab the latest sources by cloning the Git repository and fetching all submodules:

	git clone --recursive https://github.com/ComputationalReflection/veyon.git && cd veyon


### Installing the dependencies

Requirements for Debian-based distributions:

- Build tools: g++ make cmake
- Qt5: qtbase5-dev qtbase5-dev-tools qttools5-dev qttools5-dev-tools
- X11: xorg-dev libxtst-dev
- libjpeg: libjpeg-dev provided by libjpeg-turbo8-dev or libjpeg62-turbo-dev
- zlib: zlib1g-dev
- OpenSSL: libssl-dev
- PAM: libpam0g-dev
- procps: libprocps-dev
- LZO: liblzo2-dev
- QCA: libqca2-dev libqca-qt5-2-dev
- LDAP: libldap2-dev
- SASL: libsasl2-dev
- FFmpeg: libavcodec-dev libavformat-dev libswscale-dev

As root you can run

	apt install g++ make cmake qtbase5-dev qtbase5-dev-tools qttools5-dev qttools5-dev-tools \
	            xorg-dev libxtst-dev libjpeg-dev zlib1g-dev libssl-dev libpam0g-dev \
	            libprocps-dev liblzo2-dev libqca2-dev libqca-qt5-2-dev libldap2-dev \
	            libsasl2-dev libavcodec-dev libavformat-dev libswscale-dev


### Configuring and building the sources

Run the following commands:

	mkdir build
	cd build
	cmake ..
	make -j4

NOTE: If you want to build a .deb package for this software, instead of the provided cmake command, you should use:

	cmake -DCMAKE_INSTALL_PREFIX=/usr ..

to install package files in /usr instead of /usr/local.

If some requirements are not fulfilled, CMake will inform you about it and
you will have to install the missing software before continuing.

You can now generate a .deb package

For generating a package, you can run

	fakeroot make package

Then you'll get something like veyon_x.y.z_arch.deb

### Installing the binaries

	sudo dpkg -i veyon_x.y.z_amd64.deb

## License

Copyright (c) 2020 [Miguel Garcia](http://www.reflection.uniovi.es/miguel) and [Jose Quiroga](http://www.reflection.uniovi.es/quiroga) / [University of Oviedo](http://www.uniovi.es).

See the file COPYING for the GNU GENERAL PUBLIC LICENSE.


## More information

* http://www.reflection.uniovi.es/
* https://veyon.io/
