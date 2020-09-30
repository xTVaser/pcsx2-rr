#!/bin/bash

set -e

# Packages - Build Environment
declare -a BUILD_PACKAGES=(
	"ccache"
	"cmake"
	"g++-10-multilib"
)

# Packages - PCSX2
declare -a PCSX2_PACKAGES=(
	"libaio-dev"
	"libasound2-dev"
	"libcairo2-dev"
	"libegl-dev"
	"libegl1-mesa-dev"
	"libgdk-pixbuf2.0-dev"
	"libgirepository-1.0-1"
	"libgl-dev"
	"libgl1-mesa-dev"
	"libgl1-mesa-dri"
	"libgl1"
	"libgles-dev"
	"libgles-dev"
	"libgles2-mesa-dev"
	"libglib2.0-dev"
	"libglu1-mesa-dev"
	"libglu1-mesa"
	"libglvnd-dev"
	"libglx-dev"
	"libglx-mesa0"
	"libglx0"
	"libgtk-3-dev"
	"libgtk2.0-dev"
	"libharfbuzz-dev"
	"libllvm10"
	"liblzma-dev"
	"libpango1.0-dev"
	"libpcap0.8-dev"
	"libsdl2-dev"
	"libsoundtouch-dev"
	"libwxgtk3.0-gtk3-0v5"
	"libwxgtk3.0-gtk3-dev"
	"libx11-xcb-dev"
	"libxext-dev"
	"libxft-dev"
	"libxml2-dev"
	"portaudio19-dev"
	"zlib1g-dev"
)

# - https://github.com/actions/virtual-environments/blob/main/images/linux/Ubuntu2004-README.md
ARCH=""
echo "${PLATFORM}"
if [ "${PLATFORM}" == "x86" ]; then
	ARCH=":i386"
	sudo dpkg --add-architecture i386
fi

sudo apt-get -qq update
# TODO - needed? sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test

# Install packages needed for building
BUILD_PACKAGE_STR=""
for i in "${BUILD_PACKAGES[@]}"; do
	BUILD_PACKAGE_STR="${BUILD_PACKAGE_STR} ${i}"
done
echo "Will install the following packages for building - ${BUILD_PACKAGE_STR}"
sudo apt remove gcc-9 g++-9
sudo apt-get -y install ${BUILD_PACKAGE_STR}

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 10
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-10 10
sudo update-alternatives --install /usr/bin/cc cc /usr/bin/gcc 30
sudo update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++ 30

# Install packages needed by pcsx2
PCSX2_PACKAGES_STR=""
for i in "${PCSX2_PACKAGES[@]}"; do
	PCSX2_PACKAGES_STR="${PCSX2_PACKAGES_STR} ${i}${ARCH}"
done
echo "Will install the following packages for pcsx2 - ${PCSX2_PACKAGES_STR}"
sudo apt-get -y install ${PCSX2_PACKAGES_STR}
