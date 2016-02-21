#!/bin/sh

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./bin/
export PYTHONPATH=$PYTHONPATH:.
./bin/AutoFlight
