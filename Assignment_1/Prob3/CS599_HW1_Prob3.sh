#!/bin/bash
#SBATCH --job-name=ring_act3_mh2752         
#SBATCH --output=/scratch/mh2752/CS599_HPC_A1_P3.out	
#SBATCH --error=/scratch/mh2752/CS599_HPC_A1_P3.err
#SBATCH --time=02:00				# 2 min
#SBATCH --mem=2000 
#SBATCH --nodes=1
#SBATCH --ntasks=6 
#SBATCH --cpus-per-task=1

# To execute on your local machine/node:
# A correct example: mpirun -np 6 -hostfile myhostfile.txt ./ring_act3_mh2752 6
#mpirun -np 6 -hostfile myhostfile.txt ./ring_act3_mh2752 6


# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc /home/mh2752/CS599_HPC/Assignment_1/Prob3/ring_act3_mh2752.c -lm -o /home/mh2752/CS599_HPC/Assignment_1/Prob3/ring_act3_mh2752

srun /home/mh2752/CS599_HPC/Assignment_1/Prob3/ring_act3_mh2752 6