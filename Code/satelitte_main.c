#include "main.h"
#include "base_satellite.h"
#include "wsn_sensor.h"

// note to myself, broadcast iteration

// main function
// this function does not work as intended 
// runs into an dead lock
int main (int argc , char * argv []) {
    int curr_iter, iteration; // number of iterations for the system
    int comm_size, world_rank; //main comm group rank
    int m_val, n_val; // cartesian size,m x n 
    MPI_Comm sub_comm;
    fd_set rfds; 
    struct timeval tv; 
    int end_program;

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

            //if not valid 
            if( (m_val*n_val) != comm_size-1){
                printf("ERROR: m_val*n_val must equal to the number of processes %d -1 \n", comm_size);
                printf("The data you entered is m_val*n_val = %d * %d = %d is not equal to %d\n", m_val, n_val, m_val*n_val,comm_size-1);
                printf("Please try again\n");
            }else{
                valid_input = 1;
            }
        }

        printf("Enter the number of iterations for the system(before termination): ");
        fflush(stdout);
        scanf("%d", &iteration);

    }
    MPI_Bcast(&m_val, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    MPI_Bcast(&n_val, 1, MPI_INT, 0, MPI_COMM_WORLD); 
    MPI_Bcast(&iteration, 1, MPI_INT, 0, MPI_COMM_WORLD); 

    // initialise struct to store sensor node reports
    struct sensor_node_report report;
    MPI_Datatype report_type;
	MPI_Datatype type[9] = { MPI_INT, MPI_FLOAT, MPI_CHAR, MPI_INT, MPI_INT, MPI_INT, MPI_CHAR, MPI_CHAR ,MPI_INT};
	int blocklen[9] = {1,1,26,1,4,4,15,18,1};
	MPI_Aint disp[9];

	MPI_Get_address(&report.reading, &disp[0]); 
	MPI_Get_address(&report.time_taken, &disp[1]); 
    MPI_Get_address(&report.timestamp, &disp[2]);
    MPI_Get_address(&report.message_count, &disp[3]);
    MPI_Get_address(&report.adj_nodes, &disp[4]);
    MPI_Get_address(&report.adj_reading, &disp[5]);
    MPI_Get_address(&report.ip_address, &disp[6]);
    MPI_Get_address(&report.mac_address, &disp[7]);
    MPI_Get_address(&report.node_rank, &disp[8]);

    for(int i = 1; i < 9; i++) {
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
        wsn_sensor(MPI_COMM_WORLD, sub_comm, m_val, n_val,report,report_type,iteration);
    }
    
    // else{
    //     //For base station
    //     base_station(MPI_COMM_WORLD, m_val,n_val,report, report_type, iteration);
    // }
    
    /* Program clean ups & end */
    MPI_Barrier(MPI_COMM_WORLD);
    if (world_rank == 0) {
        printf("Program end. \n");
        fflush(stdout); 
    }
    MPI_Type_free(&report_type);
    MPI_Finalize();
    return 0;
}