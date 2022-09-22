#!/bin/bash
#SBATCH --job-name=CS599_HW5_act2_p36_k2         
#SBATCH --output=/scratch/mh2752/CS599_HW5_act2_p36_k2.out	
#SBATCH --error=/scratch/mh2752/CS599_HW5_act2_p36_k2.err
#SBATCH --time=5:00				# Max limit: 5 min
#SBATCH --mem=10240 # 10 GB 
#SBATCH --nodes=2
#SBATCH --ntasks=36
#SBATCH --exclusive
#SBATCH -C sl






# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc -O3 /home/mh2752/CS599_HPC/Assignment_5/activity_2/CS599_HW5_act2.c -lm -o /home/mh2752/CS599_HPC/Assignment_5/activity_2/CS599_HW5_act2_p36_k2

# Run the executable:
srun --ntasks-per-node=18 -n36 -N2 /home/mh2752/CS599_HPC/Assignment_5/activity_2/CS599_HW5_act2_p36_k2 5159737 2 2 iono_57min_5.16Mpts_2D.txt





