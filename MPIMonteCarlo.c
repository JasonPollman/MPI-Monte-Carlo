#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mpi.h"
#include <unistd.h>

#define PI 3.14159265358979

MPI_Status status;

int     n, s, S, rank, num_processes;
struct  timeval tic, toc, seed_time;

void slave_io () {

  char slave_name[256];
  gethostname(slave_name, 256);

  printf("Slave\tProcess #%d\t[%s]\n", rank, slave_name);

  int computations = 0;

  while(1) { // While true...

    int seed;

    // Get the seed from the master (or kill msg)
    MPI_Recv(&seed, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

    if(status.MPI_TAG == 0) { // Kill slave process if TAG == 0
      break;
    }
    else if(status.MPI_TAG == 200) { // Compute number of points in circle if TAG == 200

      // Compute number of circumscribed points
      srand(seed);
      int count = 0, i = 0;

      for(i = 0; i < n; i++) {
        int x = rand() % (RAND_MAX / 100);
        int y = rand() % (RAND_MAX / 100);
        if(pow((double) x / (RAND_MAX / 100), 2) + pow((double) y / (RAND_MAX / 100), 2) <= 1) count++;
      }

      // Send the count back to the master...
      // printf("N: %d, COUNT: %d\n", n, count);
      MPI_Send(&count, 1, MPI_INT, 0, 300, MPI_COMM_WORLD);

      computations++;

    } // End if/else block...

  } // End while loop

  // Send total computations back to the master
  MPI_Send(&computations, 1, MPI_INT, 0, 400, MPI_COMM_WORLD);

} // End slave_io()


void master_io () {

  char master_name[256];
  gethostname(master_name, 256);

  printf("Master\tProcess #%d\t[%s]\n", rank, master_name);

  int count = 0, i = 0;
  int computations[num_processes];

  gettimeofday(&tic, NULL); // Get start time

  int min = num_processes - 1;
  if(s < num_processes) min = s;

  // Send initial work to slaves...
  for(i = 1; i <= min; i++) {
    gettimeofday(&seed_time, NULL);
    srand(seed_time.tv_usec);
    int seed = rand();
    MPI_Send(&seed, 1, MPI_INT, i, 200, MPI_COMM_WORLD);
  }

  s -= min - 1;

  while(s > 0) {

    s--; // Decrement the number of remaining seeds...

    // While there are still seeds to distribute...
    // This is the "task queue"...

    // Get counts from any process
    int value;
    MPI_Recv(&value, 1, MPI_INT, MPI_ANY_SOURCE, 300, MPI_COMM_WORLD, &status);

    count += value;

    // Get a unique seed (based on usec) to send to a slave...
    gettimeofday(&seed_time, NULL);
    srand(seed_time.tv_usec);
    int seed = rand() % (RAND_MAX / 10) + 100000000;

    // Send the "ready" slave the seed...
    if(s > 0) MPI_Send(&seed, 1, MPI_INT, status.MPI_SOURCE, 200, MPI_COMM_WORLD);



  } // End master_io()

  gettimeofday(&toc, NULL); // Get end time

  // Work is done, send shudown signal to slaves
  for(i = 1; i < num_processes; i++)
    MPI_Send(&i, 1, MPI_INT, i, 0, MPI_COMM_WORLD);

  // Get final computation counts from each process...
  for(i = 1; i < num_processes; i++) {
    int value;
    MPI_Recv(&value, 1, MPI_INT, i, 400, MPI_COMM_WORLD, &status);
    computations[status.MPI_SOURCE] = value;
  }


  // Compute final PI Value and display:
  double pi = count / ((double) (S * n)) * 4;

  printf("N x S = %d\n", n * S);
  printf("PI Estimation: %f\n", pi);
  printf("Estimation Error: %f\n", fabs(PI - pi));

  // Print total elapsed time...
  double elapsed = (toc.tv_sec - tic.tv_sec) + ((toc.tv_usec - tic.tv_usec) / 1000000.0);
  printf("Time to Estimate PI: %fs\n", elapsed);

  // Print the number of seeds each process handled...
  printf("Seeds Computed: ");

  for(i = 1; i < num_processes; i ++)
    printf("[P%d] -> %d ", i, computations[i]);

  printf("\n\n");

} // End master_io()


int main (int argc, char *argv[]) {

  setbuf(stdout, NULL);

  MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &num_processes);

  // Must have atleast 2 processes for the workpool:
  if(num_processes < 2) {
    printf("You must utilize at least 2 processes for the workpool!\n");
    MPI_Finalize();
    return 0;
  }

  if(rank == 0) { // Master Process

    // Get number of points
    printf("Input the total number of points: ");
    scanf("%d", &n);

    // Get number of seeds
    printf("Input the total number of seeds: ");
    scanf("%d", &S);

    s = S;

  } // End if block...

  // Broadcast value of n to all processes...
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  // Workpool implementation...
  if(rank != 0) slave_io(); else master_io();


  MPI_Finalize();
  return 0;


} // End main()
