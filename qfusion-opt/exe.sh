TEST_NAME=$1
echo "TEST_NAME: $TEST_NAME"
CMD=$2
echo "CMD: $CMD"

echo "$TEST_NAME begin" >> out.txt
date +"%Y%m%d_%H%M%S" >> out.txt
$CMD >> out.txt
date +"%Y%m%d_%H%M%S" >> out.txt
echo "$TEST_NAME end" >> out.txt