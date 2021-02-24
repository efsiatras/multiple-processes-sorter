# Sort with multiple processes
A program sorting a binary file of records in various ways using multiple processes.

Received statistics about turnaround time of processes and signals are printed at the end of the program execution.

The main goal of this program implementation was to practice with **fork**, **exec**, **piping** and **sigaction**.

## Process tree
![coordinator, coaches and sorters processes](https://siatras.dev/img/process-tree-sort.jpg)

## Compilation
`$ make clean`

`$ make`

## Usage
Command Line Parameters (and their flags) can be used in any order.

Command Line Flags **h**,**q** are optional and can be used up to 4 times.
On the other hand, Command Line Flag **f** is required.

`./mysort -f <Input File> -h|q <Column ID> [-h|q <Column ID>]`

## Design Decisions
- Each **Record** is represented by a simple struct of its data.

- **Coordinator** node is represented by ***mysort*** executable. Coordinator node can fork and exec coach nodes according to the tree of processes shown above.

- **Coach** nodes are represented by ***coach0***, ***coach1***, ***coach2***  or ***coach3*** executables. Coach nodes can fork and exec sorter nodes according to the tree of processes shown above.

- **Sorter** nodes are represented by ***heapsorter*** or ***quicksorter*** executables.

## Implementation / Interface
- **Struct to define a record**: records/record.c, records/record.h

- **Header file with important definitions**: defines.h

- **Main function for Heapsort sorter**: sorters/heapsorter.c

- **Main function for Quicksort sorter**: sorters/quicksorter.c

- **Main function for coach with 2^0 = 1 sorter as child process**: coaches/coach0.c

- **Main function for coach with 2^1 = 2 sorters as child processes**: coaches/coach1.c

- **Main function for coach with 2^2 = 4 sorters as child processes**: coaches/coach2.c

- **Main function for coach with 2^3 = 8 sorters as child processes**: coaches/coach3.c

- **Main function for coordinator of 1 up to 4 coaches**: coordinator.c

## Format of Records in Output Files
The records in output files are printed in the following form:

`regNumber name surname  houseStreet houseNumber city postcode salary`