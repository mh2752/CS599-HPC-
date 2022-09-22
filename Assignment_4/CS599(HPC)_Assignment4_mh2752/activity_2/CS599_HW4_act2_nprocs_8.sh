#!/bin/bash
#SBATCH --job-name=CS599_HW4_act2_nprocs_8         
#SBATCH --output=/scratch/mh2752/CS599_HW4_act2_nprocs_8.out	
#SBATCH --error=/scratch/mh2752/CS599_HW4_act2_nprocs_8.err
#SBATCH --time=15:00				# Max limit: 15 min
#SBATCH --mem=20480 # 20 GB 
#SBATCH --nodes=1
#SBATCH --ntasks=8   # Number of processes 
#SBATCH --cpus-per-task=1
#SBATCH --mail-type=ALL	# Get email notifications for all states
#SBATCH --exclusive
#SBATCH -C sl





# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpic++ -O3 /home/mh2752/CS599_HPC/Assignment_4/activity_2/CS599_HW4_act2.cpp -lm -o /home/mh2752/CS599_HPC/Assignment_4/activity_2/CS599_HW4_act2_nrpocs_8

# Run the executable:
srun /home/mh2752/CS599_HPC/Assignment_4/activity_2/CS599_HW4_act2_nrpocs_8 2000000 100000





