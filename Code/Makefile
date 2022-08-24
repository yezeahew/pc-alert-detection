ALL: main

main: main.c universal_func.c wsn_sensor.c base_satellite.c fifo_queue.c
	mpicc main.c universal_func.c wsn_sensor.c base_satellite.c fifo_queue.c -o main -lm -pthread
	
debug:main.c universal_func.c wsn_sensor.c base_satellite.c fifo_queue.c
	mpicc -Wall main.c universal_func.c wsn_sensor.c base_satellite.c fifo_queue.c -o main -lm -pthread

run:	
	mpirun -oversubscribe -np 11 main

monarch: monarch_main.c universal_func.c wsn_sensor.c base_satellite.c fifo_queue.c
	mpicc monarch_main.c universal_func.c wsn_sensor.c base_satellite.c fifo_queue.c -o monarch_main -lm -pthread


simulate:	
	mpirun -oversubscribe -np 43 main
	
run_monarch:	
	mpirun -oversubscribe -np 43 monarch_main 6 7 100 6100
	
clean :
	/bin/rm -f main   
