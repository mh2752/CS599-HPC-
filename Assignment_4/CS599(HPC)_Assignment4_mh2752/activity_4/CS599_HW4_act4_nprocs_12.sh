#!/bin/bash
#SBATCH --job-name=CS599_HW4_act4_nprocs_12         
#SBATCH --output=/scratch/mh2752/CS599_HW4_act4_nprocs_12.out	
#SBATCH --error=/scratch/mh2752/CS599_HW4_act4_nprocs_12.err
#SBATCH --time=15:00				# Max limit: 15 min
#SBATCH --mem=20480 # 20 GB 
#SBATCH --nodes=4
#SBATCH --ntasks=12
#SBATCH --mail-type=ALL	# Get email notifications for all states
#SBATCH --exclusive
#SBATCH -C sl





# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpic++ -O3 /home/mh2752/CS599_HPC/Assignment_4/activity_4/CS599_HW4_act4.cpp -lm -o /home/mh2752/CS599_HPC/Assignment_4/activity_4/CS599_HW4_act4_nprocs_12

# Run the executable:
srun --ntasks-per-node=3 -n12 -N4 /home/mh2752/CS599_HPC/Assignment_4/activity_4/CS599_HW4_act4_nprocs_12 2000000 100000





