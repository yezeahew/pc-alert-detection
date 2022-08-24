#include "main.h"
#include "fifo_queue.h"
#include "base_satellite.h"


// prototype functions
void *satellite_simulator(void *);
void logOutput(int alert_type, FILE *output_file, struct sensor_node_report report );

// log the number of alerts and its type
int true_alerts = 0, false_alerts = 0, total_alerts_count = 0;
int row,col,size;
int termination = 0;

// global queue to store satellite generated readings
FIFOQueue* satellite_readings; // uses the fifo approach

int base_station(MPI_Comm master_comm, int nrows, int ncols,struct sensor_node_report report,MPI_Datatype struct_type,int iterations){    

    int comm_size, my_rank, i;
    MPI_Comm_size(master_comm, &comm_size);
    MPI_Comm_rank(master_comm,&my_rank);

    // create output file
    FILE *output_file; 
    output_file = fopen(OUTPUT_FILE, "w");
    
    pthread_t tid;
    
    // update global row and column
    row = nrows;
    col = ncols;
    size = row * col;

    // initialise the global array
    satellite_readings = createFIFO(CAPACITY); // FIFO queue capacity = 10

    // create thread
    int create = pthread_create(&tid, NULL, satellite_simulator, NULL);
  
    // fail safe
    // exit if cant spawn thread 
    if (create) {
        printf("ERROR; return code from pthread_create() is %d\n", create);
        exit(-1);
    }
    //sleep(2);
    
    printf("Base station will run %d iterations\n",iterations);

    //begin iterations
    for (int iter = 0; iter < iterations ; iter ++ ){    

        MPI_Request alert_request;
        MPI_Status alert_status;

        // get current time
        float start_time = MPI_Wtime();
        int alert_received;

        while( (MPI_Wtime() - start_time)*1000 < INTERVAL){
            
            // unable to receive all reports from sensor nodes 
            // limited to 1 report each time, dont know why
            sleep(WAIT_TIME);
            MPI_Irecv(&report, 1, struct_type, MPI_ANY_SOURCE, SEND_TO_ROOT_TAG, master_comm, &alert_request); // receive alert from sensor nodes
            MPI_Test(&alert_request, &alert_received, &alert_status);
            if (alert_received) {
                total_alerts_count +=1;
                printf("\n--------------------------Base station received a report--------------------------\n");
                int reading_difference = abs(rear(satellite_readings).reading - report.reading); // compare with the latest satellite reading
                // if the absolute difference between satellite reading and sensor node report reading 
                // is between 0 - tolerance value, mark as true alert
                if ( reading_difference > 0 && reading_difference < TOLERANCE ){
                    //log true alerts
                    true_alerts++;
                    logOutput(1,output_file,report);
                }
                else { // false alert
                    false_alerts++;
                    logOutput(0,output_file,report);
                    }
                break;
            }  
        }
    }
     //log summary
    fprintf(output_file, "--------------------------SUMMARY----------------------------\n");
    fprintf(output_file, "Total number of alerts: %d\n", total_alerts_count);
    fprintf(output_file, "True alerts: %d\n", true_alerts);
    fprintf(output_file, "Correct rate: %f\n", (float)true_alerts/total_alerts_count);

    termination = 1; // terminate satellite simulator
    fclose(output_file); // close file 
    pthread_join(tid, NULL); // join thread 
    free(satellite_readings);

    return total_alerts_count;
}

// posix thread function to simulate satellite altimeter
void *satellite_simulator(void *unused){
    srand(time(NULL));
    while (!termination){
        // get current time 
        time_t ts = time(NULL);
        char * tms = ctime(&ts);
        tms[strlen(tms)-1] = '\0';
        
        // generate random readings and coordinates value
        int random_reading = (rand() % (MAX_HEIGHT - MIN_HEIGHT + 1)) + MIN_HEIGHT;
        int rand_x = rand() % ( row + 1 );
        int rand_y = rand() % ( col + 1 );

        // create a new satellite_altimeter object to store the generated readings
        printf("\nSatellite generated new reading.\n");
        satellite_altimeter simulated_reading;
        simulated_reading.timestamp = ts;
        simulated_reading.reading = random_reading;
        simulated_reading.coord[0] = rand_x;
        simulated_reading.coord[1] = rand_y;

        if (!(isFull(satellite_readings))){
            // if queue is not full
            printf("Adding satellite readings to queue : reading-%d, timestamp-%s,coord-(%d,%d), array_size-%d \n", simulated_reading.reading, tms, simulated_reading.coord[0],simulated_reading.coord[1],qSize(satellite_readings));
            enqueue(satellite_readings, simulated_reading);
        }
        else{
            // if queue is full, remove one item and push new item
            satellite_altimeter removed= dequeue(satellite_readings); // remove first item
            char * rtms = ctime(&removed.timestamp);
            printf("Removed satellite readings from queue to make way: reading-%d, timestamp-%s,coord-(%d,%d)\n", removed.reading, rtms, removed.coord[0],removed.coord[1]);

            enqueue(satellite_readings, simulated_reading);
            printf("Adding satellite readings to queue : reading-%d, timestamp-%s,coord-(%d,%d), array_size-%d \n", simulated_reading.reading, tms, simulated_reading.coord[0],simulated_reading.coord[1],qSize(satellite_readings));
        }
        sleep(UPDATE_SA); // generate new readings at UPDATE_SA second interval
    }
    printf("Satellite simulator end.\n");
    return;
} 

