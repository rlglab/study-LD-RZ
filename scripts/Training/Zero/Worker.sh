#!/bin/bash

if [ $# -lt 2 ]
then
	echo "Usage: ./Worker.sh host port [-g GPU_LIST] [-b BATCH_SIZE] [--no-evaluator]"
	exit 1
else
	HOST=$1
	PORT=$2
	shift
	shift
	# default arguments
	NUM_GPU=$(nvidia-smi -L | wc -l)
	GPU_LIST=$(echo $NUM_GPU | awk '{for(i=0;i<$1;i++)printf i}')
	BATCH_SIZE=16
	MAX_NUM_CPU_THREAD_PER_GPU=4
	CAFFE2=1
	DO_EVALUATOR=1
fi

while :; do
	case $1 in
		-g|--gpu) shift; GPU_LIST=$1; NUM_GPU=${#GPU_LIST}
		;;
		-b|--batch_Size) shift; BATCH_SIZE=$1
		;;
		-c|--cpu_thread_per_gpu) shift; MAX_NUM_CPU_THREAD_PER_GPU=$1
		;;
		--no-evaluator) DO_EVALUATOR=0
		;;
		"") break
		;;
		*) echo "Unknown argument: $1"; exit 1
		;;
	esac
	shift
done

MAX_NUM_CPU_THREAD=$((MAX_NUM_CPU_THREAD_PER_GPU*NUM_GPU))

if [ -f "CGI" ]
then
	echo "./CGI exists, terminating."
	exit 1
fi

# every command in checkCommands must be executable
checkCommands=(java gogui-twogtp gogui-adapter Release/CGI rsync rm flock kill pkill nvidia-smi)

for name in "${checkCommands[@]}"
do
	which $name > /dev/null
	if [ $? -ne 0 ]
	then
		echo "Cannot run $name, exit."
		exit 1
	fi
done

function closeFd()
{
	local fd=$1
	eval "exec {fd}<&-"
	eval "exec {fd}>&-"
}

function onExit()
{
	if [[ ! -z $broker_fd ]]
	then
		[[ -z  $twoGtpPids ]] || flock -x $broker_fd kill $twoGtpPids 2>/dev/null
		[[ -z  $selfPlay_pid ]] || flock -x $broker_fd kill -CONT $selfPlay_pid
		[[ -z  $selfPlay_pid ]] || flock -x $broker_fd kill $selfPlay_pid 2>/dev/null
		rm -f $broker_fd
		closeFd $broker_fd
	fi
	exit
}

# kill all background process when exit
trap onExit SIGINT SIGTERM EXIT
trap true SIGALRM

function kill_descendant_processes()
{
	local pid="$1"
	local and_self="${2:-false}"
	if children="$(pgrep -P "$pid")"; then
		for child in $children; do
			kill_descendant_processes "$child" true
		done
	fi
	if [[ "$and_self" == true ]]; then
		kill "$pid" 2>/dev/null
	fi
}

function onRunTwoGtpExit()
{
	kill_descendant_processes $twoGtpPid true 2>/dev/null
	exit
}

