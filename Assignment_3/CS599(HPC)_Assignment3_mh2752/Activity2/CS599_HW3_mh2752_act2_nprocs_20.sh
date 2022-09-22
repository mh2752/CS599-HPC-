#!/bin/bash
#SBATCH --job-name=CS599_HW3_mh2752_act2_nprocs_20         
#SBATCH --output=/scratch/mh2752/CS599_HW3_mh2752_act2_nprocs_20.out	
#SBATCH --error=/scratch/mh2752/CS599_HW3_mh2752_act2_nprocs_20.err
#SBATCH --time=15:00				# Max limit: 15 min
#SBATCH --mem=81920 # 80 GB 
#SBATCH --nodes=1
#SBATCH --ntasks=20   # Number of processes 
#SBATCH --cpus-per-task=1
#SBATCH --mail-type=ALL	# Get email notifications for all states
#SBATCH --exclusive
#SBATCH -C sl





# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc -O3 /home/mh2752/CS599_HPC/Assignment_3/Activity_2/distribution_sort_exponential_mh2752.c -lm -o /home/mh2752/CS599_HPC/Assignment_3/Activity_2/distribution_sort_exponential_mh2752_nprocs_20

# Run the executable:
srun /home/mh2752/CS599_HPC/Assignment_3/Activity_2/distribution_sort_exponential_mh2752_nprocs_20 





