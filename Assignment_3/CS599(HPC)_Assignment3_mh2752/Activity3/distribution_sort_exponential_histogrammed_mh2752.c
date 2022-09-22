// **************************************************************************************************************************************************

    /*
        CS599 (HPC): Assignment 3 Problem 3

        Author:

            Nazmul (mh2752@nau.edu)  
    
    
    */

// **************************************************************************************************************************************************

#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>



void generateData(int * data, int SIZE);


int compfn (const void * a, const void * b)
{
  return ( *(int*)a - *(int*)b );
}


//Do not change the seed
#define SEED 72
#define MAXVAL 1000000

//Total input size is N, divided by nprocs
//Doesn't matter if N doesn't evenly divide nprocs
#define N 1000000000

int main(int argc, char **argv) 
{

   int my_rank, nprocs;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);

    //seed rng do not modify
    srand(SEED+my_rank);


    //local input size N/nprocs
    const unsigned int localN = N/nprocs;

    //All ranks generate data
    int *data = (int*) malloc(sizeof(int)*localN);

    // Each rank will generate their own datasets;
    generateData(data, localN);    


    // ------------------------------------------------ Write code here ----------------------------------------------------------------------------

        // Total time start:
        double total_time_start = MPI_Wtime();



        // Lower and upper limits of values for
        // all process (prepared and scattered 
        // by process 0):
        int *process_lower_limit;
        process_lower_limit = (int*)malloc(sizeof(int)*nprocs);

        int *process_upper_limit;
        process_upper_limit = (int*)malloc(sizeof(int)*nprocs);
        

            // ------- Only to be used by Process 0:
            int *data_histogram = NULL;
            double *data_cdf = NULL;
           


        // ----------------- Generating the histogram:

        
        // Distribute time start:
        double distribute_time_start = MPI_Wtime();

        if(my_rank == 0)
        {
           
            data_histogram = (int*)calloc(MAXVAL,sizeof(int));

            // Traverse data and fill up the counts:
            int temp_value;
            for(int index = 0; index < localN; index++)
            {
                temp_value = data[index];
               
                // Increment count value:
                data_histogram[temp_value] += 1;             

            }


            // Calculating CDF of the histogrammed data:            
            data_cdf = (double*)malloc(sizeof(double)*MAXVAL);

            // Traverse histogram and update cdf:
            double temp_count = 0.0;
            for(int index = 0; index < MAXVAL; index++)
            {
                // Get cumulative count:
                temp_count = temp_count + (double)data_histogram[index];

                // Calculate fraction as (temp_count/localN) and store:
                data_cdf[index] = (temp_count/localN);
            }



            // ------- Calculating the upper and lower limits for all processes (including self):            

            double average_data_per_process = ((1.0)/(double)nprocs); // On average, each process should work on this percentage of total data

            // Calculate and store the lower and upper limits
            // of all processes (including self):
            int temp_lower_limit = -1;
            int temp_upper_limit = -1;
            double start_percentage,temp_percentage,target_percentage;

            for(int process_num = 0; process_num < nprocs; process_num++)
            {
                // Current process's lower limit is
                // previous process's upper limit + 1:
                temp_lower_limit = temp_upper_limit + 1;

                start_percentage = data_cdf[temp_lower_limit];

                target_percentage = start_percentage + average_data_per_process;
                

                // Iteratively find the upper limit that ensures
                // the CDF percentage is equal to  or greater than
                // 'average_data_per_process':               
                temp_percentage = 0.0;

                for(temp_upper_limit = temp_lower_limit; temp_upper_limit < MAXVAL; temp_upper_limit++)
                {
                   temp_percentage = data_cdf[temp_upper_limit];

                   if(temp_percentage >= target_percentage)
                   {
                       break;
                   }
                }

                
                if(process_num == nprocs-1)
                {
                    temp_upper_limit = MAXVAL-1; // last process gets whatever data is remaining                   
                }               

                // Store the upper and lower limits:
                process_lower_limit[process_num] = temp_lower_limit;
                process_upper_limit[process_num] = temp_upper_limit;             

            }          
           

           // Free data_cdf and histogram. We do not need them anymore:
           free(data_histogram);
           free(data_cdf);        

        }

        // Now, Bcast the lower and upper limits: 
                 

            // Lower limit:
            MPI_Bcast(&process_lower_limit[0],nprocs,MPI_INT,0,MPI_COMM_WORLD);

           
            // Upper limit:
            MPI_Bcast(&process_upper_limit[0],nprocs,MPI_INT,0,MPI_COMM_WORLD);



        // ------------------------- Send & recieve buckets -------------------------------------------------------------------

        // Array for holding number of genrated items
        // for each process (including itself) by
        // this process:
        int *num_items_generated_for_processes;
        num_items_generated_for_processes = (int*)calloc(nprocs,sizeof(int));

        // Array for holding the number of
        // elements this process is going to
        // receive from others:
        int *num_items_to_recv_from_others;
        num_items_to_recv_from_others = (int*)calloc(nprocs,sizeof(int));
      


        // Finding out the number of items for each process's
        // bucket generated by this process (including itself):
        int temp_item;
        int temp_process;
        int temp_lower_limit, temp_upper_limit;
        long int data_sum = 0;

        for(int index = 0; index < localN; index++)
        {
            temp_item = data[index];

            // Initialize temp_process to an invalid value:
            temp_process = -1;

            // Find out which process's bucket this
            // item should go:
            for(int process_num = 0; process_num < nprocs; process_num++)
            {
                temp_lower_limit = process_lower_limit[process_num];
                temp_upper_limit = process_upper_limit[process_num];

                if( (temp_item >= temp_lower_limit) && (temp_item <= temp_upper_limit) )
                {
                    temp_process = process_num;
                    break;
                }
            }
            // If temp_process is not -1:
            if(temp_process!=-1)
            {
                num_items_generated_for_processes[temp_process] += 1; // Incrementing the count
                data_sum += temp_item;
            }            
        }       
      

        // This process already knows
        // how many items it has generated
        // for itself:
        num_items_to_recv_from_others[my_rank] = num_items_generated_for_processes[my_rank];

      
        // --------- Sending out the number of elements to other processes:
        MPI_Request dummy_req;
        for(int process_num = 0; process_num < nprocs; process_num++)
        {   
            // Don't need to send to self:
            if(process_num != my_rank)
            {
                // Send data using MPI_Isend(...):               
                MPI_Isend(&num_items_generated_for_processes[process_num],1,MPI_INT,process_num,0,MPI_COMM_WORLD,&dummy_req); // Non-blocking???
            }
        }

        // --------- Recv the 'number of elements to recv' from the other processes:
        for(int process_num = 0; process_num < nprocs; process_num++)
        {
            // Don't recv anything from self:
            if(process_num!=my_rank)
            {
                MPI_Recv(&num_items_to_recv_from_others[process_num],1,MPI_INT,process_num,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
            }
        }

     


        // ----------------------------------------- Sending the data to others -------------------------------
           

            // Preparing the data buckets for other processes (including self):
            int **send_buckets;
            send_buckets = (int**) malloc(sizeof(int*)*nprocs);
            
            for(int process_num = 0; process_num < nprocs; process_num++)
            {
                send_buckets[process_num] = (int*)malloc(sizeof(int)*num_items_generated_for_processes[process_num]);
            }

            // Fill the buckets:
                int *temp_process_send_bucket_index_to_write;
                temp_process_send_bucket_index_to_write = (int*)calloc(nprocs,sizeof(int));

                // Traverse the data array linearly and fill the buckets:    
                for(int index = 0; index < localN; index++)
                {
                    temp_item = data[index];

                    // Initialize temp_process to an invalid value:
                    temp_process = -1;

                    // Find out which process's bucket this
                    // item should go:
                    for(int process_num = 0; process_num < nprocs; process_num++)
                    {
                        temp_lower_limit = process_lower_limit[process_num];
                        temp_upper_limit = process_upper_limit[process_num];

                        if( (temp_item >= temp_lower_limit) && (temp_item <= temp_upper_limit) )
                        {
                            temp_process = process_num;
                            break;
                        }
                    }
                    
                    // If temp_process is not -1:
                    if(temp_process!=-1)
                    {
                       // Put the item into the appropirate
                       // bucket:
                       send_buckets[temp_process][temp_process_send_bucket_index_to_write[temp_process]] = temp_item;
                       // Increment the index:
                       temp_process_send_bucket_index_to_write[temp_process] += 1;
                    }                   
                }

               
               
                // Send all the buckets (excluding self):
                for(int process_num = 0; process_num < nprocs; process_num++)
                {
                    if(process_num != my_rank)
                    {
                        MPI_Isend(&send_buckets[process_num][0],num_items_generated_for_processes[process_num],MPI_INT,process_num,1,MPI_COMM_WORLD,&dummy_req); // Non-blocking???
                    }
                }              

                
               

        // ------------------------------- Recieve the data from others (including self) ----------------------------------------
            
            int *recv_bucket; // A single bucket for recieving all items
            int recv_bucket_size = 0; // Size of the receive bucket

            // Finding the size of the receive bucket:
            for(int process_num = 0; process_num < nprocs; process_num++)
            {
                recv_bucket_size += num_items_to_recv_from_others[process_num];
            }

            // Declare the recv_bucket memory:
            recv_bucket = (int*)malloc(sizeof(int)*recv_bucket_size);

            // Calculate the starting memory
            // addresses for receiving items
            // from all other processes (including self):
            int **starting_mem_addresses_for_recv_bucket; // Starting point for each process
            starting_mem_addresses_for_recv_bucket = (int**)malloc(sizeof(int*)*nprocs);

            for(int process_num = 0; process_num < nprocs; process_num++)
            {
                if(process_num == 0)
                {   
                    // Data from Process 0 at the beginnng of the bucket, then 1, 2, ...., nprocs-1:
                    starting_mem_addresses_for_recv_bucket[process_num] = &recv_bucket[0];
                }
                else
                {
                    starting_mem_addresses_for_recv_bucket[process_num] = starting_mem_addresses_for_recv_bucket[process_num-1]+num_items_to_recv_from_others[process_num-1];
                }
            }


            // First, copy your own items:
            for(int index = 0; index < num_items_generated_for_processes[my_rank]; index++)
            {
                *(starting_mem_addresses_for_recv_bucket[my_rank]+index) = send_buckets[my_rank][index];
            }

          
          

            // Receive from other processes:
            for(int process_num = 0; process_num < nprocs; process_num++)
            {
                // Not receieving from self:
                if(process_num != my_rank)
                {
                    MPI_Recv(starting_mem_addresses_for_recv_bucket[process_num],num_items_to_recv_from_others[process_num],MPI_INT,process_num,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                }
            }


        // Distribute time start:
        double distribute_time_end = MPI_Wtime();

        

        // --------------------------------------- Sorting ----------------------------------------------

        // Validation check:
            long int local_sum_before = 0;
            for(int index = 0; index < recv_bucket_size; index++)
            {
                local_sum_before += recv_bucket[index];
            }
            long int global_sum_before = 0;
            MPI_Reduce(&local_sum_before,&global_sum_before,1,MPI_LONG,MPI_SUM,0,MPI_COMM_WORLD);
            if(my_rank==0)
            {
                fprintf(stdout,"\n\t\t[Process %d] Global sum before sorting is %ld\n",my_rank,global_sum_before);
                fflush(stdout);
            }


            // -------------Using qsort(...) library function:          

            double sorting_start = MPI_Wtime();
            qsort(recv_bucket,recv_bucket_size,sizeof(int),compfn);
            double sorting_end = MPI_Wtime();
           

            // Validation Check:
            long int local_sum_after = 0;
            for(int index = 0; index < recv_bucket_size; index++)
            {
                local_sum_after += recv_bucket[index];
            }
            long int global_sum_after = 0;            
            MPI_Reduce(&local_sum_after,&global_sum_after,1,MPI_LONG,MPI_SUM,0,MPI_COMM_WORLD);
            if(my_rank==0)
            {
                fprintf(stdout,"\n\t\t[Process %d] Global sum after sorting is %ld\n",my_rank,global_sum_after);
                fflush(stdout);
            }


            // Ending timestamp for total time:
            double total_time_end = MPI_Wtime();

            // Calculating the durations:
            
            // Total:
            double total_duration = (total_time_end - total_time_start);

            // Bucketing stage:
            double distribute_duration = (distribute_time_end - distribute_time_start);            

            // Sorting:
            double sorting_duration = (sorting_end - sorting_start);

            // For using in reduction:
            double max_total,max_distribute,max_sorting;

            // Reduce total:
            MPI_Reduce(&total_duration,&max_total,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
            // Reduce bucketing:
            MPI_Reduce(&distribute_duration,&max_distribute,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
            // Reduce distribution:
            MPI_Reduce(&sorting_duration,&max_sorting,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);


            if(my_rank == 0)
            {
                fprintf(stdout,"\n\t nprocs = %d, total time (s): %lf, distribution time (s): %lf, sorting time (s): %lf \n",nprocs,max_total,max_distribute,max_sorting);
                fflush(stdout);
            }  
         
    

    //-----------------------free
    MPI_Barrier(MPI_COMM_WORLD);
    free(data);    
    free(process_lower_limit);
    free(process_upper_limit);
    
    free(num_items_generated_for_processes);
    free(num_items_to_recv_from_others);    
    
    for(int i=0; i < nprocs; i++)
    {
        free(send_buckets[i]);
    }
    free(send_buckets);

    free(temp_process_send_bucket_index_to_write);

    free(recv_bucket);
    

    
   

    MPI_Finalize();
    return 0;
}


double randomExponential(double lambda)
{
    double u = rand() / (RAND_MAX + 1.0);
    return -log(1- u) / lambda;
}

//generates data [0,1000000)
void generateData(int * data, int SIZE)
{
  for (int i=0; i<SIZE; i++)
  {
    double tmp=0; 
    
    //generate value between 0-1 using exponential distribution
    do{
    tmp=randomExponential(4.0);
    // printf("\nrnd: %f",tmp);
    }while(tmp>=1.0);
    
    data[i]=tmp*MAXVAL;
    
  }

  
}