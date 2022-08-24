#include "main.h"
#include "base_satellite.h"
#include "wsn_sensor.h"


// main function
// This function requests for user input
// 1. M value 
// 2. N value 
// 3. Threshold value 
// 4. Number of iterations

// Then split the number of processors into two groups (Global Communicator)
// a. Rank 0 - Base Station
// b. Remaining - Sensor Node
int main (int argc , char * argv []) {
    float start_time = MPI_Wtime();
    int iteration; // number of iterations for the system
    int comm_size, world_rank; //main comm group rank
    int m_val, n_val, threshold_val; // cartesian size,m x n 
    MPI_Comm sub_comm;

    // initialise MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
   
    if (world_rank == 0){
        // request for user input at runtime 
         int valid_input = 0;
         while (!valid_input){
            fflush(stdout);
            printf("Please enter M value : ");
            fflush(stdout);
            scanf("%d", &m_val);
            printf("Please enter N value : ");
            fflush(stdout);
            scanf("%d", &n_val);
            printf("Please enter the threshold value( ideally between 5800-6200): ");
            fflush(stdout);
            scanf("%d", &threshold_val);

            //if not valid 
            if( (m_val*n_val) != comm_size-1){
                printf("ERROR: m_val*n_val must equal to the number of processes %d -1 \n", comm_size);
                printf("The data you entered is m_val*n_val = %d * %d = %d is not equal to %d\n", m_val, n_val, m_val*n_val,comm_size-1);
                printf("Please try again\n");
            }
            else if (threshold_val < 5000 || threshold_val > 6700){
                printf("Threhold value is too low or too high.(<5000 or >6700)\n");
            
            }else{
                valid_input = 1;
            }
        }

        printf("Enter the number of iterations for the system(before termination): ");
        fflush(stdout);
        scanf("%d", &iteration);

    }
    // broadcast values to all process
    MPI_Bcast(&m_val, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    MPI_Bcast(&n_val, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    MPI_Bcast(&iteration, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    MPI_Bcast(&threshold_val, 1, MPI_INT, 0, MPI_COMM_WORLD); 

    // initialise struct to store sensor node reports
    struct sensor_node_report report;
    MPI_Datatype report_type;
	MPI_Datatype type[NUM_PARAM] = { MPI_INT, MPI_FLOAT, MPI_CHAR, MPI_INT, MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR ,MPI_INT,MPI_INT,MPI_INT,MPI_INT,MPI_INT,MPI_INT};//,MPI_Comm};
	int blocklen[NUM_PARAM] = {1,1,26,1,4,4,15,18,1,2,2,2,2,2};
	MPI_Aint disp[NUM_PARAM];

	MPI_Get_address(&report.reading, &disp[0]); 
	MPI_Get_address(&report.time_taken, &disp[1]); 
    MPI_Get_address(&report.timestamp, &disp[2]);
    MPI_Get_address(&report.message_count, &disp[3]);
    MPI_Get_address(&report.adj_nodes, &disp[4]);
    MPI_Get_address(&report.adj_reading, &disp[5]);
    MPI_Get_address(&report.ip_address, &disp[6]);
    MPI_Get_address(&report.mac_address, &disp[7]);
    MPI_Get_address(&report.node_rank, &disp[8]);
    MPI_Get_address(&report.node_coord, &disp[9]);
    MPI_Get_address(&report.L_node_coord, &disp[10]);
    MPI_Get_address(&report.R_node_coord, &disp[11]);
    MPI_Get_address(&report.U_node_coord, &disp[12]);
    MPI_Get_address(&report.D_node_coord, &disp[13]);

    for(int i = 1; i < NUM_PARAM; i++) {
        disp[i] -= disp[0];
    }
    disp[0] = 0;

	// Create struct and MPI TYPE
	MPI_Type_create_struct(9, blocklen, disp, type, &report_type);
	MPI_Type_commit(&report_type);

	// use mpi_comm_spil to split processes into groups: 
    // 1)base station  2) WSN nodes
    MPI_Comm_split( MPI_COMM_WORLD,world_rank == 0, 0, &sub_comm);
    if (world_rank != 0){
        // For WSN nodes 
        // number of alerts sent per node
        wsn_sensor(MPI_COMM_WORLD, sub_comm, m_val, n_val,report,report_type,iteration,threshold_val);
    }
    else{
        //For base station
        base_station(MPI_COMM_WORLD, m_val,n_val,report, report_type, iteration);
    }
    
    /* Program clean ups & end */
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 0) {
        printf("Program end. \n");
        fflush(stdout); 
    }
    MPI_Type_free(&report_type);
    MPI_Finalize();
    float simulation_time = MPI_Wtime() - start_time;
    printf("Total Simulation Time (s): %f\n", simulation_time);
    return 0;
}
