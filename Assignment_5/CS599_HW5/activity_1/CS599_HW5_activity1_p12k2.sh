#!/bin/bash
#SBATCH --job-name=CS599_HW5_act1_p12_k2         
#SBATCH --output=/scratch/mh2752/CS599_HW5_act1_p12_k2.out	
#SBATCH --error=/scratch/mh2752/CS599_HW5_act1_p12_k2.err
#SBATCH --time=5:00				# Max limit: 5 min
#SBATCH --mem=10240 # 10 GB 
#SBATCH --nodes=1
#SBATCH --ntasks=12
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH -C sl






# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc -O3 /home/mh2752/CS599_HPC/Assignment_5/activity_1/CS599_HW5_act1.c -lm -o /home/mh2752/CS599_HPC/Assignment_5/activity_1/CS599_HW5_act1_p12_k2

# Run the executable:
srun /home/mh2752/CS599_HPC/Assignment_5/activity_1/CS599_HW5_act1_p12_k2 5159737 2 2 iono_57min_5.16Mpts_2D.txt





