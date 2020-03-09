#!/bin/bash

# $1 will hold the path to the file handled by out char device(PS_READER).
# $2 will hold the name of the char file.
# $3 will hold the major number of the char device.

if [ $# != 3 ]
then
    echo "run script with $0 pathToCharDevice nameOfCharDevice majorNumberOfCharDevice"
    exit -1
fi

# check if the char device exists - maybe someone deleted it?
#.................

IFS=' ' # delimiter.

echo "start" >> $1$2 #send start signal.

ps -axjf | while read line; do
    len=${#line}

	read -ra ADDR <<< "$line" # str is read into an array as tokens separated by IFS

	ppid=${ADDR[0]}
	pid=${ADDR[1]}
	cmd=${ADDR[9]}
	declare -i i=0
	
	getIndex() {
        ADDR=$1
        index=$2
        if [ ${ADDR[index]} == "|" ] || [ ${ADDR[index]} == "_" ] || [ ${ADDR[index]} == "\_" ]
        then
            getIndex ADDR $(($index + 1))
        else
            i=$index
            return $index
        fi
    }
    
    getIndex ADDR 9
    if [ $ppid != "PPID" ] && [ $pid != "PID" ] && [ ${ADDR[i]} != "COMMAND" ]
    then
        echo "$ppid $pid ${ADDR[i]}" >> $1$2
    fi
done

echo "stop" >> $1$2

IFS=' '

exit 0
