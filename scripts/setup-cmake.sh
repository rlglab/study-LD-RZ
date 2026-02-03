#!/bin/bash

mode="debug"
compiler="gcc"
link="dynamic"
extend_liberty=""

while (( "$#" )); do
	case $1 in
		release)
			mode="release"
		;;
		debug)
			mode="debug"
		;;
		gcc)
			compiler="gcc"
		;;
		clang)
			compiler="clang"
		;;
		static)
			link="static"
		;;
		share)
			link="share"
		;;
		caffe)
			extend_liberty="caffe"
		;;
		caffe2)
			extend_liberty="caffe2"
		;;
		*)
			echo "Unknown option"
			exit 1
		;;
	esac
	shift
done

echo "Mode: $mode"
echo "Compilter: $compiler"
echo "link: $link"
echo "extend_liberty: $extend_liberty"

if [ "x`pwd | grep "scripts$"`" != "x" ]; then
	cd ..
fi

MODEOPT=
LINKOPT=
COMPOPT=

if [ "x$mode" == "xrelease" ]; then
	MODEOPT="-DCMAKE_BUILD_TYPE=Release"
fi

if [ "x$link" == "xstatic" ]; then
	LINKOPT="-DSTATIC_LINK=1"
fi

if [ "x$compiler" == "xclang" ]; then
	export CC=/usr/bin/clang
	export CXX=/usr/bin/clang++
	COMPOPT="-DCMAKE_USER_MAKE_RULES_OVERRIDE=scripts/ClangOverrides.txt"
fi

if [ "x$extend_liberty" == "xcaffe" ]; then
	MODEOPT=$MODEOPT" -DUSE_CAFFE=true"
fi

if [ "x$extend_liberty" == "xcaffe2" ]; then
	MODEOPT=$MODEOPT" -DUSE_CAFFE2=true"
fi

cmake . $MODEOPT $LINKOPT $COMPOPT
