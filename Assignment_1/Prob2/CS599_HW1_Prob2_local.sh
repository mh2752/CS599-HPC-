#!/bin/bash

# To execute on your local machine/node:
# A correct example: mpirun -np 6 -hostfile myhostfile.txt ./ring_act2_mh2752 6
mpirun -np 6 -hostfile myhostfile.txt ./ring_act2_mh2752 6
