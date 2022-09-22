/*
	CS-599(HPC, Spring-2022): Assignment 1, Problem 2

	Author:

		Nazmul (mh2752@nau.edu)

*/

#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>

// #################################################################################################


int main(int argc,char **argv)
{	


	// Each process' local variables:
	int my_rank, nprocs, recv_buffer, local_counter;


	// Initializing and calling necessary MPI
	// functions:
	MPI_Init(&argc,&argv);  	
  	MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  	MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

  	// If correct number of command line
	// inputs are not supplied:
	if(argc != 2)
	{
		if(my_rank == 0)
		{
			fprintf(stdout,"[CMD_LINE_ERR] You need to pass the required number of command line inputs.\n");
			fflush(stdout);	
		}
		
		MPI_Finalize();
		return -1;
	}



	int user_specified_num_ranks = atoi(argv[1]);

  		// Helps in finalizing in case of invalid
  		// number of process rank inputs:
  		unsigned int invalid_input_flag = 0;

	  	
	  	if((user_specified_num_ranks <=1) || (nprocs != user_specified_num_ranks))
	  	{	
	  		if(my_rank==0)
	  		{
	  			fprintf(stdout,"[ABORT] Number of processes should be greater than 1.\n");
				fflush(stdout);
	  		}
	  		
	  		invalid_input_flag = 1;
	  	}

	  	if(invalid_input_flag==1)
	  	{
	  		MPI_Finalize();
	  	}

	  	else
	  	{	

	  		// Initialize local counter and buffer:
	  		local_counter = 0;
	  		recv_buffer = 0;

	  		int loop_controller = 0;

	  		// Process 0:
	  		if(my_rank == 0)
	  		{
	  			while(loop_controller < 10)
	  			{

	  				// Send to your next rank:
	  				MPI_Send(&my_rank,1,MPI_INT,((my_rank+1)%nprocs),0,MPI_COMM_WORLD);

	  				// Receive from your previous rank:
	  				MPI_Recv(&recv_buffer,1,MPI_INT,(nprocs-1),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

	  				// Add to your local counter:
	  				local_counter += recv_buffer;

	  				// Increment counter:
	  				loop_controller++;
	  			}


	  			// Print local_counter value:
	  			fprintf(stdout,"[OUTPUT] Process %d: Value in local counter = %d\n",my_rank,local_counter);
				fflush(stdout);

	  		}

	  		// All other processes:
	  		else
	  		{
	  			while(loop_controller < 10)
	  			{

	  				
	  				// Receive from your previous rank:
	  				MPI_Recv(&recv_buffer,1,MPI_INT,((my_rank-1)%nprocs),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

	  				// Add to your local counter:
	  				local_counter += recv_buffer;

	  				// Send to your next rank:
	  				MPI_Send(&my_rank,1,MPI_INT,((my_rank+1)%nprocs),0,MPI_COMM_WORLD);


	  				// Increment counter:
	  				loop_controller++;
	  			}


	  			// Print local_counter value:
	  			fprintf(stdout,"[OUTPUT] Process %d: Value in local counter = %d\n",my_rank,local_counter);
				fflush(stdout);


	  		}

	  		// Finalize:
	  		MPI_Finalize();
	  	}


	return 0;


}