#!/bin/bash

# To execute on your local machine/node:
# A correct example: mpirun -np 8 -hostfile myhostfile.txt ./pingpong_act1_mh2752 8
mpirun -np 8 -hostfile myhostfile.txt ./pingpong_act1_mh2752 8
