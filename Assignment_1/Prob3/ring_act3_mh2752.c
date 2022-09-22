/*
	CS-599(HPC, Spring-2022): Assignment 1, Problem 3

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



	  	// If invalid input flag is NOT true:
	  	else
	  	{	

	  		// Initialize local counter and buffer:
	  		local_counter = 0;
	  		recv_buffer = 0;

	  		int loop_controller = 0;

	  		MPI_Request req_obj;


	  		while(loop_controller < 10)
	  		{
	  			// Perform a non-blocking send:
	  			MPI_Isend(&my_rank,1,MPI_INT,((my_rank+1)%nprocs),0,MPI_COMM_WORLD,&req_obj);

	  			// Perform a recv:

	  				// For rank 0 only:
		  			if(my_rank == 0)
		  			{
		  				// Receive from the last rank in the ring:
		  				MPI_Recv(&recv_buffer,1,MPI_INT,(nprocs-1),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);	
		  			}

		  			// For all other ranks:
		  			else
		  			{
		  				// Receive from your previous rank:
		  				MPI_Recv(&recv_buffer,1,MPI_INT,((my_rank-1)%nprocs),0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		  			}

	  			// Add to buffer:
	  			local_counter += recv_buffer;

	  			// Increment loop controller:
	  			loop_controller++;
	  		}


	  		// Print local counter value:
	  		fprintf(stdout,"[OUTPUT] Process %d: Value in local counter is %d\n",my_rank,local_counter);
			fflush(stdout);
	  		
	  		// Finalize:
	  		MPI_Finalize();
	  	}



	return 0;


}