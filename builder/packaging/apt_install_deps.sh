#!/bin/bash
# shell script used by CI to install apt dependencies

if [[ "$1" == "x86" ]]; then
    apt_suffix=":i386"
fi

deps="alien build-essential${apt_suffix}" # alien does not need ${apt_suffix}
for dep in sdl2 sdl2-image sdl2-mixer sdl2-ttf sdl2-net; do
    deps="$deps lib${dep}-dev${apt_suffix}"
done

if [[ $apt_suffix == ":i386" ]]; then
    dpkg --add-architecture i386
fi

apt-get update --fix-missing
apt-get upgrade -y

if [[ $apt_suffix == ":i386" ]]; then
    apt-get install gcc-multilib g++-multilib -y
fi

echo Installing: $deps

apt-get install $deps -y

 