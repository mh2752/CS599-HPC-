#!/bin/bash
#SBATCH --job-name=CS599_HW4_act4_nprocs_20         
#SBATCH --output=/scratch/mh2752/CS599_HW4_act4_nprocs_20.out	
#SBATCH --error=/scratch/mh2752/CS599_HW4_act4_nprocs_20.err
#SBATCH --time=15:00				# Max limit: 15 min
#SBATCH --mem=20480 # 20 GB 
#SBATCH --nodes=4
#SBATCH --ntasks=20
#SBATCH --mail-type=ALL	# Get email notifications for all states
#SBATCH --exclusive
#SBATCH -C sl





# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpic++ -O3 /home/mh2752/CS599_HPC/Assignment_4/activity_4/CS599_HW4_act4.cpp -lm -o /home/mh2752/CS599_HPC/Assignment_4/activity_4/CS599_HW4_act4_nprocs_20

# Run the executable:
srun --ntasks-per-node=5 -n20 -N4 /home/mh2752/CS599_HPC/Assignment_4/activity_4/CS599_HW4_act4_nprocs_20 2000000 100000