// function to log output
void logOutput(int alert_type, FILE *output_file, struct sensor_node_report report ) {
    
    struct tm *tm;
    char* tms; 
    
    time_t t = time(NULL);
    tm = localtime(&t);
    tms = asctime(tm);
    fprintf(output_file, "------------------------------------------------------\n");
    fprintf(output_file, "Sensor Alert reporting Timestamp: %s\n", report.timestamp);
    fprintf(output_file, "Logging Timestamp: %s\n", tms);

    // if alert_type = 1 , true alert
    if (alert_type){
        fprintf(output_file, "ALERT TYPE: %s", "TRUE\n");
    }else{
        fprintf(output_file, "ALERT TYPE: %s", "FALSE\n");
    }

    fprintf(output_file, "Reporting Node 	   Coord		Height		IPv4\n");
    //fprintf(output_file, "%d                (%d,%d)             %d		%s\n\n", report.node_rank,report.node_coord[0],report.node_coord[1], report.reading,report.ip_address);
    fprintf(output_file, "%d                (x,y)             %d		%s\n\n", report.node_rank,report.reading,report.ip_address);
    fprintf(output_file, "Adjacent Nodes	   Coord		Height		IPv4		\n");

    // intend to include coordinates but the node_coord values are wrong
    for (int j = 0; j < NEIGHBOUR_COUNT; j++) {
        // if reading is valid
        if (report.adj_reading[j] >= 0) {
            int neigh_coord[2];
            if(j==0){
                neigh_coord[0] = report.L_node_coord[0]; 
                neigh_coord[1] = report.L_node_coord[1]; 
            }
            else if(j==1){
                neigh_coord[0] = report.R_node_coord[0];
                neigh_coord[1] = report.R_node_coord[1];
            }
            else if(j==2){
                neigh_coord[0] = report.U_node_coord[0];
                neigh_coord[1] = report.U_node_coord[1];
            }
            else if(j==3){
                neigh_coord[0] = report.D_node_coord[0];
                neigh_coord[1] = report.D_node_coord[1];
            }
            fprintf(output_file, "%d                (x,y)             %d		%s\n", report.adj_nodes[j],report.adj_reading[j],report.ip_address);
            //fprintf(output_file, "%d                (%d,%d)             %d		%s\n", report.adj_nodes[j],neigh_coord[0],neigh_coord[1],report.adj_reading[j],report.ip_address); 
            //printf("report neighbour coords, %d,%d\n",neigh_coord[0],neigh_coord[1]);
            }
        }

    satellite_altimeter sa_reading = rear(satellite_readings);
    fprintf(output_file, "Satellite Altimeter reading: %d\n", sa_reading.reading);
    
    time_t sa_time = sa_reading.timestamp;
    tm= localtime(&sa_time);
    tms = asctime(tm);
    fprintf(output_file, "Satellite Altimeter report time : %s", tms);
    fprintf(output_file, "Satellite Altimeter coord : (%d,%d)\n", sa_reading.coord[0],sa_reading.coord[1]);
   
    fprintf(output_file, "Number of matching neighbours: %d\n", report.message_count);
    fprintf(output_file, "Total Messages send between reporting node and base station: 1\n");
    fprintf(output_file, "Total Communication time taken(s): %f\n", report.time_taken-WAIT_TIME); // minus wait time; process sleeps for WAIT_TIME seconds every iteration // 
    fprintf(output_file, "Max. tolerance range between nodes readings (m): %d \n",TOLERANCE);
}






