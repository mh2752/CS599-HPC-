#!/bin/bash
#SBATCH --job-name=CS599_HW4_act1_nprocs_4         
#SBATCH --output=/scratch/mh2752/CS599_HW4_act1_nprocs_4.out	
#SBATCH --error=/scratch/mh2752/CS599_HW4_act1_nprocs_4.err
#SBATCH --time=15:00				# Max limit: 15 min
#SBATCH --mem=20480 # 20 GB 
#SBATCH --nodes=1
#SBATCH --ntasks=4   # Number of processes 
#SBATCH --cpus-per-task=1
#SBATCH --mail-type=ALL	# Get email notifications for all states
#SBATCH --exclusive
#SBATCH -C sl





# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc -O3 /home/mh2752/CS599_HPC/Assignment_4/activity_1/CS599_HW4_act1.c -lm -o /home/mh2752/CS599_HPC/Assignment_4/activity_1/CS599_HW4_act1_nrpocs_4

# Run the executable:
srun /home/mh2752/CS599_HPC/Assignment_4/activity_1/CS599_HW4_act1_nrpocs_4 2000000 100000





