#!/bin/bash
#
# sw_test.sh
#
#  Copyright (c) 2010, Jim Sukha
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the authors nor the names of its
#       contributors may be used to endorse or promote products
#       derived from this software without specific prior written
#       permission.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
#  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
#  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
#  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
#  AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
#  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
#  WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#  POSSIBILITY OF SUCH DAMAGE.

#  
# Sample script for running Smith-Waterman dynamic program.
# 
# This simple bash script runs tests for various matrix sizes, block
# sizes, and test types.
#

# Maximum number of processors to test.
maxP=8

# Which test types to run.
# (See SWComputeType enum in sw_compute.cpp for details).
maxTestType=5

B=16
numReps=10


for N in 1000 #2000 5000 # 15000 
do
  for ((P=$maxP; P>=1 ;P-=1)) do
  echo "-----------------------N=$N-----------------------------"
    M=$N
    echo "***********N = $N, B=$B ********************"
    for test_type in 1 2 3 4 # 5  
    do
      for ((k=0; k<=$numReps; k+=1)) do
        estring="CILK_NWORKERS=$P ./swblock_$B $N $M $test_type 0"
        echo $estring
        eval $estring
      done
    done
  done
done
