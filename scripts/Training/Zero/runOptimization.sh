#!/bin/bash

################################

# check argument
if [ $# -ne 2 ]
then
	echo "Usage: ./runOptimization.sh modelDir cgi_optimization_cfg"
	exit 1
fi

modelDir=$1
cgi_optimization_cfg=$2
rsync_ip=$(grep "ZERO_OPTIMIZATION_RSYNC_IP" $cgi_optimization_cfg | awk -F "=" '{print $2}')
rsync_dir=$(grep "ZERO_OPTIMIZATION_RSYNC_DIR" $cgi_optimization_cfg | awk -F "=" '{print $2}')

# check modelDir is a directory
if [[ ! -d $modelDir ]]
then
	echo "$modelDir is not directory."
	exit 1
fi

# check modelDir contains two prototxts
numPrototxts=$(ls -1 $modelDir/*.prototxt | wc -l 2>/dev/null )
if [ $numPrototxts -ne 1 ]
then
	echo "Please put 1 prototxts, for optimization and evaluator."
	exit 1
fi

# every command in checkCommands must be executable
checkCommands=(Release/CGI rsync rm kill)

for name in "${checkCommands[@]}"
do
	which $name > /dev/null
	if [ $? -ne 0 ]
	then
		echo "Cannot run $name, exit."
		exit 1
	fi
done

# delete existing caffemodels in modelDir
rm -f $modelDir/*.caffemodel $modelDir/*.pb $modelDir/*.pbtxt 2>/dev/null

function onExit()
{
	kill $syncPid 2>/dev/null
	rm -f password-file-modify
	exit
}

echo "rsync_ip = $rsync_ip"
echo "rsync_dir = $rsync_dir"
read -p "Press enter to continue"

# kill all background process when exit
trap onExit SIGINT SIGTERM EXIT
trap true SIGALRM

function syncNet()
{
	while true
	do
		# caffe to caffe2
		cd $modelDir
		prototxt=`ls -1 *.prototxt`
		for model in `ls *.caffemodel 2>/dev/null`
		do
		    pb=$(sed 's/caffemodel/pb/g' <<< $model)
			pb_prototxt=$(sed 's/prototxt/pb/g' <<< $prototxt)
		    if [ ! -f $pb ]; then
				echo "$model -> $pb"
		        python -m caffe2.python.caffe_translator $prototxt $model 2>/dev/null
				mv init_net.pb $pb
				mv predict_net.pb $pb_prototxt
		    fi
		done
		cd ..

		echo "cgimodel_modify~" > password-file-modify
		chmod 600 password-file-modify
		rsync -qrzt --delete --password-file=password-file-modify --exclude=".*" $modelDir/ cgimodel_modify@$rsync_ip::$rsync_dir
		
		sleep 10
	done
}

syncNet &
syncPid=$!

LD_PRELOAD=libkeepalive.so KEEPCNT=1 KEEPIDLE=59 KEEPINTVL=59 Release/CGI -conf_file $cgi_optimization_cfg -mode zero_optimization
