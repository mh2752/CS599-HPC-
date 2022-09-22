/*
	CS-599(HPC, Spring-2022): Assignment 1, Problem 1

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
			printf("[CMD_LINE_ERR] You need to pass the required number of command line inputs.\n");	
		}
		
		MPI_Finalize();
		return -1;
	}


  	// Making sure we operate with even number
  	// of process ranks:

  		int user_specified_num_ranks = atoi(argv[1]);

  		// Helps in finalizing in case of invalid
  		// number of process rank inputs:
  		unsigned int invalid_input_flag = 0;

	  	
	  	if((nprocs != user_specified_num_ranks) || (nprocs%2 != 0))
	  	{	
	  		if(my_rank==0)
	  		{
	  			fprintf(stdout,"[ABORT] Uneven number of process ranks specified. Aborting program.\n");
				fflush(stdout);
	  		}
	  		
	  		invalid_input_flag = 1;
	  	}
  		

  		// Go with the message passing
  		// only if invalid input flag is NOT true(1):
  		if(invalid_input_flag != 1)
  		{
  			// We have all the necessary inputs in order.
  			// Proceed with rest of the program:

		  		// Initialie local counter to zero:
		  		local_counter = 0;

		  		// Even ranks sends first and odd ranks
		  		// receives first:

		  			// Even ranks:
		  			if(!(my_rank%2))
		  			{
		  				// Send your rank number to your neighbor (my_rank+1)
		  				// a total of five times:
		  				int send_count = 0;
		  				int send_value = my_rank;

		  				while(send_count < 5)
		  				{
		  					// Send data to your neighbor:
		  					MPI_Send(&send_value,1,MPI_INT,my_rank+1,0,MPI_COMM_WORLD);

		  					// Increment loop counter:
		  					send_count++;
		  				}


		  				// Now receive from your neighbor (my_rank+1)
		  				// a total of five times:
		  				int recv_count = 0;

		  				while(recv_count < 5)
		  				{
		  					// Receive into your buffer (ignore status):
		  					MPI_Recv(&recv_buffer,1,MPI_INT,my_rank+1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

		  					// Add received value to your local counter:
		  					local_counter += recv_buffer;

		  					// Increment loop counter:
		  					recv_count++;
		  				}

		  				// Print the value in your local counter:
		  				fprintf(stdout,"[OUTPUT] Process %d: Value in local counter = %d\n",my_rank,local_counter);
						fflush(stdout);


		  			} // if ends

		  			// Odd ranks:
		  			else
		  			{
		  				// Receive from your neighbor (my_rank-1)
		  				// a total of five times:
		  				int recv_count = 0;

		  				while(recv_count < 5)
		  				{
		  					// Receive into your buffer (ignore status):
		  					MPI_Recv(&recv_buffer,1,MPI_INT,my_rank-1,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

		  					// Add received value to your local counter:
		  					local_counter += recv_buffer;

		  					// Increment loop counter:
		  					recv_count++;
		  				}

		  				// Send your rank number to your neighbor (my_rank-1)
		  				// a total of five times:
		  				int send_count = 0;
		  				int send_value = my_rank;

		  				while(send_count < 5)
		  				{
		  					// Send data to your neighbor:
		  					MPI_Send(&send_value,1,MPI_INT,my_rank-1,0,MPI_COMM_WORLD);

		  					// Increment loop counter:
		  					send_count++;
		  				}

		  				// Print the value in your local counter:
		  				fprintf(stdout,"[OUTPUT] Process %d: Value in local counter = %d\n",my_rank,local_counter);
						fflush(stdout);

		  			} // else ends

		  	// Finalize:
		  	MPI_Finalize();

  		}


  		// In case of invalid input flag
  		// being true:
  		else
  		{  			
  			MPI_Finalize();  		
  		}




  	

	return 0;
}





