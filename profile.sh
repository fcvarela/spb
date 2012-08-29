#!/bin/bash

python -m cProfile -o profile gltest.py 
python -m pstats profile

