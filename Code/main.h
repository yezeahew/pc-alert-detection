#include <stdio.h>
#include <math.h>
#include <mpi.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>



#ifndef MAIN_H
#define MAIN_H

#define THRESHOLD 6000 // if moving average > THRESHOLD, possible event
#define TOLERANCE 250 // allow 250 meters tolerance 
#define OUTPUT_FILE "log.txt" // output text file
#define MIN_HEIGHT 5600
#define MAX_HEIGHT 7000
#define NEIGHBOUR_COUNT 4
#define SENSOR_INTERVAL 1000    // sensor node interval to send / receive request

// Tags to distinguish messahe type 
#define REQUEST_TAG 0
#define SEND_READING_TAG 3
#define SEND_TO_ROOT_TAG 2

// cartesian topology
#define SHIFT_ROW 0
#define SHIFT_COL 1
#define DISP 1
#define MOVING_AVERAGE_WINDOW 10

#define NUM_PARAM 14 // number of param in sensor_node_report


struct sensor_node_report { // struct for node reports sent to base station
    int reading;
    float time_taken;
    char timestamp[26];
    char ip_address[15]; // store ip address of node
    char mac_address[18]; // store mac address of node
    int message_count; // num messages compared
    int adj_nodes[4]; // adjacent nodes rank
    int adj_reading[4]; // adjacent nodes reading
    int node_rank; // reporting node rank and coord
    int node_coord[2];
    int L_node_coord[2]; // neighbour node coord 
    int R_node_coord[2];
    int U_node_coord[2];
    int D_node_coord[2];
};

#endif // MAIN_H




