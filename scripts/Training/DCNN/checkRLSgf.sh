if [ "$1" == "" ]; then
	echo "./checkRLSgf.sh sgf_path"
	exit
fi

output1=$(tail -c2 $@ | awk '{ if( index($1,"==>") ) { counter=0; dString=$0; } counter++; if( counter==2 && $1!=")" ) { print dString; } }')
output2=$(tail -c2 $@ | awk '{ if( index($1,"==>") ) { counter=0; dString=$0; } counter++; if( counter==2 && $1!=")" ) { print dString; } }')

if [ "$output1" == "$output2" ] && [ "$output1" == "" ]; then
	echo "check successfully"
else
	echo $output1
fi
