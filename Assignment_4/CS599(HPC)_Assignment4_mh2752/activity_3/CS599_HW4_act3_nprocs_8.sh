#!/bin/bash
#SBATCH --job-name=CS599_HW4_act3_nprocs_8         
#SBATCH --output=/scratch/mh2752/CS599_HW4_act3_nprocs_8.out	
#SBATCH --error=/scratch/mh2752/CS599_HW4_act3_nprocs_8.err
#SBATCH --time=15:00				# Max limit: 15 min
#SBATCH --mem=20480 # 20 GB 
#SBATCH --nodes=2
#SBATCH --ntasks=20
#SBATCH --mail-type=ALL	# Get email notifications for all states
#SBATCH --exclusive
#SBATCH -C sl





# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpic++ -O3 /home/mh2752/CS599_HPC/Assignment_4/activity_3/CS599_HW4_act3.cpp -lm -o /home/mh2752/CS599_HPC/Assignment_4/activity_3/CS599_HW4_act3_nrpocs_8

# Run the executable:
srun --ntasks-per-node=4 -n8 -N2 /home/mh2752/CS599_HPC/Assignment_4/activity_3/CS599_HW4_act3_nrpocs_8 2000000 100000





