#!/bin/bash

# To execute on your local machine/node:
# A correct example: mpirun -np 50 -hostfile myhostfile.txt ./random_act5_mh2752 50
mpirun -np 50 -hostfile myhostfile.txt ./random_act5_mh2752 50

