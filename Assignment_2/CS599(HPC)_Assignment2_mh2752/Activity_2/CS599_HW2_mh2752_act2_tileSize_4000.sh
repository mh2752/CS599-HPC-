#!/bin/bash
#SBATCH --job-name=CS599_HW2_mh2752_distance_act2_tileSize_4000           
#SBATCH --output=/scratch/mh2752/CS599_HW2_mh2752_distance_act2_tileSize_4000.out	
#SBATCH --error=/scratch/mh2752/CS599_HW2_mh2752_distance_act2_tileSize_4000.err
#SBATCH --time=5:00				# Max limit: 5 min
#SBATCH --mem=87040
#SBATCH --nodes=1
#SBATCH --ntasks=20   # Number of processes 
#SBATCH --cpus-per-task=1
#SBATCH --mail-type=ALL	# Get email notifications for all states
#SBATCH --exclusive
#SBATCH -C sl



# --------------------------------- Below lines for running on MONSOON ------------------------------------------------------------------------------

module load openmpi

mpicc -O3 /home/mh2752/CS599_HPC/Assignment_2/Activity_2/distance_act2_mh2752.c -lm -o /home/mh2752/CS599_HPC/Assignment_2/Activity_2/distance_act2_mh2752_tile4000

# Commandline arguments: <Number_of_data_points> <dimension_of_each_data_points> <blocksize_for_tiling> <dataset_filename(.txt format)>
srun /home/mh2752/CS599_HPC/Assignment_2/Activity_2/distance_act2_mh2752_tile4000 100000 90 4000 MSD_year_prediction_normalize_0_1_100k.txt





