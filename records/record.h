#ifndef RECORD_H
#define RECORD_H

#define SIZE 20
#define SSIZE 6

typedef struct {
	long regNumber;
	char name[SIZE];
	char surname[SIZE];
	char houseStreet[SIZE];
	int houseNumber;
	char city[SIZE];
	char postcode[SSIZE];
	float salary;
} record;

typedef record *recordPtr;

recordPtr recordInit(); // Initialize empty record
						// Return pointer to record 

recordPtr recordInit2(long, char *, char *, char *, int, char *, char *, float); // Initialize record 
																				   // Return pointer to record 
																				   // Data as arguments

void recordFree(recordPtr); // Free record

void recordFPrint(recordPtr, FILE *); // Print data of record in file

int recordCmp(recordPtr, recordPtr, int); // Compare records based on column id attribute
										     // Return value < 0 if record1 is less than record2
											 // Return 0 if equal
											 // Return value > 0 if record1 is more than record2

void recordSwap(recordPtr *, recordPtr *); // Swap two records

// size_t recordGetSize(); // Return size of struct record

#endif