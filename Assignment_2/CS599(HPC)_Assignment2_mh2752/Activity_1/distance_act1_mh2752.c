//**********************************************************************************************************************************
/*

    CS-599(HPC, Spring-2022): Assignment 2 Problem 1

    Author:

        Nazmul (mh2752)

*/


//************************************************  Header Imports  ****************************************************************

#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//*************************************************  Utility Function Definitions  *************************************************

    // All utility function prototype declarations:

        // For reading and importing data from the dataset text file
        // named MSD_year_prediction_normalize_0_1_100k.txt
        int importDataset(char * fname, int N, double **dataset);
        
        // Function for printing commandline argument error:
        void displayCmdlineArgumentErrorMessage();


//**************************************************  Main Function  ****************************************************************


int main(int argc, char **argv) 
{

    int my_rank, nprocs;

    // MPI Initialization:
    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&my_rank);
    MPI_Comm_size(MPI_COMM_WORLD,&nprocs);


    //Process command-line arguments
    int N;
    int DIM;
    int blocksize;
    char inputFname[500];

    // If correct number of command line
    // arguments are not passed:
    if (argc != 5)
    {   
        // Only master rank should display error message. Other
        // ranks should just finalize and quit:
        if(my_rank == 0)
        {
            displayCmdlineArgumentErrorMessage();
        }
            
        MPI_Finalize();
        exit(0);
    }

    
    // Reading the command line argument values
    // into program variables:
    sscanf(argv[1],"%d",&N);
    sscanf(argv[2],"%d",&DIM);
    sscanf(argv[3],"%d",&blocksize);
    strcpy(inputFname,argv[4]);
    
    // Pointer to dataset
    double **dataset;

    // For starting timestamp:
    double start_time;

        
    // Quit in case of invalid number of data points or
    // data point dimension:
    if (N<1 || DIM <1)
    {
        // Only master rank should display error message. Other
        // ranks should just finalize and quit:
        if(my_rank == 0)
        {
            printf("\nN is invalid or DIM is invalid\n");
        }
       
        MPI_Finalize();
        exit(0);
    }


    // Inputs are valid. Proceed:
    else
    {  
        if (my_rank==0)
        {
            fprintf(stdout,"Supplied command line arguments: \n");
            fflush(stdout);
            fprintf(stdout,"\n\tNumber of lines (N): %d, \n\tDimensionality: %d, \n\tBlock size: %d, \n\tNum of Processes: %d, \n\tFilename: %s\n\n", N, DIM, blocksize, nprocs, inputFname);
            fflush(stdout);
        }

        // Allocate memory for dataset (all ranks):
        dataset = (double**)malloc(sizeof(double*)*N);
        for (int i=0; i<N; i++)
        {
            dataset[i] = (double*)malloc(sizeof(double)*DIM);
        }

        // Importing the dataset:
        int ret = importDataset(inputFname, N, dataset);

        // Failed to import dataset into allocated
        // memory. Free memory and quit.
        if(ret==1)
        {
            printf("[ERROR] Process %d: Failure in importing dataset. Aborting.\n");

            // Freeing dataset memories:
            for (int i=0; i<N; i++)
            {
                free(dataset[i]);
            }
            free(dataset);

            // Finalizing and quitting:
            MPI_Finalize();
            return 0;
        }

    }

    //------------------------------------------------ Write Code Here -----------------------------------------------------------------

    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Reading dataset, Scattering row limits <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

        // Utility variables:
            // Array for row limits information           
            int *row_start = NULL; 
            int *row_end = NULL;

            // For all ranks
            int *process_row_start = (int*)malloc(sizeof(int)*1);
            int *process_row_end = (int*)malloc(sizeof(int)*1);

            double sum_of_all_local_sum = 0.0;

           
            // Allocate memory against *row_limits
            // by the master rank, compute the row
            // limits for all of the ranks and place
            // those limits in 'row_limits':
            if(my_rank == 0)
            {
                // Allocate memories for (2*nprocs) number
                // of integers:
                row_start = (int*) malloc(sizeof(int)*nprocs); 
                row_end = (int*) malloc(sizeof(int)*nprocs);

                // Temporary index variables:
                int start_index,end_index;

                // Compute and assign the row limits:
                for(int process_rank = 0; process_rank < nprocs; process_rank++)
                {   
                    // If not the last rank, get total
                    // (N/nprocs) number of rows:
                    if(process_rank != (nprocs-1))
                    {                        
                        // Calculate the start and end indices:
                        start_index = process_rank * (int)(N/nprocs);
                        end_index = start_index + (int)(N/nprocs) - 1;

                        // Put these information in the row_limits
                        // array => [ @Index i: start index, @Index i+1: end index ]
                        row_start[process_rank] = start_index;
                        row_end[process_rank] = end_index;
                    }

                    // The last rank gets a total of 
                    // ((N/nrpocs)+(N%nrpcos)) number
                    // of rows:
                    else
                    {                         
                        // Calculate the start and end indices:
                        start_index = process_rank * (int)(N/nprocs);
                        end_index = start_index + (int)(N/nprocs) + (N%nprocs) - 1;

                        // Put these information in the row_limits
                        // array => [ @Index i: start index, @Index i+1: end index ]
                        row_start[process_rank] = start_index;
                        row_end[process_rank] = end_index;                       
                    }

                }// for loop ends            
            }


            // Scatter the row_limits array:
            MPI_Scatter((void*)row_start,1,MPI_INT,(void*)process_row_start,1,MPI_INT,0,MPI_COMM_WORLD);
            MPI_Scatter((void*)row_end,1,MPI_INT,(void*)process_row_end,1,MPI_INT,0,MPI_COMM_WORLD);


            // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Creating and Populating the Local Distance Matrices <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


            // Wait for all process to have the data:
            MPI_Barrier(MPI_COMM_WORLD);

            // Getting the starting timestamp:
            start_time = MPI_Wtime();


            // At this point in code, all the process ranks have their row limits 
            // infomration  and the required dataset in their respective 'dataset' array.
            //
            // Use those information to determine the size of the local distance
            // matrix and allocate the required memory:
            int matrix_row_size = (*process_row_end - *process_row_start) + 1;          

            // Allocate memories for the local
            // distance matrices:
            double **local_distance_matrix;
            local_distance_matrix = (double**)malloc(sizeof(double*)*matrix_row_size);
            for(int row = 0; row < matrix_row_size; row++)
            {
                local_distance_matrix[row] = (double*)malloc(sizeof(double)*N);
            }

                // Now, populate the local distance matrices:
                double *source_point;
                double *destination_point;
                double temp_distance_squared;

                for(int data_row = *process_row_start, local_index = 0; data_row <= *process_row_end && local_index < matrix_row_size; data_row++, local_index++)
                {
                    // Distance from this point:
                    source_point = dataset[data_row];

                    // Go over all the points in the dataset(inclusive):
                    for(int data_col = 0; data_col < N; data_col++)
                    {
                        // Distance to this point:
                        destination_point = dataset[data_col];

                        // Initialize distance:
                        temp_distance_squared = 0.0;

                        // Calculate the cartesian distance in dimension = DIM:
                        for(int dim = 0; dim < DIM; dim++)
                        {
                            temp_distance_squared += pow(source_point[dim] - destination_point[dim],2);
                        }

                        // Get the square-root of temp_distance_squared and save it in the local distance matrix:
                        local_distance_matrix[local_index][data_col] = sqrt(temp_distance_squared);

                    }

                }

                
                double endtime_excl_sanity_check = MPI_Wtime();

    // >>>>>>>>>>>>>>>>>>>>>>>>>> Sanity-check: calculate each process' local sum of distance and compare against global <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

                // Calculate the sum of all local sum of distances
                // using MPI_Reduce():
                double local_sum_of_dist = 0.0;
                for(int row = 0; row < matrix_row_size; row++)
                {
                    for(int col = 0; col < N; col++)
                    {
                        local_sum_of_dist += local_distance_matrix[row][col];
                    }
                }               

                // Everybody calls this:    
                MPI_Reduce(&local_sum_of_dist,&sum_of_all_local_sum,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);

                
                // Only Process 0 double checks the
                // sums of distances:
                if(my_rank == 0)
                {
                    // Print sum of all local sum of distances:
                    fprintf(stdout,"\n\tGlobal Sum = %lf\n",sum_of_all_local_sum);
                    fflush(stdout);               

                    fprintf(stdout,"\n\t Total time (seconds) to calculate and populate local distance matrices: %lf\n",endtime_excl_sanity_check - start_time);
                    fflush(stdout);

                }


    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> End of Code <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



    // ----------------------- Free all memories --------------------------------
    for (int i=0; i<N; i++)
    {
        free(dataset[i]);
    }
    free(dataset);

    for(int i=0; i<matrix_row_size; i++)
    {
        free(local_distance_matrix[i]);
    }
    free(local_distance_matrix);

    if(my_rank == 0)
    {
        free(row_start);
        free(row_end);
    }

    
    free(process_row_start);
    free(process_row_end);

    // --------------------------------------------------------------------------


    MPI_Finalize();

    return 0;
}





// ********************************************** Utility Function Definitions *****************************************

// For importing the dataset from the text
// file in the same directory as this 
// source code file:
int importDataset(char * fname, int N, double ** dataset)
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


// Display a fixed error message regarding any
// commandline argument error:
void displayCmdlineArgumentErrorMessage()
{

    fprintf(stderr,"Please provide the following on the command line: N (number of lines in the file), dimensionality (number of coordinates per point), block size, dataset filename.\n");

}