function runTwoGtp()
{
	trap onRunTwoGtpExit SIGINT SIGTERM RETURN
	
	if [ $# -ne 8 ]
	then
		echo "need 8 arguments: netA netB boardSize komi CGI_CONFIGURE CGI_RANDOM_SEED gpuID postfix" >&1
		sleep 1
		return 0
	fi
	local netA=$1
	local netB=$2
	local boardSize=$3
	local komi=$4
	local CGI_RANDOM_SEED=$6
	local CGI_CONFIGURE_netA="DCNN_DEFAULT_BATCH_SIZE=4:RANDOM_SEED=$CGI_RANDOM_SEED:$5"
	local CGI_CONFIGURE_netB="DCNN_DEFAULT_BATCH_SIZE=4:RANDOM_SEED=$((CGI_RANDOM_SEED+1)):$5"
	local gpu=$7
	local postfix=$8
	local numGames=0
	local nthGames=0
	
	[[ -d results ]] || mkdir results
	
	# setup cgi configure
	[[ $CAFFE2 ]] || CGI_CONFIGURE_netA=$(echo $CGI_CONFIGURE_netA | perl -pe "s/\.prototxt/_Evaluator\.prototxt/g")
	CGI_CONFIGURE_netA=$(echo $CGI_CONFIGURE_netA | perl -pe "s/caffemodel/$netA/g")
	CGI_CONFIGURE_netA=$(echo $CGI_CONFIGURE_netA | perl -pe "s/gpu_id/$gpu$gpu/g")
	[[ ! $CAFFE2 ]] || CGI_CONFIGURE_netA=$(sed 's/caffemodel/pb/g;s/prototxt/pb/g' <<< $CGI_CONFIGURE_netA)
	
	[[ $CAFFE2 ]] || CGI_CONFIGURE_netB=$(echo $CGI_CONFIGURE_netB | perl -pe "s/\.prototxt/_Evaluator\.prototxt/g")
	CGI_CONFIGURE_netB=$(echo $CGI_CONFIGURE_netB | perl -pe "s/caffemodel/$netB/g")
	CGI_CONFIGURE_netB=$(echo $CGI_CONFIGURE_netB | perl -pe "s/gpu_id/$gpu$gpu/g")
	[[ ! $CAFFE2 ]] || CGI_CONFIGURE_netB=$(sed 's/caffemodel/pb/g;s/prototxt/pb/g' <<< $CGI_CONFIGURE_netB)
	
	# run twogtp
	BLACK="Release/CGI -conf_str $CGI_CONFIGURE_netA"
	WHITE="Release/CGI -conf_str $CGI_CONFIGURE_netB"
	ADAPTER_BLACK="gogui-adapter -name $netA \"$BLACK\""
	ADAPTER_WHITE="gogui-adapter -name $netB \"$WHITE\""
	mkdir -p "results/$netA-$netB"
	sgffile="results/$netA-$netB/fight$gpu-$postfix"
	rm -f "$sgffile.dat"
	echo "gogui-twogtp -black \"$ADAPTER_BLACK\" -white \"$ADAPTER_WHITE\" -size $boardSize -komi $komi -alternate -force -auto -games 0 -sgffile $sgffile"
	gogui-twogtp -black "$ADAPTER_BLACK" -white "$ADAPTER_WHITE" -size $boardSize -komi $komi -alternate -force -auto -games 0 -sgffile $sgffile &
	twoGtpPid=$!

	while true
	do
		numGames=$(grep -s -v "#" $sgffile.dat | wc -l)
		while [ $nthGames -lt $numGames ]
		do
			sleep 0.01
			local sgfFileName="$sgffile-$nthGames.sgf"
			local sgfContent=$(cat $sgfFileName | paste -s -d ' ')
			local winNet=""
			
			# change RE[] color when nthGames is odd
			if [[ $sgfContent =~ RE\[([BW0])\+ ]]
			then
				local color=${BASH_REMATCH[1]}
				color=${color^^} # toupper
				if [ $color == "B" ]
				then
					if [ $(($nthGames%2)) -eq 1 ]
					then
						sgfContent=$(echo $sgfContent | perl -pe 's/RE\[B\+/RE\[W\+/g')
					fi
					winNet=$netA
				elif [ $color == "W" ]
				then
					if [ $(($nthGames%2)) -eq 1 ]
					then
						sgfContent=$(echo $sgfContent | perl -pe 's/RE\[W\+/RE\[B\+/g')
					fi
					winNet=$netB
				elif [ $color == "0" ]
				then
					winNet="Draw"
				fi
				# send sgf to broker
				flock -x $broker_fd echo "Evaluator $winNet $sgfContent" >&$broker_fd
			else
				# no match result
				flock -x $broker_fd echo "Evaluator ? $sgfContent" >&$broker_fd
			fi
			
			nthGames=$(($nthGames+1))
		done
		sleep 0.5
	done
}

function syncFile()
{
	if [ $# -ne 1 ]
	then
		echo "need 1 arguments: file" >&1
		sleep 1
		return 0
	fi

	local file=$1
	local httpuser=4cvYAztEMR
	local httppasswd=25QkJW7kgPX9vk2s6gc2WR6vkzQAEGsC7JNrYZCS2ErKyTWY66q2tgE3w5rRFpRAv3gUhKxAQT
	[[ ! $CAFFE2 ]] || file=$(sed 's/caffemodel/pb/g;s/prototxt/pb/g' <<< $file)

	# step 1: check file exist on server
	local fileMissing=$(wget --spider --http-user=$httpuser --http-password=$httppasswd http://$rsync_ip/$rsync_dir/$file 2>&1 | grep "404 Not Found" | wc -l)
	while (( $fileMissing ))
	do
		# sleep and check again
		sleep 15
		fileMissing=$(wget --spider --http-user=$httpuser --http-password=$httppasswd http://$rsync_ip/$rsync_dir/$file 2>&1 | grep "404 Not Found" | wc -l)
	done
	
	# step 2: server have file, download file
	sleep $(bc -l <<< "scale=4 ; ${RANDOM}/32767") # randomly sleep 0~1 seconds
	wget -N --http-user=$httpuser --http-password=$httppasswd --directory-prefix=database/dcnn http://$rsync_ip/$rsync_dir/$file

	# step 3: server and local have file, check file size match
	local serverFileSize=$(wget --spider --http-user=$httpuser --http-password=$httppasswd http://$rsync_ip/$rsync_dir/$file 2>&1 | grep Length | awk '{print $2}')
	local localFileSize=$(wc -c database/dcnn/$file | awk '{print $1}')
	while (( $serverFileSize != $localFileSize ))
	do
		# other Worker.sh is downloading, just wait until download finish and check again
		sleep 1
		localFileSize=$(wc -c database/dcnn/$file | awk '{print $1}')
	done
}

NUM_CPU_THREAD=$(lscpu -p=CORE | grep -v "#" | wc -l)
if [ $NUM_CPU_THREAD -gt $MAX_NUM_CPU_THREAD ]; then
	NUM_CPU_THREAD=$MAX_NUM_CPU_THREAD
fi

while true
do
	# try to connect to broker
	twoGtpPids=""
	selfPlay_pid=""
	broker_fd=""
	prevBestNet=""
	firstDownload=1
	exec {broker_fd}<>/dev/tcp/$HOST/$PORT
	if [[ -z $broker_fd ]]
	then
		# connect failed
		echo "connect to $HOST:$PORT failed, retry after 60s..."
		sleep 60
	else
		while true
		do
			# read from broker
			read -u $broker_fd line
			error_code=$?
			if [ $error_code -eq 0 ]
			then
				echo "read: $line"
				if [ "$line" == "Info" ]
				then
					# NAME="hostname_GPU_LIST@ip"
					NAME=$(hostname)"_"$GPU_LIST"@"$(ifconfig | grep -Eo 'inet (addr:)?([0-9]*\.){3}[0-9]*' | grep -Eo '([0-9]*\.){3}[0-9]*' | grep -v '127.0.0.1' | head -n1)
					echo "Info $NAME $NUM_GPU" 1>&$broker_fd
				elif [[ $line =~ ^Rsync_info\ (.+)\ (.+) ]]
				then
					# record rsync info
					rsync_ip=${BASH_REMATCH[1]}
					rsync_dir=${BASH_REMATCH[2]}
					echo "Rsync_info: $rsync_ip $rsync_dir"
				elif [[ $line =~ ^Download\ (.+) ]]
				then
					if [ $DO_EVALUATOR -eq 1 ] || [ $firstDownload -eq 1 ]
					then
						# sync file
						file=${BASH_REMATCH[1]}
						syncFile $file
						firstDownload=0
					else
						echo "skip download"
					fi
				elif [ "$line" == "keep_alive" ]
				then
					true
				elif [[ $line =~ ^Self-play\ (.+)\ (.+)\ (.+) ]]
				# Self-play * cgi_configure seed
				then
					echo "Self-play"
					bestNet=${BASH_REMATCH[1]}

					# kill previous runTwoGtp
					[[ -z  $twoGtpPids ]] || flock -x $broker_fd kill $twoGtpPids 2>/dev/null
					twoGtpPids=""

					[[ -z  $selfPlay_pid ]] || flock -x $broker_fd kill -CONT $selfPlay_pid
					# if skip evaluator and bestNet is not changed, just keep running
					if [ "$bestNet" == "$prevBestNet" ]
					then
						echo "keep running"
						continue
					else
						prevBestNet=$bestNet
					fi
					
					# kill previous self-play
					[[ -z  $selfPlay_pid ]] || flock -x $broker_fd kill $selfPlay_pid 2>/dev/null
					selfPlay_pid=""
					
					CGI_CONFIGURE=${BASH_REMATCH[2]}
					CGI_RANDOM_SEED=${BASH_REMATCH[3]}
					echo "bestNet is $bestNet"
					
					# sync model file
					echo "syncFile..."
					syncFile $bestNet
					echo "syncFile done"
					
					# run cgi self-play
					CGI_CONFIGURE="\"DCNN_DEFAULT_BATCH_SIZE=$BATCH_SIZE:RANDOM_SEED=$CGI_RANDOM_SEED:NUM_THREAD=$NUM_CPU_THREAD:DCNN_TRAIN_GPU_LIST=$GPU_LIST:$CGI_CONFIGURE\""
					CGI_CONFIGURE=$(echo $CGI_CONFIGURE | perl -pe "s/caffemodel/$bestNet/g")
					
					
					[[ ! $CAFFE2 ]] || CGI_CONFIGURE=$(sed 's/caffemodel/pb/g;s/prototxt/pb/g' <<< $CGI_CONFIGURE)
					
					echo "Release/CGI -conf_str \"$CGI_CONFIGURE\" -mode zero_selfplay"
					Release/CGI -conf_str \"$CGI_CONFIGURE\" -mode zero_selfplay 1>&$broker_fd 2>/dev/null &
					selfPlay_pid=$!
				elif [[ $line =~ ^Evaluator\ (.+)\ (.+)\ (.+)\ (.+)\ (.+)\ (.+) ]]
				# Evaluator * theta boardSize komi cgi_configure seed
				then
					if [ $DO_EVALUATOR -eq 0 ]
					then
						echo "skip evaluator"
						continue
					fi
					
					echo "Evaluator"
					bestNet=${BASH_REMATCH[1]}
					newNet=${BASH_REMATCH[2]}
					boardSize=${BASH_REMATCH[3]}
					komi=${BASH_REMATCH[4]}
					CGI_CONFIGURE=${BASH_REMATCH[5]}
					CGI_RANDOM_SEED=${BASH_REMATCH[6]}
					
					# kill previous runTwoGtp
					[[ -z  $twoGtpPids ]] || flock -x $broker_fd kill $twoGtpPids 2>/dev/null
					twoGtpPids=""
					
					# sync model file
					echo "syncFile..."
					syncFile $bestNet
					syncFile $newNet
					echo "syncFile done"
					
					# run cgi evaluator
					[[ -z  $selfPlay_pid ]] || flock -x $broker_fd kill -STOP $selfPlay_pid 2>/dev/null
					#selfPlay_pid=""
					
					for (( i=0; i<$NUM_GPU; i++ ))
					do
						gpuID=${GPU_LIST:$i:1}
						netA=""
						netB=""
						if [[ $((RANDOM%2)) == 0 ]]
						then
							netA=$bestNet
							netB=$newNet
						else
							netA=$newNet
							netB=$bestNet
						fi
						echo $netA $netB
						runTwoGtp $netA $netB $boardSize $komi ${CGI_CONFIGURE} $((CGI_RANDOM_SEED+i*4)) $gpuID 0 &
						twoGtpPids="$twoGtpPids $!"
					done
					echo "Running gogui-twogtp (bestNet: $bestNet, newNet: $newNet)"
				else
					echo "read format error"
					echo "msg: $line"
					
					# close socket
					closeFd $broker_fd
					echo "disconnected from broker"
					sleep 10
					
					break
				fi
			elif [ $error_code -gt 128 ]
			then
				# read timeout, do nothing
				true
			else
				# disconnected
				echo "disconnected from broker"
				sleep 10
				
				break
			fi
		done
	fi

	# disconnected, clean up running process
	if [[ ! -z  $broker_fd ]]
	then
		[[ -z  $twoGtpPids ]] || flock -x $broker_fd kill $twoGtpPids 2>/dev/null
		[[ -z  $selfPlay_pid ]] || flock -x $broker_fd kill $selfPlay_pid 2>/dev/null
	fi
done
