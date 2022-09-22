/*
	CS-599(HPC, Spring-2022): Assignment 1, Problem 4

	Author:

		Nazmul (mh2752@nau.edu)

*/

#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>

// #################################################################################################


//Do not change the seed, or your answer will not be correct
#define SEED 72

//Change this if you want, but make sure it is set to 10 when submitting the assignment
#define TOTALITER 10

int generateRandomRank(int max_rank, int my_rank);


int main(int argc,char **argv)
{	


	// Each process' local variables:
	int my_rank, nprocs, root, recv_buffer, local_counter, target_rank, loop_counter;
	int bcast_buffer[2];

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

	
	int user_specified_num_ranks = atoi(argv[1]);

  	// Helps in finalizing in case of invalid
  	// number of process rank inputs:
  	unsigned int invalid_input_flag = 0;


	  	
	if((user_specified_num_ranks < 3) || (nprocs != user_specified_num_ranks))
	{	
	  	if(my_rank==0)
	  	{
	  		printf("[ABORT] Number of processes should be greater than 2.\n");
	  	}
	  		
	  	invalid_input_flag = 1;
	}

	// Exit gracefully if invalid input received:
	if(invalid_input_flag==1)
	{
	  	MPI_Finalize();
	} 

	// Continue otherwise:
	else
	{	
		//seed rng do not modify
  		srand(SEED+my_rank);
			
		root = 0;		

		if(my_rank == root)
		{
			target_rank = generateRandomRank(nprocs-1,my_rank);
			fprintf(stdout,"[OUTPUT] Master, first rank: %d\n",target_rank);
			fflush(stdout);
			bcast_buffer[0] = target_rank;
			bcast_buffer[1] = my_rank;			
		}

		// Bcast:
		MPI_Bcast(bcast_buffer,2,MPI_INT,root,MPI_COMM_WORLD);

		// Send:
		if(my_rank == root)
		{
			local_counter = 0;
			local_counter += my_rank;
			MPI_Request req;
			
			MPI_Isend(&local_counter,1,MPI_INT,bcast_buffer[0],0,MPI_COMM_WORLD,&req);			
		}
	
		loop_counter = 0;

		while(loop_counter < TOTALITER)
		{
			// Get the target process:
			int tgt = bcast_buffer[0];

			// Get the source process:
			int source = bcast_buffer[1];
			

			// Receieve only if you're the target:
			if(my_rank == tgt)
			{
				MPI_Recv(&recv_buffer,1,MPI_INT,source,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
				local_counter = recv_buffer;

				fprintf(stdout,"\n");
				fflush(stdout);

				fprintf(stdout,"[OUTPUT %d] MY rank: %d, old counter: %d\n",loop_counter,my_rank,local_counter);
				fflush(stdout);

				// Update counter:
				local_counter += my_rank;

				fprintf(stdout,"[OUTPUT %d] MY rank: %d, new counter: %d\n",loop_counter,my_rank,local_counter);
				fflush(stdout);

				// Get the next rank:
				target_rank = generateRandomRank(nprocs-1,my_rank);

				fprintf(stdout,"[OUTPUT %d] MY rank: %d, next to receive: %d\n",loop_counter,my_rank,target_rank);
				fflush(stdout);

				// Update bcast buffers:
				bcast_buffer[0] = target_rank;
				bcast_buffer[1] = my_rank;

				// Send non-blockingly:
				MPI_Request req;
				MPI_Isend(&local_counter,1,MPI_INT,bcast_buffer[0],0,MPI_COMM_WORLD,&req);
								
			}				

			// Broadcast:
			MPI_Bcast(bcast_buffer,2,MPI_INT,tgt,MPI_COMM_WORLD);
			

			// Increment controller:
			loop_counter++;
			// Synchronize:
			//MPI_Barrier(MPI_COMM_WORLD);
		
		}

		MPI_Finalize();

	}
	  	



	return 0;


}





// --------------------------------------- Misc. Function Definition ---------------------------------------------------

//Do not modify the rank generator or you will get the wrong answer
//returns a rank between 1 and max_rank, but does not return itself
//does not generate rank 0
int generateRandomRank(int max_rank, int my_rank)
{
  int tmp=round(max_rank*((double)(rand()) / RAND_MAX));
  while(tmp==my_rank || tmp==0)
  {  
  tmp=round(max_rank*((double)(rand()) / RAND_MAX)); 
  }

  return tmp;
}
