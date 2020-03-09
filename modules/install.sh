#!/bin/bash

########################################################################
# Script used to install all modules.
########################################################################

# compile.
cd parser/ && make && cd ../

# clear debug log.
dmesg --clear

# install parser.
result=sudo insmod parser/parser.ko

# extract ps_parser char device major number.
output=sudo dmesg
echo output

#install reader.
result=sudo insmod reader/reader.ko

#install keylog.
result=sudo insmod keylog/keylog.ko

#install analyser.
result=sudo insmod analyser/analyser.ko

echo "Install ready."
