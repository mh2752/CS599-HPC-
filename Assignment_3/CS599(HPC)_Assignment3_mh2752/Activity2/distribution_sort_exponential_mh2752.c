// **************************************************************************************************************************************************

    /*
        CS599 (HPC): Assignment 3 Problem 2

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

        // Starting timestamp for total_time:
        double total_time_start = MPI_Wtime();

        // Calculating ranges of each process' bucket:
        int *process_bucket_lower_range = (int*) malloc(sizeof(int)*nprocs);
        int *process_bucket_upper_range = (int*) malloc(sizeof(int)*nprocs);

        int temp_lower;
        int temp_upper;
        for(int process_rank = 0; process_rank < nprocs; process_rank++)
        {
            if(process_rank == 0)
            {   
                // Process 0's bucket range is [0,(MAXVAL/nprocs))
                temp_lower = 0;
                temp_upper = temp_lower + (MAXVAL/nprocs) - 1;

                // Save these info in the array cells:
                process_bucket_lower_range[process_rank] = temp_lower;
                process_bucket_upper_range[process_rank] = temp_upper;
            }
            
            else
            {
                // This ranks lower range is previous ranks upper range + 1:
                temp_lower = temp_upper+1;               

                // Only if it is the last rank:
                if(process_rank == nprocs-1)
                {
                     temp_upper = MAXVAL - 1;
                }

                else
                {
                    temp_upper = temp_lower + (MAXVAL/nprocs) - 1;
                }
                // Save these info in the array cells:
                process_bucket_lower_range[process_rank] = temp_lower;
                process_bucket_upper_range[process_rank] = temp_upper;
            }
        }

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


        // Starting timestamp for bucketing process:
        double bucketing_time_start_part1 = MPI_Wtime();


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
                temp_lower_limit = process_bucket_lower_range[process_num];
                temp_upper_limit = process_bucket_upper_range[process_num];

                if((temp_item>=temp_lower_limit) && (temp_item<=temp_upper_limit))
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

            else
            {
                fprintf(stdout,"\n\t[EXCEPTION] In Process %d, Generated value %d does not belong to any bucket.\n",my_rank,temp_item);
                fflush(stdout);
            }
        }

        // Starting timestamp for bucketing process:
        double bucketing_time_end_part1 = MPI_Wtime();  
      

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

            // Starting timestamp for bucketing process:
            double bucketing_time_start_part2 = MPI_Wtime();

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
                        temp_lower_limit = process_bucket_lower_range[process_num];
                        temp_upper_limit = process_bucket_upper_range[process_num];

                        if((temp_item>=temp_lower_limit) && (temp_item<=temp_upper_limit))
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

                // Starting timestamp for bucketing process:
                double bucketing_time_end_part2 = MPI_Wtime();

                // Get the starting time of distributing data:
                double distributing_data_start_part1 = MPI_Wtime();
               
                // Send all the buckets (excluding self):
                for(int process_num = 0; process_num < nprocs; process_num++)
                {
                    if(process_num != my_rank)
                    {
                        MPI_Isend(&send_buckets[process_num][0],num_items_generated_for_processes[process_num],MPI_INT,process_num,1,MPI_COMM_WORLD,&dummy_req); // Non-blocking???
                    }
                }

                // Get the ending time of distributing data:
                double distributing_data_end_part1 = MPI_Wtime();

                // Copy into self bucket at the next stage:
               

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


            // Get the starting time of distributing data:
            double distributing_data_start_part2 = MPI_Wtime();

            // Receive from other processes:
            for(int process_num = 0; process_num < nprocs; process_num++)
            {
                // Not receieving from self:
                if(process_num != my_rank)
                {
                    MPI_Recv(starting_mem_addresses_for_recv_bucket[process_num],num_items_to_recv_from_others[process_num],MPI_INT,process_num,1,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
                }
            }

            // Get the ending time of distributing data:
            double distributing_data_end_part2 = MPI_Wtime();       

        
        // ---------------------------------------------- Now, perform the sorting  --------------------------------------------- 
          
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
            double bucketing_duration = (bucketing_time_end_part1 - bucketing_time_start_part1) + (bucketing_time_end_part2 - bucketing_time_start_part2);
            double distribution_duration = (distributing_data_end_part1 - distributing_data_start_part1) + (distributing_data_end_part2 - distributing_data_start_part2);
            bucketing_duration = bucketing_duration + distribution_duration; // Make up the whole bucketing stage

            // Sorting:
            double sorting_duration = (sorting_end - sorting_start);

            // For using in reduction:
            double max_total,max_bucketing,max_sorting;

            // Reduce total:
            MPI_Reduce(&total_duration,&max_total,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
            // Reduce bucketing:
            MPI_Reduce(&bucketing_duration,&max_bucketing,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);
            // Reduce distribution:
            MPI_Reduce(&sorting_duration,&max_sorting,1,MPI_DOUBLE,MPI_MAX,0,MPI_COMM_WORLD);


            if(my_rank == 0)
            {
                fprintf(stdout,"\n\t nprocs = %d, total time (s): %lf, distribution time (s): %lf, sorting time (s): %lf \n",nprocs,max_total,max_bucketing,max_sorting);
                fflush(stdout);
            }


        
        
       
         
    // --------------------------------------------------------------------------------------------------------------------------------------------

    //free
    free(data);     
    free(process_bucket_lower_range);
    free(process_bucket_upper_range);
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