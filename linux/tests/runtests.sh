#!/bin/bash
base=/u/c/s/cs537-1/ta/ref/6
python $base/runtests.py --test-path $base $@
exit $?

