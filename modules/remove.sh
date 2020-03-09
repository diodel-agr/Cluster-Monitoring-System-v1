#!/bin/bash

########################################################################
# Script used to remove all modules.
########################################################################

#remode analyser.
sudo rmmod analyser

# remove keylog.
sudo rmmod keylog

#remove reader.
sudo rmmod reader

# remove parser.
sudo rmmod parser
