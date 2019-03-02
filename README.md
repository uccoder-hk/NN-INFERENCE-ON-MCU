# NN-INFERENCE-ON-MCU
Accelerating Machine Learning Neural Networks on MCU

Examples: 
  1. KWS realtime test
  2. CIFAR-10 test

# Getting Started
## Enviornment
Ubuntu 18.04  
pip install mbed-cli

## Getting Source
git clone https://github.com/uccoder-hk/NN-INFERENCE-ON-MCU.git

## Build
cd NN-INFERENCE-ON-MCU/Deployment/nn-test  
./build.sh (kws | cifar10)

## Flash
./flash.sh (Note: STM32F746NG development kit must be connected to Linux PC)

