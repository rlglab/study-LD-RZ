if [ "x`whoami`" != "xroot" ]; then 
	echo "Only root can install packages"
	exit
fi

echo "install CMake ... "
apt-get install -y cmake

echo "install boost ... "

apt-get install -y libboost-dev libboost-thread-dev libboost-random-dev libboost-filesystem-dev libboost-regex-dev libboost-system-dev

