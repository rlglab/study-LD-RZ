if [ "$1" == "" ]; then
	echo "./summarize_bv.sh BV_log.file interval=1000"
	exit
fi

INTERVAL=$2
if [ "$2" == "" ]; then
	INTERVAL=1000
fi

tac $1 | awk -v interval=$INTERVAL 'BEGIN {
	scale_split = 15;
	max_scale = 300/scale_split;
	bIsStart = 0;
	iteration = 0;
} {	
	# for scale loss
	if( iteration<=interval ) {
		if($1=="move_number") { bIsStart=0; iteration++; }
		if(bIsStart==1) {
			scale = int(($1+1)/scale_split);
			scale = (scale>max_scale)? max_scale: scale;
			dBVError[scale]+=$2;
			dCount[scale]++;
			dTotalCount++;
		}
		if($1=="CGI" && $2=="loss") { bIsStart=1; }
	}
} END {
	printf("scale\t: bv\t\t(count / per)\t");
	for(i=0; i<80; i++) {
		if(i%20==0) { printf(int(i/20)); }
		else { printf(" "); }
	}
	printf("I\n");
	
	dTotalBVError = 0;
	for(scale=0; scale<max_scale; scale++) {
		dTotalBVError += dBVError[scale];
		if( dCount[scale]!=0 ) {
			dAvgBVLoss = dBVError[scale]/dCount[scale];
		} else { dAvgBVLoss = 0; }
		
		printf(scale*scale_split"-"(scale+1)*scale_split"\t: %.4f"shift"\t(%4d / %.2f%%)\t",dAvgBVLoss,dCount[scale],dCount[scale]*100/dTotalCount);
		
		loss_bv_index = int(2*dAvgBVLoss);
		for(i=0; i<80; i++) {
			if(i==loss_bv_index) { printf("\033[1;31mX\033[0m"); }
			else { printf(" "); }
		}
		printf("\n");
	}
	printf("\n");
	
	printf("Total CGI avg loss\t= " dTotalBVError/dTotalCount " ("dTotalCount")\n");
}'
