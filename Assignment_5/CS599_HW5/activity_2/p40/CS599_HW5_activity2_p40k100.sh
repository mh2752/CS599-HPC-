#!/bin/bash
#SBATCH --job-name=CS599_HW5_act2_p40_k100         
#SBATCH --output=/scratch/mh2752/CS599_HW5_act2_p40_k100.out	
#SBATCH --error=/scratch/mh2752/CS599_HW5_act2_p40_k100.err
#SBATCH --time=5:00				# Max limit: 5 min
#SBATCH --mem=10240 # 10 GB 
#SBATCH --nodes=2
#SBATCH --ntasks=40
#SBATCH --exclusive
#SBATCH -C sl






# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc -O3 /home/mh2752/CS599_HPC/Assignment_5/activity_2/CS599_HW5_act2.c -lm -o /home/mh2752/CS599_HPC/Assignment_5/activity_2/CS599_HW5_act2_p40_k100

# Run the executable:
srun --ntasks-per-node=20 -n40 -N2 /home/mh2752/CS599_HPC/Assignment_5/activity_2/CS599_HW5_act2_p40_k100 5159737 2 100 iono_57min_5.16Mpts_2D.txt





