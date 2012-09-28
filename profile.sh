#!/bin/bash

python2.7 -m cProfile -o profile gltest.py 
python2.7 -m pstats profile

