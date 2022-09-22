#!/bin/bash
#SBATCH --job-name=CS599_HW5_act1         
#SBATCH --output=/scratch/mh2752/CS599_HW5_act1.out	
#SBATCH --error=/scratch/mh2752/CS599_HW5_act1.err
#SBATCH --time=5:00				# Max limit: 5 min
#SBATCH --mem=20480 # 20 GB 
#SBATCH --nodes=1
#SBATCH --ntasks=11
#SBATCH --cpus-per-task=1
#SBATCH --exclusive
#SBATCH -C sl






# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc -O3 /home/mh2752/CS599_HPC/Assignment_5/activity_1/CS599_HW5_act1.c -lm -o /home/mh2752/CS599_HPC/Assignment_5/activity_1/CS599_HW5_act1

# Run the executable:
srun /home/mh2752/CS599_HPC/Assignment_5/activity_1/CS599_HW5_act1 5159737 2 2 iono_57min_5.16Mpts_2D.txt





