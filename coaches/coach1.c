#include <sys/types.h>
#include <sys/wait.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "../records/record.h"
#include "../defines.h"

int sigCounter = 0; // Counter of SIGUSR2 signals received

void sigusr2_handler(int num) {
	sigCounter++;
}

int main(int argc, char *argv[]) {
	double t1, t2, dt;
	struct tms tb1, tb2;

	double ticspersec = (double) sysconf(_SC_CLK_TCK);

	t1 = (double) times(&tb1); // Start timer

	if (argc < 5) {
		printf("Too few arguments.\nUsage: './coach1 inputfile numOfRecords typeOfSort columnid'\n");

		return 1;
	}

	else if (argc > 5) {
		printf("Too many arguments.\nUsage: './coach1 inputfile numOfRecords typeOfSort columnid'\n");

		return 1;
	}

	long int numOfRecords = atol(argv[2]);
	if (numOfRecords < 0) {
		printf("Invalid 'numOfRecords' parameter.\n");

		return 4;
	}

	char *typeOfSort = argv[3];
	if (strcmp(typeOfSort, "q") && strcmp(typeOfSort, "h")) {
		printf("Invalid 'typeOfSort' parameter.\n");

		return 4;
	}

	int columnid = atoi(argv[4]);
	if ((columnid < 1) || (columnid > 8)) {
		fprintf(stderr, "Invalid 'columnid' parameter.\n");

		return 4;
	}

	struct sigaction sa;
	sa.sa_handler = sigusr2_handler;
	sa.sa_flags = SA_RESTART; // Important flag to avoid system call failures cause of signal

	sigaction(SIGUSR2, &sa, NULL);

	int fdsorter0[2];
	if (pipe(fdsorter0) == -1) { // Check pipe failure
		perror("Coach-sorter pipe");

		return 6;
	}

	pid_t sPid0 = fork();
	if (sPid0 < 0) { // Check fork failure
		perror("Fork");

		return 5;
	}

	else if (sPid0 == 0) { // Child process
		close(fdsorter0[READ]); // Sorter can only write in pipe with coach
		dup2(fdsorter0[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

		long high = (numOfRecords / 2) - 1; // Calculate high
		char highBuff[MEDIUMBUFF]; // Buffer to convert long int 'high' to array
		sprintf(highBuff, "%ld", high);

		char colBuff[SMALLBUFF]; // Buffer to convert int 'columnid' to array
		sprintf(colBuff, "%d", columnid);

		if (*typeOfSort == 'q') {

			execlp("./quicksorter", "quicksorter", argv[1], "0", highBuff, colBuff, (char*) NULL);
		}

		else { // *typeOfSort == 'h'

			execlp("./heapsorter", "heapsorter", argv[1], "0", highBuff, colBuff, (char*) NULL);
		}
		
		perror("Exec*"); // This command runs only if execlp fails

		return 7;
	}
	// Parent process
	close(fdsorter0[WRITE]); // Parent can only read in pipe with sorter

	int fdsorter1[2];
	if (pipe(fdsorter1) == -1) { // Check pipe failure
		perror("Coach-sorter pipe");

		return 6;
	}

	pid_t sPid1 = fork();
	if (sPid1 < 0) { // Check fork failure
		perror("Fork");

		return 5;
	}

	else if (sPid1 == 0) { // Child process
		close(fdsorter1[READ]); // Sorter can only write in pipe with coach
		dup2(fdsorter1[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

		long low = numOfRecords / 2; // Calculate low
		char lowbuff[MEDIUMBUFF]; // Buffer to convert long int 'low' to array
		sprintf(lowbuff, "%ld", low);

		long high = numOfRecords - 1; // Calculate high
		char highBuff[MEDIUMBUFF]; // Buffer to convert long int 'high' to array
		sprintf(highBuff, "%ld", high);

		char colBuff[SMALLBUFF]; // Buffer to convert int 'columnid' to array
		sprintf(colBuff, "%d", columnid);

		if (*typeOfSort == 'q') {

			execlp("./quicksorter", "quicksorter", argv[1], lowbuff, highBuff, colBuff, (char*) NULL);
		}

		else { // *typeOfSort == 'h'

			execlp("./heapsorter", "heapsorter", argv[1], lowbuff, highBuff, colBuff, (char*) NULL);
		}
		
		perror("Exec*"); // This command runs only if execlp fails

		return 7;
	}
	// Parent process
	close(fdsorter1[WRITE]); // Parent can only read in pipe with sorter

	size_t recordSize = sizeof(record); // Get size of struct record

	size_t recordPipeCap = PIPECAPACITY / recordSize; // The pipe capacity in number of records

	recordPtr recordList = malloc(numOfRecords * sizeof(record)); // Malloc array of pointers to records

	long recordsTransferred = 0; // Number of records that have been transferred

	long recordsLeft = numOfRecords / 2; // Number of records left to be transferred by each sorter
	while (recordsLeft > 0) { // While there are still records to be transferred from sorter0
		if (recordsLeft >= recordPipeCap) { // If there are records left more than (or equal) to the record pipe capacity
			if (read(fdsorter0[READ], &recordList[recordsTransferred], recordSize * recordPipeCap) == -1) { // Check write failure

				perror("Read from coach-sorter pipe");
			}

			recordsTransferred += recordPipeCap; // Update number of records that have been transferred
			recordsLeft -= recordPipeCap; // Update number of records left to be transferred
		}

		else { // recordsLeft < recordPipeCap
			if (read(fdsorter0[READ], &recordList[recordsTransferred], recordsLeft * recordSize) == -1) { // Check write failure

				perror("Read from coach-sorter pipe");
			}

			recordsTransferred += recordsLeft; // Update the number of records that have been transferred
			recordsLeft = 0; // Update number of records left to be transferred
		}
	}

	recordsLeft = numOfRecords - numOfRecords / 2; // Reset counter for next sorter
	while (recordsLeft > 0) { // While there are still records to be transferred from sorter1
		if (recordsLeft >= recordPipeCap) { // If there are records left more than (or equal) to the record pipe capacity
			if (read(fdsorter1[READ], &recordList[recordsTransferred], recordSize * recordPipeCap) == -1) { // Check write failure

				perror("Read from coach-sorter pipe");
			}

			recordsTransferred += recordPipeCap; // Update number of records that have been transferred
			recordsLeft -= recordPipeCap; // Update number of records left to be transferred
		}

		else { // recordsLeft < recordPipeCap
			if (read(fdsorter1[READ], &recordList[recordsTransferred], recordsLeft * recordSize) == -1) { // Check write failure

				perror("Read from coach-sorter pipe");
			}

			recordsTransferred += recordsLeft; // Update the number of records that have been transferred
			recordsLeft = 0; // Update number of records left to be transferred
		}
	}

	char output[MEDIUMBUFF]; // Buffer to store output file name
	FILE *foutput = NULL;

	char colBuff[SMALLBUFF]; // Buffer to convert int 'columnid' to array
	sprintf(colBuff, "%d", columnid);

	strcpy(output, "myinputfile.");
	strcat(output, colBuff);

	foutput = fopen(output, "w");
	if (foutput == NULL) {
		printf("Invalid output file: File cannot be opened.\n");

		return 2;
	}

	for (long i = 0; i < numOfRecords; i++) {  // Print and free records

		recordFPrint(&recordList[i], foutput);
	}
	free(recordList);
	recordList = NULL;

	if (fclose(foutput) == EOF) { // Check fclose failure
		fprintf(stderr, "Error closing input file.\n");

		return 3;
	}
	foutput = NULL;

	int status;
	while (wait(&status) == -1); // Wait all children to finish

	double tsortermin; // Min time of sorters
	double tsortermax; // Max time of sorters
	double tsorteravg; // Average time of sorters

	double tsorter0;
	double tsorter1;

	if (read(fdsorter0[READ], &tsorter0, sizeof(double)) == -1) { // Check read failure

		perror("Read from coordinator-coach pipe");
	}

	tsortermin = tsorter0; // Initalize min time of sorters
	tsortermax = tsorter0; // Initalize max time of sorters

	if (read(fdsorter1[READ], &tsorter1, sizeof(double)) == -1) { // Check read failure

		perror("Read from coordinator-coach pipe");
	}

	if (tsorter1 > tsortermax) { // If we found new min time of sorters

		tsortermax = tsorter1;
	}

	if (tsorter1 < tsortermin) { // If we found new max time of sorters

		tsortermin = tsorter1;
	}

	tsorteravg = (tsorter0 + tsorter1) / 2; // Calculate average time of sorters

	if (write(STDOUT_FILENO, &tsortermin, sizeof(double)) == -1) { // Check write failure

		perror("Write to coordinator-coach pipe");
	}

	if (write(STDOUT_FILENO, &tsortermax, sizeof(double)) == -1) { // Check write failure

		perror("Write to coordinator-coach pipe");
	}

	if (write(STDOUT_FILENO, &tsorteravg, sizeof(double)) == -1) { // Check write failure

		perror("Write to coordinator-coach pipe");
	}

	if (write(STDOUT_FILENO, &sigCounter, sizeof(int)) == -1) { // Check write failure

		perror("Write to coordinator-coach pipe");
	}

	t2 = (double) times(&tb2); // Stop timer

	dt = (t2 - t1) / ticspersec;
	if (write(STDOUT_FILENO, &dt, sizeof(double)) == -1) { // Check write failure

		perror("Write to coordinator-coach pipe");
	}

	return 0;
}