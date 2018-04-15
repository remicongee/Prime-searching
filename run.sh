hosts="$1"
echo "Please choose version of searching primes program:"
echo "0--prime broadcast(naive) 1--replicate preprocess, 2--eliminate even numbers."
read version

if [ $version -eq 0 ]
  then
  code="./naive.o"
elif [ $version -eq 1 ]
  then
  code="./prime.o"
elif [ $version -eq 2 ]
  then
  code="./primeUp.o"
else
  echo "Version should be 0/1/2."
  exit 1
fi

if [ ! -e "$code" ]
  then
  echo "Not yet compiled."
  exit 1
fi

echo "Please enter an integer larger than 1 for range of primes: "
read size
if [ $size -le 1 ]
  then
  echo "Range should be larger than 1."
  exit 1
fi

echo "Primes will be displayed?"
echo "0--count, 1--display"
read display
if [ $display -ne 0 -a $display -ne 1 ]
  then
  echo "Display option should be 0/1."
  exit 1
fi

echo "Please set process number:"
read num
if [ $num -lt 1 ]
  then
  echo "Process number should be strictly positive."
fi

if [ -e "$hosts" ]
  then
  echo "Multiple nodes required."
  mpirun -n $num -f $hosts $code $size $display
elif [ ! "$hosts" ]
  then
  echo "Local node required."
  mpirun -n $num $code $size $display
else
  echo "Hosts file ($hosts) does not exist."
  exit 1
fi  

#echo "process-$num, code-$code, size-$size, display-$display"

