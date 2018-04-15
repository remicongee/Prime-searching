#include "utils.h"

int main(int argc, char* argv[])
{
  int size, sizeReduce, display;

  int rank, numProcess, nameLen;
  int *N, *basicPrimes, *primes, globalCount;
  char processorName[MPI_MAX_PROCESSOR_NAME];
  // printf("start mpi\n");
  MPI_Init(&argc, &argv);
  // MPI_Barrier(MPI_COMM_WORLD);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcess);
  MPI_Get_processor_name(processorName, &nameLen);

  prepare(argv, &size, &display, rank);
  double time = - MPI_Wtime();

  basicPrimes = findBasicPrimes(size);
  // MPI_Barrier(MPI_COMM_WORLD);

  if(rank == 0) N = genBlock(size);

  if(display == 1)
  {
    primes = findPrimes(rank, numProcess, basicPrimes, size);
    gatherPrimes(primes, N, size, rank, numProcess, MPI_COMM_WORLD);
    time += MPI_Wtime();
    if(rank == 0) globalCount = printPrimes(N, size);
    free(primes);
  }
  else
  {
    int count = findPrimesNumber(rank, numProcess, basicPrimes, size);
    MPI_Reduce(&count, &globalCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    time += MPI_Wtime();
  }

  if(rank == 0)
  {
    printf ("%d primes are less than or equal to %d.\n", globalCount, size);
    printf ("Total elapsed time: %10.6f.\n", time);
  }
  if(rank == 0) free(N);
  free(basicPrimes);
  MPI_Finalize();
  return 0;
}
