#!/bin/bash
#SBATCH --job-name=random_act5_mh2752         
#SBATCH --output=/scratch/mh2752/CS599_HPC_A1_P5.out	
#SBATCH --error=/scratch/mh2752/CS599_HPC_A1_P5.err
#SBATCH --time=02:00				# 2 min
#SBATCH --mem=2000 
#SBATCH --nodes=1
#SBATCH --ntasks=50 
#SBATCH --cpus-per-task=1




# To execute on your local machine/node:
# A correct example: mpirun -np 50 -hostfile myhostfile.txt ./random_act5_mh2752 50
#mpirun -np 50 -hostfile myhostfile.txt ./random_act5_mh2752 50


# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc /home/mh2752/CS599_HPC/Assignment_1/Prob5/random_act5_mh2752.c -lm -o /home/mh2752/CS599_HPC/Assignment_1/Prob5/random_act5_mh2752

srun /home/mh2752/CS599_HPC/Assignment_1/Prob5/random_act5_mh2752 50

