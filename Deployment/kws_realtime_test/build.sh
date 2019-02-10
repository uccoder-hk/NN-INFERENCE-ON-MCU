#!/bin/bash

mbed compile -m DISCO_F746NG -t GCC_ARM \
  --source . --source ../Source --source ../Examples/realtime_test \
  --source ../CMSIS_5/CMSIS/NN/Include --source ../CMSIS_5/CMSIS/NN/Source \
  --source ../CMSIS_5/CMSIS/DSP/Include --source ../CMSIS_5/CMSIS/DSP/Source \
  --source ../CMSIS_5/CMSIS/Core/Include \
  --profile ../release_O3.json -j 8
