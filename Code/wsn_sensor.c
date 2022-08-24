#include "main.h"
#include "wsn_sensor.h"

// This represents the sensor nodes 
// It does the following tasks :
// 1. Generate random reading
// 2. Calculate Moving Average ( Only keep the last MOVING_AVERAGE_WINDOW(10) values )
// 3. Check if moving average exceeds threshold value ( User set at runtime )
//          a. Yes - Send request to all neighbours
//                   Receive Value from all neighbours
//                   If number of neighbours matched > 2, send report to base station
//                   
int wsn_sensor(MPI_Comm main_comm, MPI_Comm comm, int nrows, int ncols,struct sensor_node_report report,MPI_Datatype struct_type,int iterations,int threshold_val){
    
    // stores sensor readings 
    int simulated_readings[MOVING_AVERAGE_WINDOW]; // use this array to calculate moving_average
    
    // initialised array of smiulated readings ( by default 5700 )
     for (int i = 0; i < MOVING_AVERAGE_WINDOW ; i++){
            simulated_readings[i] =5700 ;
        }

    int curr_reading_val ; // current randomly generated reading
    int curr_reading_i = 0; // index (cyclic index to update array)
    int moving_average = 0; // current moving average

    // fucntion to update moving average
    void update_moving_average(int new_reading){
        int sum = 0;
        simulated_readings[curr_reading_i] = new_reading;
        curr_reading_i = (curr_reading_i + 1) % MOVING_AVERAGE_WINDOW;
        
        // compute moving average
        for (int i = 0; i < MOVING_AVERAGE_WINDOW ; i++){
            sum += simulated_readings[i];
        }
        moving_average = sum / MOVING_AVERAGE_WINDOW;
    }    

    //neighbours
    int neighbours[4]; // 4 adjacent neighbours : 0-left, 1-right, 2-up, 3-down
    int neighbour_values[4] = {-1,-1,-1,-1}; // stores neighbours' values
    int message_count = 0; // number of message between neighbours

    // time
    float start_time, time_taken;
    time_t t; 
    struct tm *tm; 
    char* tms; 

    // Cartesian Topology
    int ndims = 2, size, sub_rank, reorder, my_cart_rank, ierr;
    MPI_Comm comm2D;
	int dims[ndims],coord[ndims];
	int wrap_around[ndims];

  	MPI_Comm_size(comm, &size);					// Size of sensor node communicator. //
	MPI_Comm_rank(comm, &sub_rank);				// Rank of sensor node communicator. //
	
	dims[0] = nrows;							// Number of rows. //
	dims[1] = ncols;							// Number of columns. //

    // create cartesian topology
    MPI_Dims_create(size, ndims, dims);

	// create cartesian mapping */
	wrap_around[0] = 0;
	wrap_around[1] = 0;  //periodic shift = false
	reorder = 1;
	ierr = 0;
	ierr = MPI_Cart_create(comm, ndims, dims, wrap_around, reorder, &comm2D);

    // fail safe
	if(ierr != 0) {
        printf("ERROR[%d] creating CART\n",ierr);
        }

    
    // find my coordinates in the 2D cartesian communicator group
	MPI_Cart_coords(comm2D, sub_rank, ndims, coord); // coordinates is saved into the coord array
    
    // use my 2D cartesian coordinates to find my rank in cartesian group*/
    MPI_Cart_rank(comm2D, coord, &my_cart_rank);

	MPI_Cart_shift( comm2D, SHIFT_ROW, DISP, &neighbours[2], &neighbours[3] ); // neighbours of a node
	MPI_Cart_shift( comm2D, SHIFT_COL, DISP, &neighbours[0], &neighbours[1] );

    //seed random value
	srand(time(NULL) + my_cart_rank);

    //Initialize request
	MPI_Request send_request[NEIGHBOUR_COUNT];
	MPI_Request receive_request[NEIGHBOUR_COUNT];
    int request_received; // to check request from neighbours

    // Run for a fixed number of iterations 
    // User input 
    for (int iter = 0; iter < iterations ; iter ++ ){ 
        // periodically simulate sensor readings
        curr_reading_val = (random() % (MAX_HEIGHT - MIN_HEIGHT + 1)) + MIN_HEIGHT;
        
        // add simulated reading to moving average
        update_moving_average(curr_reading_val);

        // get start time
        start_time = MPI_Wtime();

        // if moving average exceeds predefined threshold, theres a possible event
        if (moving_average > threshold_val){

            // send and request to neighbours
            for (int i = 0; i < NEIGHBOUR_COUNT; i++) {
                // if neighbour exists
                if (neighbours[i] != MPI_PROC_NULL) { 
                    MPI_Isend(&moving_average, 1, MPI_INT, neighbours[i], REQUEST_TAG, comm2D, &send_request[i]);
                    MPI_Irecv (&neighbour_values[i] , 1 , MPI_INT,  neighbours[i] , SEND_READING_TAG ,comm2D , &receive_request[i] ) ; /// send acquire reading requests to neighbour
                    message_count ++;
                }
            }
        }

        MPI_Request reading_request;
        MPI_Status status_recv;
        int isReceived = 0;

        // receive request from neighbours
        MPI_Irecv ( &request_received , 1 , MPI_INT,  MPI_ANY_SOURCE , REQUEST_TAG ,comm2D , &reading_request ) ;
        
        while ( (MPI_Wtime() - start_time)*1000 < SENSOR_INTERVAL){
            
            MPI_Test(&reading_request, &isReceived, &status_recv);
            if (isReceived){ 

                MPI_Request send_req;
                if (status_recv.MPI_SOURCE == neighbours[0] ||status_recv.MPI_SOURCE == neighbours[1]  || status_recv.MPI_SOURCE == neighbours[2] ||status_recv.MPI_SOURCE == neighbours[3] ){
                    MPI_Isend(&moving_average, 1, MPI_INT, status_recv.MPI_SOURCE,SEND_READING_TAG, comm2D, &send_req ); // send moving_average reading to requested neighbour
                }
                MPI_Irecv (&request_received ,1, MPI_INT,  MPI_ANY_SOURCE , REQUEST_TAG ,comm2D , &reading_request ) ;
            }   
        }
        MPI_Cancel(&reading_request);
        

        // if moving average exceeds predefined threshold, theres a possible event
        if (moving_average > threshold_val){

            if (matched_neighbours(moving_average,neighbour_values) >= 2) {
                int neigh_coord[ndims];
                
                // record current time when event is detected
                t = time(NULL);
                tm = localtime(&t);
                tms = asctime(tm);
         
                report.node_rank = my_cart_rank;
                time_taken = MPI_Wtime() - start_time; // calculate total time taken by a sensor node
                report.reading = moving_average; // sensor node current moving average
                report.time_taken = time_taken; 
                strcpy(report.timestamp, tms); // event timestamp
                report.message_count = message_count; // number of messages exchanged
                
                memcpy(report.ip_address, get_ip_address(), 15); // get ip address of node
                
                report.node_coord[0] = coord[0];
                report.node_coord[1] = coord[1];

                for (int j = 0; j < NEIGHBOUR_COUNT; j++) {
                    report.adj_nodes[j] = neighbours[j]; // neighbour rank (left, right, up, down)
                    report.adj_reading[j] = neighbour_values[j]; // neighbour readings
                    
                    if (neighbours[j] >= 0 ){ // check if the neighbour reading is valid ( not -2 / -1 )
                        MPI_Cart_coords(comm2D,neighbours[j], ndims, neigh_coord);
                        // store the coordinates of neighbour nodes
                        if(j==0){
                            report.L_node_coord[0] =  neigh_coord[0];
                            report.L_node_coord[1] =  neigh_coord[1];
                        }
                        else if(j==1){
                            report.R_node_coord[0] =  neigh_coord[0];
                            report.R_node_coord[1] =  neigh_coord[1];
                        }
                        else if(j==2){
                            report.U_node_coord[0] =  neigh_coord[0];
                            report.U_node_coord[1] =  neigh_coord[1];
                        }
                        else if(3==2){
                            report.D_node_coord[0] =  neigh_coord[0];
                            report.D_node_coord[1] =  neigh_coord[1];
                        }
                        
                        //report.adj_node_coord[0] =  neigh_coord[0];
                        //report.adj_node_coord[1] =  neigh_coord[1];
                        //printf("report neighbour coords, %d,%d\n",report.adj_node_coord[0],report.adj_node_coord[1]);
                    }
                }
                
                //printf("\nNode rank %d (%d,%d) trigger event. Moving Average (>Threshold %d) : %d and number of neighbours matched > 2:\n", my_cart_rank,report.node_coord[0],report.node_coord[1], threshold_val,moving_average);  
                //printf("REPORT nodes L: %d R: %d U: %d D: %d \n",report.adj_nodes[0],report.adj_nodes[1],report.adj_nodes[2],report.adj_nodes[3]);
                //printf("REPORT Readings L: %d R: %d U: %d D: %d\n",report.adj_reading[0],report.adj_reading[1],report.adj_reading[2],report.adj_reading[3]);

                // Trigger alert
                // Send a report to base station
                MPI_Request req;
                MPI_Status sta;
                MPI_Isend(&report, 1, struct_type, 0, SEND_TO_ROOT_TAG, main_comm, &req);
                MPI_Wait(&req, &sta);
                message_count = 0;   
                }
        }

    }
    //printf("Terminating node : %d.\n" , sub_rank);
    //clean ups & return 
    MPI_Comm_free(&comm2D);
    return 0;

}

int matched_neighbours(int moving_average, int neighbour_values[]){
    // get the number of matched neighbours
    // if the difference between neighbour values and current moving average is between 0 - tolerance , 
    // increment count by 1
        int count = 0; 
            for (int i = 0; i< NEIGHBOUR_COUNT; i++){
                if ( abs(neighbour_values[i]- moving_average) > 0 && abs(neighbour_values[i]- moving_average) < TOLERANCE ){
                    count +=1;
                }
            }
        return count;
    }


