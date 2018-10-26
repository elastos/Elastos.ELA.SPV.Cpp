# Elastos.ELA.SPV.Cpp

## Summary
This repository is a basic library aimed to provide a serials of wallet related interfaces for anyone who want to build a wallet themselves.


## Build on Ubuntu/MacOs
### Check the required tools
Make sure your computer have installed the required packages below:
* [git](https://www.git-scm.com/downloads)
* [cmake](https://cmake.org/download)
* [wget](https://www.gnu.org/software/wget)

### Clone source code
Open a terminal, go to `/home/xxx/dev`
```shell
$ cd /home/xxx/dev/
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
$ cd Elastos.ELA.SPV.Cpp
$ git submodule init
$ git submodule update
```

### Build Elastos.ELA.SPV.Cpp

Create a build directory `cmake-build-release`
```shell
$ cd /home/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build-release
```

Execute cmake command to generate Makefile, and make
```shell
$ cd cmake-build-release
$ cmake ..
$ make
```


## Build for Android
### Check the required tools
Make sure your computer have installed the required packages below:
* [git](https://www.git-scm.com/downloads)
* [cmake](https://cmake.org/download)
* [wget](https://www.gnu.org/software/wget)
* [ndk](https://developer.android.com/ndk/downloads/)

### Install ndk
`NDK` version: r16+

Unzip to somewhere, for example `/Users/xxx/dev/android-ndk-r16`

Set system environment variable `ANDROID_NDK` to `/Users/xxx/dev/android-ndk-r16`

### Clone source code
Open terminal, go to `/Users/xxx/dev`
```shell
$ cd /Users/xxx/dev/
$ git clone git@github.com:elastos/Elastos.ELA.SPV.Cpp.git
$ cd Elastos.ELA.SPV.Cpp
$ git submodule init
$ git submodule update
```

### Build Elastos.ELA.SPV.Cpp

Create a build directory `cmake-build-ndk-release`
```shell
$ cd /Users/xxx/dev/Elastos.ELA.SPV.Cpp
$ mkdir cmake-build-ndk-release
```

Execute cmake command to generate Makefile, and make
```shell
$ cd cmake-build-ndk-release
$ cmake -DSPV_FOR_ANDROID=ON ..
$ make
```


## Build for iOS
### Install twilio-boost-build
Go to https://github.com/twilio/twilio-boost-build and build the library

### Clone source code
This explanation is specific to branch release_v0.3.0
$ git clone https://github.com/ademcan/Elastos.ELA.SPV.Cpp
$ cd Elastos.ELA.SPV.Cpp
$ git remote add upstream git@github.com:elastos/Elastos.ELA.SPV.Cpp
$ git checkout release_v0.3.0
$ git pull upstream release_v0.3.0
$ git submodule init
$ git submodule update

Edit the ThirdParty/boost.cmake on lines 7, 49 and 73 by providing the correct paths (will not be required on the long run as building twilio-boost-build can be included on the cmake file)
$ mkdir cmake-build-ios-release
$ cd cmake-build-ios-release
$ cmake -DSPV_FOR_IOS=ON -G Xcode ..

### Build SPVSDK on Xcode
Open SPVSDK.xcodeproj
Click the name of the project and select PROJECT/SPVSDK -> Build Settings
Change Base SDK to iOS
Wait for "Indexing | Processing files" to finish
Select TARGE/ALL_BUILD and build for any simulator
This will generate the final library under: Elastos.ELA.SPV.Cpp/cmake-build-ios-release/SPVSDK.build/Debug/spvsdk_object.build/Objects-normal/libspvsdk_object.a



## Development
Patches are welcome. Please submit pull requests against the `dev` branch.


## More

Learn more about this repository please refer to the following links:
- [interfaces document](https://raindust.github.io/Elastos.ELA.SPV.Cpp.Document/)
- [wiki](https://github.com/elastos/Elastos.ELA.SPV.Cpp/wiki)
