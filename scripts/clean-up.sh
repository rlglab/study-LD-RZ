#!/bin/bash

if [ "x`pwd | grep "scripts$"`" != "x" ]; then
	cd ..
fi

echo "Clean up CMake files ... "
rm -rf CGI/CMakeFiles MCTPS/CMakeFiles CMakeFiles CGI/cmake_install.cmake MCTPS/cmake_install.cmake cmake_install.cmake CMakeCache.txt
echo "Clean up Result files ... "
rm -rf Debug Release
echo "Clean Make file ... "
rm -f Makefile

