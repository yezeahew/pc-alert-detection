#ifndef BASE_SATELLITE_H
#define BASE_SATELLITE_H

#include <pthread.h>
#include <unistd.h>
#include "universal_func.h"

#define WAIT_TIME 1
#define UPDATE_SA 3
#define INTERVAL 10000


int base_station(MPI_Comm master_comm, int nrows, int ncols,struct sensor_node_report report, MPI_Datatype struct_type,int iterations);

#endif // BASE_SATELLITE_H
