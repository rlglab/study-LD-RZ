#!/bin/bash
# gensgf.sh gpu_per gpu_device_start start_second

if [ "$1" == "" ] || [ "$2" == "" ] || [ "$3" == "" ]; then
	echo "gensgf.sh gpu_per gpu_device_start start_second"
	exit
fi

# download CGI
cd ~
mkdir GO
cd GO
mkdir gensgf
export CGI_RL=/home/cgilab/GO/gensgf
cd $CGI_RL
if ! [ -e CGI_GenSgf ] || ! [ -e database ] || ! [ -e pattern ]; then
	svn co http://sys.aigames.nctu.edu.tw/svn/GO/CGI/trunk GenSgf
	cd GenSgf
	./scripts/setup-cmake.sh release caffe
	make -j10
	mv Release/CGI ../CGI_GenSgf
	mv database/ ../
	mv pattern/ ../
	cd $CGI_RL
	rm -rf GenSgf
fi

# modify kill_cgi for all user
echo "touch /tmp/.kill_cgi.txt" > /bin/kill_cgi
chmod +x /bin/kill_cgi

counter=0
while true; do
	gpu_device=$(nvidia-smi | grep "Default" | awk -v per=$1 -v gpu_device_start=$2 ' BEGIN {
		gpu_device = "";
		step = (gpu_device_start==0)? 1: -1;
		counter = gpu_device_start;
	} {
		if( $12=="|" ) { split($13,result,"%"); }
		else { split($12,result,"%"); }
		if( result[1]<per ) {
			gpu_device = gpu_device""counter;
		}
		counter += step;
	} END { print gpu_device; }')

	if [ "$gpu_device" != "" ]; then
		counter=$((counter+1))
	else
		counter=0
	fi
	
	if [ $counter -eq $3 ]; then
		echo "Detect gpu ("$gpu_device") percentage is lower than "$1"%, start running CGI.\n Use \"kill_cgi\" to stop CGI." | wall
    
		cd $CGI_RL
		NUM_THREAD=$((${#gpu_device}*2))
		SL_CONFIGURE="DCNN_SL_FEATURE_STRING=G49:DCNN_SL_NET=Go19_G49_SL.prototxt:DCNN_SL_MODEL=Go19_G49_RL_iter10000.caffemodel:DCNN_SL_RANDOM_ROTATE=1"
		./CGI_GenSgf -conf_str "NUM_THREAD="$NUM_THREAD":"$SL_CONFIGURE":USE_TIME_SEED=true:DCNN_TRAIN_GPU_LIST="$gpu_device -mode gensgf > /dev/null 2>&1 &
		pid=$!
		counter=0
	fi
	
	if [ -e "/tmp/.kill_cgi.txt" ]; then
		kill -9 $pid
		rm -rf /tmp/.kill_cgi.txt
	fi
	
	sleep 1
done
