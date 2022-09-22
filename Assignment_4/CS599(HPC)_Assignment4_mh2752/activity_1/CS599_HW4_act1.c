#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//Example compilation
//mpicc -O3 range_query_starter.c -lm -o range_query_starter

//Example execution
//mpirun -np 1 -hostfile myhostfile.txt ./range_query_starter 100 100

struct dataStruct
{
  double x;
  double y;
};

struct queryStruct
{
  double x_min;
  double y_min;
  double x_max;
  double y_max;
};

void generateData(struct dataStruct * data, unsigned int localN);
void generateQueries(struct queryStruct * data, unsigned int localQ, int my_rank);


//Do not change constants
#define SEED 72
#define MAXVAL 100.0
#define QUERYRNG 10.0 



int main(int argc, char **argv) {

  int my_rank, nprocs;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

  //Process command-line arguments
  int N;
  int Q;
  

  if (argc != 3) {
    fprintf(stderr,"Please provide the following on the command line: <Num data points> <Num query points> \n");
    MPI_Finalize();
    exit(0);
  }

  sscanf(argv[1],"%d",&N);
  sscanf(argv[2],"%d",&Q);
  
  


  
  const unsigned int localN=N;
  const unsigned int localQ=Q/nprocs;

  //local storage for the number of results of each query -- init to 0
  unsigned int * numResults=(unsigned int *)calloc(localQ, sizeof(unsigned int));

  //All ranks generate the same input data
  struct dataStruct * data=(struct dataStruct *)malloc(sizeof(struct dataStruct)*localN);
  generateData(data, localN);

  //All ranks generate different queries
  struct queryStruct * queries=(struct queryStruct *)malloc(sizeof(struct queryStruct)*localQ);
  generateQueries(queries, localQ, my_rank);

  MPI_Barrier(MPI_COMM_WORLD);  


  // ----------------------------------------- Write code here ----------------------------------------------------------------------

  // Getting starting timestamp:
  double time_start = MPI_Wtime();


  for(int query_index = 0; query_index < localQ; query_index++)
  {
    // Get the query object at query_index:
    struct queryStruct temp_query_object = queries[query_index];

    // Compare against data using brute force (linear search) approach:
    for(int data_index = 0; data_index < localN; data_index++)
    {
      // Get the temporary data object:
      struct dataStruct temp_data_object = data[data_index];

      // Check if this data object falls within the
      // seqrch query boundary:
      if(temp_data_object.x >= temp_query_object.x_min && temp_data_object.x <= temp_query_object.x_max && temp_data_object.y >= temp_query_object.y_min && temp_data_object.y <= temp_query_object.y_max)
      {
        // Increment the count:
        numResults[query_index] += 1;
      }
    }

  }
  // Getting ending timestamp:
  double time_end = MPI_Wtime();

  // Getting local response time:
  double local_response_time = (time_end - time_start);

  // Global sum of local response times:
  double global_total_response_time;
  double global_average_response_time;

  // Calling reduction:
  MPI_Reduce(&local_response_time,&global_total_response_time,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);

  if(my_rank == 0)
  {
    global_average_response_time = global_total_response_time/(double)nprocs;
  }

  


  // Computing local sum:
  unsigned int local_sum_of_hits = 0;
  for(int index = 0; index < localQ; index++)
  {
    local_sum_of_hits += numResults[index];
  }

  // Preapring for global sum:
  unsigned int *global_sum_of_hits;
  global_sum_of_hits = (unsigned int*)calloc(1,sizeof(unsigned int));

  // Calling reduction:
  MPI_Reduce(&local_sum_of_hits,global_sum_of_hits,1,MPI_UNSIGNED,MPI_SUM,0,MPI_COMM_WORLD);

  if(my_rank == 0)
  {
    fprintf(stdout,"\n\t[Process %d] Total execution time (averaged over all %d processes) %lf\n",my_rank,nprocs,global_average_response_time);
    fflush(stdout);

    fprintf(stdout,"\n\t\t[Process %d] Global sum of positive search hits = %u\n",my_rank,*global_sum_of_hits);
    fflush(stdout);

  }



  

  

  // --------------------------------------------------------------------------------------------------------------------------------

  MPI_Finalize();

  // De-allocate:
  free(global_sum_of_hits);
  free(data);
  free(queries);


  return 0;
}


//generates data [0,MAXVAL)
void generateData(struct dataStruct * data, unsigned int localN)
{
  //seed rng do not modify
  //Same input dataset for all ranks
  srand(SEED);
  for (int i=0; i<localN; i++)
  {
        data[i].x=((double)rand()/(double)(RAND_MAX))*MAXVAL;      
        data[i].y=((double)rand()/(double)(RAND_MAX))*MAXVAL;      
  }
}

//generates queries
//x_min y_min are in [0,MAXVAL]
//x_max y_max are x_min+d1 y_min+d2
//distance (d1)= [0, QUERYRNG)
//distance (d2)= [0, QUERYRNG)

void generateQueries(struct queryStruct * data, unsigned int localQ, int my_rank)
{
  //seed rng do not modify
  //Different queries for each rank
  srand(SEED+my_rank);
  for (int i=0; i<localQ; i++)
  {
        data[i].x_min=((double)rand()/(double)(RAND_MAX))*MAXVAL;      
        data[i].y_min=((double)rand()/(double)(RAND_MAX))*MAXVAL;
        
        double d1=((double)rand()/(double)(RAND_MAX))*QUERYRNG;      
        double d2=((double)rand()/(double)(RAND_MAX))*QUERYRNG;      
        data[i].x_max=data[i].x_min+d1;      
        data[i].y_max=data[i].y_min+d2;
  }
}
