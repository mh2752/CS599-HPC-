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


int main(int argc, char **argv) 
{
  int i, my_rank,nprocs;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

  if (nprocs<3)
  {
    if (my_rank==0)
    printf("\nRun with at least 3 ranks.\n\n");
    MPI_Finalize();
    return 0;
  }

  //seed rng do not modify
  srand(SEED+my_rank);

  int target;
  int local_counter = 0;
  int recv_buffer;

  
  if(my_rank == 0)
  {
	  target = generateRandomRank(nprocs-1,my_rank);
	  fprintf(stdout,"[OUTPUT] Master, next rank: %d\n",target);
	  fflush(stdout);

	  MPI_Request req;
	  MPI_Isend(&my_rank,1,MPI_INT,target,0,MPI_COMM_WORLD,&req);
  }

  MPI_Bcast(&target,1,MPI_INT,0,MPI_COMM_WORLD);

  i = 0;
  
	while(i < TOTALITER)
	{	
		  int bcast_root = target;
		  
		  if(my_rank == target)
		  {
			  	MPI_Recv(&recv_buffer,1,MPI_INT,MPI_ANY_SOURCE,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

				fprintf(stdout,"\n");
				fflush(stdout);

				local_counter = recv_buffer;
				fprintf(stdout,"[OUTPUT %d] My rank: %d, old counter: %d\n",i,my_rank,local_counter);
				fflush(stdout);

				local_counter += my_rank;
				fprintf(stdout,"[OUTPUT %d] My rank: %d, new counter: %d\n",i,my_rank,local_counter);
				fflush(stdout);

				target = generateRandomRank(nprocs-1,my_rank);
				fprintf(stdout,"[OUTPUT %d] My rank: %d, next to receive: %d\n",i,my_rank,target);
				fflush(stdout);

				MPI_Request req;
	  			MPI_Isend(&local_counter,1,MPI_INT,target,0,MPI_COMM_WORLD,&req);
		  }		  

		  MPI_Bcast(&target,1,MPI_INT,bcast_root,MPI_COMM_WORLD);

		  // Synchronize:
		  MPI_Barrier(MPI_COMM_WORLD);

		  // Incrtement i:
		  i++;
	}  

  MPI_Finalize();
  
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
