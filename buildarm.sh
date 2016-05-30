#!/bin/sh

export ARCH=arm
TARGET=$1

# Sets up toolchain environment variables for various mips toolchain

warn()
{
	echo "$1" >&2
}

if [ ! -z $(which arm-linux-gnueabi-gcc) ];
then
	export CC=$(which arm-linux-gnueabi-gcc)
else
	warn "Not setting CC: can't locate arm-linux-gnueabi-gcc."
fi

if [ ! -z $(which arm-linux-gnueabi-ld) ];
then
	export LD=$(which arm-linux-gnueabi-ld)
else
	warn "Not setting LD: can't locate arm-linux-gnueabi-ld."
fi

if [ ! -z $(which arm-linux-gnueabi-ar) ];
then
	export AR=$(which arm-linux-gnueabi-ar)
else
	warn "Not setting AR: can't locate arm-linux-gnueabi-ar."
fi


if [ ! -z $(which arm-linux-gnueabi-strip) ];
then
	export STRIP=$(which arm-linux-gnueabi-strip)
else
	warn "Not setting STRIP: can't locate arm-linux-gnueabi-strip."
fi

if [ ! -z $(which arm-linux-gnueabi-nm) ];
then
	export NM=$(which arm-linux-gnueabi-nm)
else
	warn "Not setting NM: can't lcoate arm-linux-gnueabi-nm."
fi

make clean
make $TARGET || exit $?


