###############################################################
#
# CSCI 5103
# Lab 4 - Linux Device Drivers
# 
# MEMBERS
#   Steven Storla - storl060 - storl060
#   Jon Huhn - huhnx025 - 4967467
#   Paul Rheinberger - rhein055 - 4716178
#
###############################################################


Compiling and installing the scullBuffer device:
--------------------------------------------------
1. Execute Make: 
   make
2. If scull devices already exist then unload them first.
   sudo ./unload.py
   This may ask you for your root password.
3. Load scull devices
   sudo ./load.py scull_size=x
   This will create one scull buffer device: /dev/scullBuffer0


Compiling and running producer and consumer tests:
--------------------------------------------------
1. Execute Make: 
   make

   This step can be skipped if Step 1 from 'Compiling and installing the
     scullBuffer device' was executed

2. Run the producer program
   ./producer [num_writes]

   The producer program will issue num_writes, default 1, writes to the device.
     The program waits for the user to press enter before each write is
     executed.

3. Run the consumer program
   ./consumer [num_reads]

   The consumer program will issue num_reads, default 1, reads from the device.
     The program waits for the user to press enter before each read is
     executed.
   

Running / Testing:
$ echo "hello world" | tee /dev/scullBuffer0
hello world
$ cat /dev/scullBuffer0
hello world
$ echo "foo 1 2 3" | tee /dev/scullBuffer0
foo 1 2 3
$ cat /dev/scullBuffer0
foo 1 2 3

