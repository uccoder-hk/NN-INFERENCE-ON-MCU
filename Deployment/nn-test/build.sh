#!/bin/bash

usage() {
        cat <<EOF
USAGE: `basename $0` [OPTIONS]
OPTIONS:
	kws

For examples:
        `basename $0` kws
EOF
}

case $1 in
	kws*)
		mbed compile -m DISCO_F746NG -t GCC_ARM \
		  --source . \
		  --source ../Source \
		  --source ../Examples/realtime_test \
		  --source ../CMSIS_5/CMSIS/NN/Include \
		  --source ../CMSIS_5/CMSIS/NN/Source \
		  --source ../CMSIS_5/CMSIS/DSP/Include \
		  --source ../CMSIS_5/CMSIS/DSP/Source \
		  --source ../CMSIS_5/CMSIS/Core/Include \
		  --profile ../release_O3.json -j 8
		;;
	*)
		usage 1>&2
		exit 1
		;;
esac
