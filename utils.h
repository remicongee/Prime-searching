#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "mpi.h"
// #define SIZE 200
// First element controlled by process i
#define BLOCK_LOW(i, p, n) \
        ((i) * (n) / (p) + 1)
// Last element controlled by process i
#define BLOCK_HIGH(i, p, n) \
        (BLOCK_LOW((i) + 1, p, n) - 1)
// Size of data controlled by process i
#define BLOCK_SIZE(i, p, n) \
        (BLOCK_LOW((i) + 1, p, n) - BLOCK_LOW(i, p, n))
// Owner of element j
#define BLOCK_OWNER(j, p, n) \
        (((p) * ((j) + 1) - 1) / (n))

int* genArray(int _size)
{
  int* _N = (int*)malloc(sizeof(int) * (_size + 1));
  _N[0] = _N[1] = 1;
  int _i;
  for(_i = 0;_i <= _size;_i++)
  {
    _N[_i] = 0;
  }
  return _N;
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

int* findBasicPrimes(int _size)
{
  int _count = 1, _up = sqrt(_size), _k = 2, _first, _i;
  int* _N = genArray(_up);
  int* _primes = (int*)malloc(sizeof(int) * _up);
  _primes[0] = 2;
  // printf("%d\n", _up);
  do {
    _first = _k * _k;
    for(_i = _first;_i <= _up;_i += _k)
    {
      _N[_i] = 1;
    }
    while (_k < _up && _N[++_k]);

    if(_k <= _up && !_N[_k])
    {

      if(_count > _up)
      {
        _primes = (int*)realloc(_primes, sizeof(int) * (_count + 1));
      }
      _primes[_count] = _k;
      _count++;
      // printf("%d %d\n", _k, _count);
    }
  } while(_k < _up);
  _primes[_count] = 0;
  return _primes;
}

int* findPrimes(int _rank, int _numProcess, int _basicPrimes[], int _size)
{
  int _blockSize = BLOCK_SIZE(_rank, _numProcess, _size), \
      _lowValue = BLOCK_LOW(_rank, _numProcess, _size), \
      _highValue = BLOCK_HIGH(_rank, _numProcess, _size);
  int _testRank = 1;
  int* _N = genBlock(_blockSize);
  int _j, _first, \
      _prime = _basicPrimes[0], _count = 0, _i = 1;
  if(_rank == 0) _N[0] = 1;
  while(_prime != 0)
  {
    if(_prime * _prime > _highValue) break;
    if(_prime * _prime > _lowValue)
      _first = _prime * _prime - _lowValue;
    else
    {
      if(!(_lowValue % _prime))
        _first = 0;
      else
        _first = _prime - (_lowValue % _prime);
    }
    // if(_rank == _testRank) printf("Prime %d %d\n", _prime, _lowValue);
    for(_j = _first;_j < _blockSize;_j += _prime)
    {
      _N[_j] = 1;
      // if(_rank == _testRank) printf("%d ", _j + _lowValue);
    }
    // if(_rank == _testRank) printf("\n");
    _prime = _basicPrimes[_i++];
  }
  return _N;
}

void gatherPrimes(int* _primes, int* _N, int _size, int _rank, int _numProcess, MPI_Comm _comm)
{
  int* _recvCount = (int*)malloc(sizeof(int) * _numProcess);
  int* _displace = (int*)malloc(sizeof(int) * _numProcess);
  int _i;
  for(_i = 0;_i < _numProcess;_i++)
  {
    _recvCount[_i] = BLOCK_SIZE(_i, _numProcess, _size);
    _displace[_i] = BLOCK_LOW(_i, _numProcess, _size);
  }
  MPI_Datatype _primeType;
  MPI_Type_vector(1, BLOCK_SIZE(_rank, _numProcess, _size), 0, MPI_INT, &_primeType);
  MPI_Type_commit(&_primeType);
  MPI_Gatherv(_primes, 1, _primeType, _N, _recvCount, _displace, MPI_INT, 0, _comm);
}

int findPrimesNumber(int _rank, int _numProcess, int _basicPrimes[], int _size)
{
  int _blockSize = BLOCK_SIZE(_rank, _numProcess, _size), \
      _lowValue = BLOCK_LOW(_rank, _numProcess, _size), \
      _highValue = BLOCK_HIGH(_rank, _numProcess, _size);
  // int _testRank = 1;
  int* _N = genBlock(_blockSize);
  int _j, _first, \
      _prime = _basicPrimes[0], _count = 0, _i = 1;
  if(_rank == 0) _N[0] = 1;
  while(_prime != 0)
  {
    if(_prime * _prime > _highValue) break;
    if(_prime * _prime > _lowValue)
      _first = _prime * _prime - _lowValue;
    else
    {
      if(!(_lowValue % _prime))
        _first = 0;
      else
        _first = _prime - (_lowValue % _prime);
    }
    // if(_rank == _testRank) printf("Prime %d %d\n", _prime, _lowValue);
    for(_j = _first;_j < _blockSize;_j += _prime)
    {
      _N[_j] = 1;
      // if(_rank == _testRank) printf("%d ", _j + _lowValue);
    }
    // if(_rank == _testRank) printf("\n");
    _prime = _basicPrimes[_i++];
  }
  for(_j = 0;_j < _blockSize;_j++)
  {
    if(_N[_j] == 0) _count++;
    // if(_rank == _testRank) printf("%d ", _j + _lowValue);
  }
  free(_N);
  return _count;
}

int printPrimes(int* _N, int _size)
{
  int _i, _count = 0;
  for(_i = 1;_i <= _size;_i++)
  {
    if(_N[_i] == 0)
    {
      printf("%d ", _i);
      _count++;
    }
  }
  printf("\n");
  return _count;
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
