#!/bin/bash

make
if [ ! -f "filesys.dat" ]; then
  ./mkfs filesys.dat 256 40
fi


./mkdir /empty
./mkdir /test
./mkdir /test/{zero,one,two,three}

echo "hello" | ./tee /hello.txt

echo "test one 1" | ./tee /test/one/1.txt

echo "test two 1" | ./tee /test/two/1.txt
echo "test two 2" | ./tee /test/two/2.txt

echo "test three 1" | ./tee /test/three/1.txt
echo "test three 2" | ./tee /test/three/2.txt
echo "test three 3" | ./tee /test/three/3.txt

echo "loooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooooong" | ./tee /long.txt
