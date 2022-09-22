#!/bin/bash
#SBATCH --job-name=ping_pong_act1           
#SBATCH --output=/scratch/mh2752/CS599_HPC_A1_P1_Act1.out	
#SBATCH --error=/scratch/mh2752/CS599_HPC_A1_P1_Act1.err
#SBATCH --time=02:00				# 2 min
#SBATCH --mem=2000 
#SBATCH --nodes=1
#SBATCH --ntasks=8   # Number of processes 
#SBATCH --cpus-per-task=1



# To execute on your local machine/node:
# A correct example: mpirun -np 8 -hostfile myhostfile.txt ./pingpong_act1_mh2752 8
# mpirun -np 8 -hostfile myhostfile.txt ./pingpong_act1_mh2752 8


# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc /home/mh2752/CS599_HPC/Assignment_1/Prob1/pingpong_act1_mh2752.c -lm -o /home/mh2752/CS599_HPC/Assignment_1/Prob1/pingpong_act1_mh2752

srun /home/mh2752/CS599_HPC/Assignment_1/Prob1/pingpong_act1_mh2752 8





