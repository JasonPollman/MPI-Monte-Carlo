#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mpi.h"
#include <unistd.h>

#define PI 3.14159265358979

int main () {

  struct timeval tic, toc;

  int n = 0, i = 0, count = 0;
  double x, y, pi, elapsed;

  printf("Input the total number of points: ");
  scanf("%d", &n);

  srand(time(NULL));

  gettimeofday(&tic, NULL);

  for(i = 0; i < n; i++)
    if(pow((double) rand() / RAND_MAX, 2) + pow((double) rand() / RAND_MAX, 2) <= 1) count++;

  gettimeofday(&toc, NULL);


  pi = (double) count / n * 4;
  printf("\nPI Estimate:\t\t%f", pi);
  printf("\nEstimate Error:\t\t%f", fabs(PI - pi));

  elapsed = (toc.tv_sec - tic.tv_sec) + ((toc.tv_usec - tic.tv_usec) / 1000000.0);
  printf("\nTime to Estimate PI:\t%fs\n\n", elapsed);

  return 0;

}
