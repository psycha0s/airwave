## About
Airwave is a [WINE](https://www.winehq.org/)-based VST bridge, that allows for the use of Windows 32- and 64-bit VST 2.4 audio plugins with Linux VST hosts.
Due to the use of shared memory, only one extra copying is made for each data transfer. Airwave also uses the XEMBED protocol to correctly embed the plugin editor into the host window.

## Compatibility
The following list is not complete. It contains only plugins, that have been tested by me or by people, who sent me a report.

 VST-Plugins | works? | Notes |
------------:|:----------:|:-------|
 Blue Cat Audio Oscilloscope Multi | no | doesn't work with wine
 Credland Audio BigKick | no | doesn't work with wine
 FabFilter Total bundle | yes | haven't tested them all
 Image-Line Harmless | yes |
 Image-Line Sytrus | yes |
 LennarDigital Sylenth1 | no | doesn't work with wine
 LePou Plugins | yes | LeCab2 has slight GUI redrawing issues
 NI Absynth | yes |
 NI FM8 | yes |
 NI Guitar Rig 5 | yes | activation doesn't work
 NI Kontakt 5 | mostly | up to v5.3.1, can import libraries only in Windows XP mode
 NI Massive | yes | only 32-bit
 NI Reaktor 5 | yes |
 Peavey Revalver Mark III.V | yes |
 ReFX Nexus2 | yes |
 ReFX Vanguard | yes |
 Reveal Sound Spire | yes | starting form 1.0.19 the GUI doesn't appear (wine issue)
 Sonic Academy A.N.A. | yes |
 Sonic Academy KICK | yes |
 Smartelectronix s(M)exoscope | yes |
 Synth1 by Ichiro Toda | yes |
 Tone2 FireBird | yes |
 Tone2 Nemesis | yes |
 Tone2 Saurus | yes |
 u-he A.C.E. | yes | Linux version is also available
 u-he Bazille | yes | Linux version is also available
 u-he Diva | yes | Linux version is also available
 u-he Hive | yes | Linux version is also available
 u-he Presswerk | yes | Linux version is also available
 u-he Satin | yes | Linux version is also available
 u-he Uhbik | yes | Linux version is also available
 u-he Zebra2 | yes | Linux version is also available
 Variety of Sound | no | crash immediately
 Voxengo SPAN | yes |
 Voxengo SPAN Pro | mostly | inter plugin routing doesn't work (architecture issue)
 Xfer Serum | partly | the GUI doesn't appear (wine issue), but audio works

## Requirements
- WINE, supporting XEMBED protocol (versions greater than 1.7.19 were tested,
but earlier versions also may work)
- libmagic
- Qt5 for the airwave manager application (GUI)

## Building the source
1. Install the required packages: multilib-enabled GCC, cmake, git, wine, Qt5, libmagic.
  * **Arch Linux (x86_64)** example:
    ```
    sudo pacman -S gcc-multilib cmake git wine qt5-base
    ```

  * **Fedora 20 (x86_64)** example:
    ```
    sudo yum -y install gcc-c++ git cmake wine wine-devel wine-devel.i686 file file-devel libX11-devel libX11-devel.i686 qt5-devel glibc-devel.i686 glibc-devel
    ```

  * **Ubuntu 14.04 (x86_64)** example:
    ```
    sudo apt-get install git cmake gcc-multilib g++-multilib libx11-dev libx11-dev:i386 qt5-default libmagic-dev
    sudo add-apt-repository ppa:ubuntu-wine/ppa
    sudo apt-get update
    sudo apt-get install wine1.7 wine1.7-dev
    ```
2. Get the VST Audio Plugins SDK [from Steinberg](http://www.steinberg.net/en/company/developers.html). I cannot distribute it myself due to the license restrictions.

3. Unpack the VST SDK archive. Further I'll assume that you have unpacked it in your home directory: ${HOME}/VST3\ SDK.

4. Clone the airwave GIT repository
  ```
  git clone https://github.com/phantom-code/airwave.git
  ```

5. Go to the airwave source directory and execute the following commands:
  ```
  mkdir build && cd build
  cmake -DCMAKE_BUILD_TYPE="Release" -DCMAKE_INSTALL_PREFIX=/opt/airwave -DVSTSDK_PATH=${HOME}/VST3\ SDK ..
  make
  make install
  ```

Of course, you can change the CMAKE_INSTALL_PREFIX as you like.

## Usage
1. Run the airwave-manager
2. Press the "Create link" button on the toolbar.
3. Select desired WINE loader and WINE prefix in the appropriate combo boxes.
4. Enter a path to VST plugin DLL file in the "VST plugin" field (you can use the "Browse" button for convenience). Note, that the path is relative to the selected WINE prefix.
5. Enter a "Link location" path (the directory, where your VST host looks for the plugins).
6. Enter a link name, if you don't like the auto-suggested one.
7. Select a desired log level for this link. The higher the log level, the more messages you'll receive. The 'default' log level is a special value. It corresponds to the 'Default log level' value from the settings dialog. In most cases, the 'default' log level is the right choice. For maximum performance do not use a higher level than 'trace'.
7. Press the "OK" button. At this point, your VST host should be able to find a new plugin inside of the "Link location" directory.

**Note:** After you have created the link you cannot move/rename it with a file manager. All updates have to be done inside the airwave-manager. Also, you should update your links after updating the airwave itself. This could be achived by pressing the "Update links" button.

## Under the hood
The bridge consists of four components:
- Plugin endpoint (airwave-plugin-<arch>.so)
- Host endpoint (airwave-host-<arch>.exe.so and airwave-host-<arch>.exe launcher script)
- Configuration file (${XDG_CONFIG_PATH}/airwave/airwave.conf)
- GUI configurator (airwave-manager)

When the airwave-plugin is loaded by the VST host, it obtains its absolute path and use it as the key to get the linked VST DLL from the configuration. Then it starts the airwave-host process and passes the path to the linked VST file. The airwave-host loads the VST DLL and works as a fake VST host. Starting from this point, the airwave-plugin and airwave-host act together like a proxy, translating commands between the native VST host and the Windows VST plugin.

## Known issues
- Due to a bug in WINE, there is some hacking involved when embedding the editor window. There is a chance that you get a black window instead of the plugin GUI. Also some areas might not update correctly when increasing the window size. On some hosts (Bitwig Studio for example) this can be solved by closing and re-opening the plugin window.
