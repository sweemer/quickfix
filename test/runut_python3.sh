#!/bin/sh

DIR=`pwd`

cd ../src/python3
./test.sh
RESULT=$?
cd $DIR
exit $RESULT
