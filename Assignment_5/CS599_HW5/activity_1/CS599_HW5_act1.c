/*
  CS-599(HPC, Spring-2022)

  Assignment-5

  Author:

    Md Nazmul Hossain (mh2752@nau.edu)

*/

#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>



#define KMEANSITERS 10
#define DEBUG_PRINTS 1

//compile
//mpicc kmeans.c -lm -o kmeans

//run example with 2 means
//mpirun -np 4 -hostfile myhostfile.txt ./kmeans 5159737 2 2 iono_57min_5.16Mpts_2D.txt

//function prototypes
int importDataset(char * fname, int DIM, int N, double ** dataset);



// *********************************** main(...) ***********************************************************************************************************

int main(int argc, char **argv) 
{

  int my_rank, nprocs;

  MPI_Init(&argc,&argv);
  MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
  MPI_Comm_size(MPI_COMM_WORLD,&nprocs);


  //Process command-line arguments
  int N;
  int DIM;
  int KMEANS;
  char inputFname[500];


  if (argc != 5) 
  {
    fprintf(stderr,"Please provide the following on the command line: N (number of lines in the file), dimensionality (number of coordinates per point/feature vector), K (number of means), dataset filename. Your input: %s\n",argv[0]);
    MPI_Finalize();
    exit(0);
  }

  sscanf(argv[1],"%d",&N);
  sscanf(argv[2],"%d",&DIM);
  sscanf(argv[3],"%d",&KMEANS);
  strcpy(inputFname,argv[4]);
  
  // Pointer to entire dataset
  double **dataset;  

  if (N<1 || DIM <1 || KMEANS < 1)
  {
    printf("\nOne of the followings is invalid: N, DIM, K(MEANS)\n");
    MPI_Finalize();
    exit(0);
  }

  //All ranks import dataset
  else
  {
   
    if (my_rank==0)
    {
      fprintf(stdout,"\nNumber of lines (N): %d, Dimensionality: %d, KMEANS: %d, Filename: %s\n", N, DIM, KMEANS, inputFname);
      fflush(stdout);
    }
  

    //allocate memory for dataset
    dataset=(double**)malloc(sizeof(double*)*N);
    for (int i=0; i<N; i++)
    {
      dataset[i]=(double*)malloc(sizeof(double)*DIM);
    }

    int ret=importDataset(inputFname, DIM, N, dataset);

    if (ret==1)
    {
      MPI_Finalize();
      return 0;
    }

  }  

  MPI_Barrier(MPI_COMM_WORLD);
  
  
  // --------------------------------------------------- Write code here --------------------------------------------------------------------------

    
    // Starting timestamp for total response time:
    double total_response_start = MPI_Wtime();

    double total_distance_calc_sum = 0.0; // Incrementally add
    double total_centroid_update_sum = 0.0; // Incrementally add



    // ************ Allocating rows by rank 0 **************

      int lower_index_limit[nprocs];
      int upper_index_limit[nprocs];

      if(my_rank == 0)
      {
        
        // If N is divisible by nprocs:
        if(N%nprocs == 0)
        {
          int row_range = (int)(N/nprocs);

          for(int process_rank = 0, previous_rank_end = -1; process_rank < nprocs; process_rank++)
          {

            lower_index_limit[process_rank] = previous_rank_end + 1;
            upper_index_limit[process_rank] = lower_index_limit[process_rank] + row_range - 1;

            // Update the previous_rank_end index value with the
            // upper limit of this process rank:
            previous_rank_end = upper_index_limit[process_rank];
          }
        }

        // N is not divisible by nprocs:
        else
        {
          // We allocate the reaminder rows to the last process rank:
          int row_range = (int)(N/nprocs);

          for(int process_rank = 0, previous_rank_end = -1; process_rank < nprocs; process_rank++)
          {
            // Not the last rank:
            if(process_rank < nprocs-1)
            {
              lower_index_limit[process_rank] = previous_rank_end + 1;
              upper_index_limit[process_rank] = lower_index_limit[process_rank] + row_range;

              // Update the previous_rank_end index value with the
              // upper limit of this process rank:
              previous_rank_end = upper_index_limit[process_rank];

            }
            
            // It's the last rank:
            else
            {
              lower_index_limit[process_rank] = previous_rank_end + 1;
              upper_index_limit[process_rank] = N-1; // Last rank takes in all the reamining rows
            }
          }
        }

      } // Calculating rows allocation ends

      // Braodcasting the allocation from rank 0 to all the other ranks:
      MPI_Bcast(lower_index_limit,nprocs,MPI_INT,0,MPI_COMM_WORLD); // lower limits
      MPI_Bcast(upper_index_limit,nprocs,MPI_INT,0,MPI_COMM_WORLD); // upper limits

      // Printing the row_ranges for sanity check:
      if(DEBUG_PRINTS && 0)
      {
        fprintf(stdout,"\n\t[DEBUG-1] Process %d, lower_limit = %d, upper_limit = %d\n",my_rank,lower_index_limit[my_rank],upper_index_limit[my_rank]);
        fflush(stdout);
      }



    // ************* K-means preliminary **************


      // Total number of allocated rows to this rank:
      int num_of_allocated_rows = upper_index_limit[my_rank] - lower_index_limit[my_rank] + 1;

      // Array for holding the cluster allotment information for the points:
      int local_cluster_allotment_of_points[num_of_allocated_rows];

      // Sizes of the local clusters:
      int local_cluster_sizes[KMEANS];
      // Centroids of the local clusters:
      double local_cluster_centroids[KMEANS][DIM];


      // Sizes of the remote clusters:
      int remote_cluster_sizes[nprocs][KMEANS];
      // Centroids of remote clusters:
      double remote_cluster_centroids[nprocs][KMEANS][DIM];


      // 'Global' size of the clusters. Should have same values
      // across all process ranks:
      int global_cluster_sizes[KMEANS];
      // 'Global' cluster centroids. Intiialized with the first
      // KMEANS points from the read dataset:
      double global_cluster_centroids[KMEANS][DIM];
      for(int cluster = 0; cluster < KMEANS; cluster++)
      {
        for(int dimension = 0; dimension < DIM; dimension++)
        {
          global_cluster_centroids[cluster][dimension] = dataset[cluster][dimension];
        }
      }


      // Arrays holding sum of coordinates of the points in 
      // individual clusters:
      double local_cluster_points_sums[KMEANS][DIM];


    // *********** K-means cluster primary operation ************

      for(int kmeans_iteration = 0; kmeans_iteration < KMEANSITERS; kmeans_iteration++)
      {

        // ************************************* Assign points to the closest cluster ******************************        

          // Initialize local_cluster_points_sums to zeroes:
          for(int cluster = 0; cluster < KMEANS; cluster++)
          {
            for(int dimension = 0; dimension < DIM; dimension++)
            {
              local_cluster_points_sums[cluster][dimension] = 0.0;
            }
          }

          // initalize local cluster sizes to zeroes:
          for(int cluster = 0; cluster < KMEANS; cluster++)
          {
            local_cluster_sizes[cluster] = 0;
          }


          for(int current_row = lower_index_limit[my_rank]; current_row <= upper_index_limit[my_rank]; current_row++)
          {
            // Utility variables:
            double distance_from_centroid, previous_distance_from_centroid, squared_distance;
            int assigned_cluster = -1;
            
            // Initialize utility variables:
            distance_from_centroid = 0.0;
            previous_distance_from_centroid = 0.0;


            // Calculate distance from each cluster centroid iteratively
            // and compare and assign to the approapriate cluster:
            double temp_dist_start = MPI_Wtime();

            for(int cluster = 0; cluster < KMEANS; cluster++)
            {
              squared_distance = 0.0;
              for(int dimension = 0; dimension < DIM; dimension++)
              {
                squared_distance += pow(global_cluster_centroids[cluster][dimension] - dataset[current_row][dimension],2);
              }

              // Obtain the square root of distance:
              distance_from_centroid = sqrt(squared_distance);

              if(cluster == 0)
              {
                assigned_cluster = cluster;
                previous_distance_from_centroid = distance_from_centroid;
              }

              else
              {
                if(distance_from_centroid < previous_distance_from_centroid)
                {
                  assigned_cluster = cluster;
                  previous_distance_from_centroid = distance_from_centroid;
                }
              }
            } // Distance calculation is done

            double temp_dist_end = MPI_Wtime();

            // Add to the cumulative total:
            total_distance_calc_sum += temp_dist_end - temp_dist_start;

            double temp_update_start = MPI_Wtime();
            
            // Update this points cluster assignemnt info:
            local_cluster_allotment_of_points[current_row - lower_index_limit[my_rank]] = assigned_cluster;
            // Update local cluster size:
            local_cluster_sizes[assigned_cluster] += 1;
            // Update local_cluster_sum:
            for(int dimension = 0; dimension < DIM; dimension++)
            {
              local_cluster_points_sums[assigned_cluster][dimension] += dataset[current_row][dimension];
            }
            
            double temp_update_end = MPI_Wtime();
            total_centroid_update_sum += temp_update_end - temp_update_start;

          } // for( .... current_row....) ends here
         


        // ************************** Send local cluster sizes to other ranks *****************************************

          double temp_update_start = MPI_Wtime();
          // A dummy request object:
          MPI_Request dummy_request;
          // Use MPI_ISend() to send to remote ranks:
          for(int remote_process = 0; remote_process < nprocs; remote_process++)
          {
            // Don't need to send to self. Already have
            // the data:
            if(remote_process != my_rank)
            {
              MPI_Isend(local_cluster_sizes,KMEANS,MPI_INT,remote_process,0,MPI_COMM_WORLD,&dummy_request);
            }
          }

        // ************************* Recieve remotes' local cluster sizes **********************************************

          // Use MPI_Recv() to recieve the data:
          for(int remote_process = 0; remote_process < nprocs; remote_process++)
          {
            // We do not recieve from self:
            if(remote_process != my_rank)
            {
              MPI_Recv(&remote_cluster_sizes[remote_process][0],KMEANS,MPI_INT,remote_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            }

          }

          double temp_update_end = MPI_Wtime();

          total_centroid_update_sum += temp_update_end - temp_update_start;
         
        // *********************** Calculate global cluster sizes ******************************************************

          // Combine local cluster sizes and remote cluster sizes:

          // Copy local cluster sizes first:
          for(int cluster = 0; cluster < KMEANS; cluster++)
          {
            global_cluster_sizes[cluster] = local_cluster_sizes[cluster];
          }

          // Add remote process' local cluster sizes:
          for(int remote_process = 0; remote_process < nprocs; remote_process++)
          {
            if(remote_process != my_rank)
            {
              for(int cluster = 0; cluster < KMEANS; cluster++)
              {
                global_cluster_sizes[cluster] += remote_cluster_sizes[remote_process][cluster];
              }
            }
          }


        // ******************** Compute local weighted means, a.k.a, local centroids **********************************************************

          for(int cluster = 0; cluster < KMEANS; cluster++)
          {
            for(int dimension = 0; dimension < DIM; dimension++)
            {
              local_cluster_centroids[cluster][dimension] = (double)(local_cluster_points_sums[cluster][dimension] / (double)global_cluster_sizes[cluster]);
            }
          }
         

        // ****************** Send local clusters weighted means(centroids) to remote process **********************************************************
          
          // Use MPI_ISend() to send to remote ranks:
          for(int remote_process = 0; remote_process < nprocs; remote_process++)
          {
            // Don't need to send to self. Already have
            // the data:
            if(remote_process != my_rank)
            {
              MPI_Isend(local_cluster_centroids,KMEANS*DIM,MPI_DOUBLE,remote_process,0,MPI_COMM_WORLD,&dummy_request);
            }
          }

        // ************** Recieve remote clusters weighted means (centroids) ***************************************************************************

          // Use MPI_Recv() to recieve the data:
          for(int remote_process = 0; remote_process < nprocs; remote_process++)
          {
            // We do not recieve from self:
            if(remote_process != my_rank)
            {
              MPI_Recv(remote_cluster_centroids[remote_process],KMEANS*DIM,MPI_DOUBLE,remote_process,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            }

          }

         
        
        // *********** Combine local centroids & remote centroids to get global centroids ************************************************************

          // Copy local centroids to global first:
          for(int cluster = 0; cluster < KMEANS; cluster++)
          {
            for(int dimension = 0; dimension < DIM; dimension++)
            {
              global_cluster_centroids[cluster][dimension] = local_cluster_centroids[cluster][dimension];
            }
          }

          // Add the remote centroids:
          for(int remote_process = 0; remote_process < nprocs; remote_process++)
          {
            if(remote_process != my_rank)
            {
              for(int cluster = 0; cluster < KMEANS; cluster++)
              {
                for(int dimension = 0; dimension < DIM; dimension++)
                {
                  global_cluster_centroids[cluster][dimension] += remote_cluster_centroids[remote_process][cluster][dimension];
                }
              }
            }
          }         

          // ****************** Print (to .txt file) centroid values at second last iteration *********************************************************************
          if(kmeans_iteration == KMEANSITERS-2 && my_rank == 0)
          {
            
            // Prepare the filename:
            char file_name[512];
            char temp[] = "centroids_";
            for(int index = 0; index < 11; index++)
            {
              if(index != 10)
              {
                file_name[index] = temp[index];
              }
              else
              {
                file_name[index] = '\0';
              }
            }
            char buffer[256];
            sprintf(buffer,"%d",nprocs);
            char buffer2[256];
            sprintf(buffer2,"%d",KMEANS);

            strcat(file_name,buffer);
            strcat(file_name,buffer2);
            strcat(file_name,".txt");


            FILE *fp;
            fp = fopen(file_name,"w");

            for(int cluster = 0; cluster < KMEANS; cluster++)
            {
              for(int dimension = 0; dimension < DIM; dimension++)
              {
                if(dimension != DIM-1)
                {
                  fprintf(fp,"%lf,",global_cluster_centroids[cluster][dimension]);
                  fflush(fp);
                }

                else
                {
                  fprintf(fp,"%lf",global_cluster_centroids[cluster][dimension]);
                  fflush(fp);
                }
              }

              if(cluster != KMEANS-1)
              {
                fprintf(fp,"\n");
                fflush(fp);
              }
              
            } 

            fclose(fp);     
          
          } 


        MPI_Barrier(MPI_COMM_WORLD);

      }


  // --------------------------------------------------- Finsih writing code here --------------------------------------------------------------------
  
  // Ending timestamp for total response time:
  double total_response_end = MPI_Wtime();

  double total_response = total_response_end - total_response_start;


  // For reducing the times:
  double max_total;
  double max_dist_calc;
  double max_centroid_update;


  // Reduce the total time:
  MPI_Reduce(&total_response,&max_total,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
  
  // Reduce the dist calc time:
  MPI_Reduce(&total_distance_calc_sum,&max_dist_calc,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);

  // Reduce the centroid update time:
  MPI_Reduce(&total_centroid_update_sum,&max_centroid_update,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);


  if(my_rank == 0)
  {
    fprintf(stdout,"\n\tTotal response = %lf, total dist calc = %lf, total cumu update = %lf\n",max_total,max_dist_calc,max_centroid_update);
    fflush(stdout);
  }
  
  
  
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Finalize();

  // Free all dynamically allocated memories:
  for (int i=0; i<N; i++)
  {
    free(dataset[i]);
  }
  free(dataset);

  

  return 0;
}






int importDataset(char * fname, int DIM, int N, double ** dataset)
{

    FILE *fp = fopen(fname, "r");

    if (!fp) {
        printf("Unable to open file\n");
        return(1);
    }

    char buf[4096];
    int rowCnt = 0;
    int colCnt = 0;
    while (fgets(buf, 4096, fp) && rowCnt<N) {
        colCnt = 0;

        char *field = strtok(buf, ",");
        double tmp;
        sscanf(field,"%lf",&tmp);
        dataset[rowCnt][colCnt]=tmp;

        
        while (field) {
          colCnt++;
          field = strtok(NULL, ",");
          
          if (field!=NULL)
          {
          double tmp;
          sscanf(field,"%lf",&tmp);
          dataset[rowCnt][colCnt]=tmp;
          }   

        }
        rowCnt++;
    }

    fclose(fp);
    return 0;


}


