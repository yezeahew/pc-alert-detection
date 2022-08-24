#include "main.h"
#include "universal_func.h"

#ifndef WSN_SENSOR_H
#define WSN_SENSOR_H

int wsn_sensor(MPI_Comm main_comm, MPI_Comm comm, int nrows, int ncols,struct sensor_node_report report,MPI_Datatype struct_type, int iterations, int threshold_val);
int matched_neighbours(int moving_average, int neighbour_values[*] );
#endif // WSN_SENSOR_H
