# Table of Contents

1. [Basic Documentation] (README.md#basic-documentation)
2. [Running the Program] (README.md#running-the-program)

##Basic Documentation

This entry to the coding challenge was written in C++, compiled using g++ 4.8.4 on Ubuntu 14.04
No dependencies are required outside of STL.

To run over the full 3.8 million entries from the input took about 90 seconds, while running over the
3.0 million entries to verify from stream_payment took another 15 minutes, for a processing time of
0.3ms per entry.  

While processing the data, I observed new user ids are assigned consecutively with time. Based on the maximum user 
ID, I assumed for this program that the user id number was bound 0 < n < 100000.

The user network is designed to be generally extensible for any number of users or transactions, but the verification 
program includes an array of visited nodes that is bound by the above restriction.

##Running the Program

The executable resides in the digital-wallet top directory and is called antifraud.exe.

To compile the program, run:

$ source compile.sh

antifraud.exe takes 5 arguments; 2 input files and 3 output files.  You may run antifraud.exe using the 
batch_payment and stream_payment csv files by running:

$ source run.sh


