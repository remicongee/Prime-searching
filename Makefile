# Makefile

# Compilor
CC = mpicc

# Source
SRCS = prime.c
SRCS += primeUp.c
SRCS += naive.c

# Library
LIBS = -lm

# Grammar replacement
OBJS = $(SRCS:.c=.o)

# Target
all : $(OBJS)
		@echo "Compile completed."
		chmod a+x ./run.sh

prime.o : utils.h
		$(CC) prime.c $(LIBS) -o prime.o

primeUp.o : utilsUp.h
		$(CC) primeUp.c $(LIBS) -o primeUp.o

naive.o :
	        $(CC) naive.c $(LIBS) -o naive.o

.PHONY : clean
clean :
	@-rm $(all) $(OBJS)
