#include <mpi.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MIN(a,b) ((a)<(b)?(a):(b))

int BLOCK_LOW(int id,int p,int n){
	return id*n/p;
}

int BLOCK_HIGH(int id,int p,int n){
	return (id+1)*n/p;
}
int BLOCK_SIZE(int id,int p,int n){
	return BLOCK_LOW(id+1,p,n)-BLOCK_LOW(id,p,n);
}

void prepare(char* _argv[], int* _size, int* _display, int _rank)
{
  *_size = atoi(_argv[1]);
  if((*_size) <= 1)
  {
    if(_rank == 0) printf("Please enter an integer larger than 1.\n");
    exit(1);
  }
  *_display = atoi(_argv[2]);
  if(*_display == 1)
  {
    if(_rank == 0) printf("Primes will be dispalyed.\n");
  }
  else
  {
    if(*_display == 0)
    {
      if(_rank == 0) printf("Primes will be conuted.\n");
    }
    else
    {
      if(_rank == 0) printf("Please enter 0 or 1 for display option.\n");
      exit(1);
    }
  }
  if(_rank == 0) printf("Start searching primes less than or equal to %d.\n", *_size);
}

int* genBlock(int _size)
{
  int* _block = (int*)malloc(sizeof(int) * _size);
  int _i;
  for(_i = 0;_i < _size;_i++)
  {
    _block[_i] = 0;
  }
  return _block;
}

int printPrimes(int* _N, int _size)
{
  int _i, _count = 0;
  for(_i = 1;_i < _size;_i++)
  {
    if(_N[_i] == 0)
    {
      printf("%d ", _i + 1);
      _count++;
    }
  }
  printf("\n");
  return _count;
}

void gatherPrimes(int* _primes, int* _N, int _size, int _rank, int _numProcess, MPI_Comm _comm)
{
  int _recvCount[_numProcess];
  int _displace[_numProcess];
  int _i, _sendCount = BLOCK_SIZE(_rank, _numProcess, _size - 1);
  for(_i = 0;_i < _numProcess;_i++)
  {
    _recvCount[_i] = BLOCK_SIZE(_i, _numProcess, _size - 1);
    _displace[_i] = BLOCK_LOW(_i, _numProcess, _size - 1) + 1;
    // if(_rank == 0) printf("(%d %d)\n", _recvCount[_i], _displace[_i]);
  }
  MPI_Gatherv(_primes, _sendCount, MPI_INT, _N, _recvCount, _displace, MPI_INT, 0, _comm);
}

int main (int argc, char *argv[])
{
	int id, p, display;
	int n, low_value, high_value, size;
	int *N;
	int* marked;
	double elapsed_time;
	MPI_Init (&argc, &argv);
	MPI_Barrier(MPI_COMM_WORLD);
	elapsed_time = -MPI_Wtime();
	MPI_Comm_rank (MPI_COMM_WORLD, &id);
	MPI_Comm_size (MPI_COMM_WORLD, &p);
	prepare(argv, &n, &display, id);
	low_value = 2 + BLOCK_LOW(id,p,n-1);
	high_value = 2 + BLOCK_HIGH(id,p,n-1);
	size = BLOCK_SIZE(id,p,n-1);
	// proc0_size = (n-1)/p;
  marked = (int *) malloc (sizeof(int) * size);
  if (marked == NULL) {
			printf ("Cannot allocate enough memory\n");
			MPI_Finalize();
			exit (1);
		}

		int index, prime, first;
		int i=0;
		for (i = 0; i < size; i++) marked[i] = 0; if (!id) index = 0;
		prime = 2;
		do {
			if (prime * prime > low_value)
				first = prime * prime - low_value;
		  else {
				if (!(low_value % prime)) first = 0; else first = prime - (low_value % prime);
			}
			for (i = first; i < size; i += prime) marked[i] = 1; if (!id) {
      	while (marked[++index]);
      	prime = index + 2;
   		}
			MPI_Bcast (&prime, 1, MPI_INT, 0, MPI_COMM_WORLD);
		} while (prime * prime <= n);

		int count = 0;
		if(id == 0) N = genBlock(n);
		int global_count;
		if(display == 0)
		{
			for (i = 0; i < size; i++)
					if (!marked[i]) count++;
			MPI_Reduce (&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
			elapsed_time += MPI_Wtime();
	 	}
	 	if(display == 1)
	 	{
		 	gatherPrimes(marked, N, n, id, p, MPI_COMM_WORLD);
		 	elapsed_time += MPI_Wtime();
		 	if(id == 0) global_count = printPrimes(N, n);
     	free(marked);
	 	}
	 	if (!id) {
		 	printf ("%d primes are less than or equal to %d\n", global_count, n);
			printf ("Total elapsed time: %10.6f\n", elapsed_time);
		}
		MPI_Finalize ();
		return 0;
}
