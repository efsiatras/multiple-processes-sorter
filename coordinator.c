#include <sys/types.h>
#include <sys/times.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./records/record.h"
#include "./defines.h"

int main(int argc, char *argv[]) {
	double t1, t2, dt;
	struct tms tb1, tb2;

	double ticspersec = (double) sysconf(_SC_CLK_TCK);

	t1 = (double) times(&tb1); // Start timer

	char *input = NULL;
	FILE *finput = NULL;

	int columnid0 = -1;
	int columnid1 = -1;
	int columnid2 = -1;
	int columnid3 = -1;

	char typeOfSort0 = '\0';
	char typeOfSort1 = '\0';
	char typeOfSort2 = '\0';
	char typeOfSort3 = '\0';

	if (argc < 3) { // Τοο few arguments
		printf("Too few arguments.\nUsage: './mysort -f inputfile -h|q columnid [-h|q columnid]'\n");

		return 1;
	}

	else if (argc > 11) { // Too many arguments
		printf("Too many arguments.\nUsage: './mysort -f inputfile -h|q columnid [-h|q columnid]'\n");

		return 1;
	}

	else {
		for (int i = 1; i < argc; i += 2) { // Check all flags and parameters
			if (argv[i][0] == '-') { // If argument is flag
				switch (argv[i][1]) { // Check which option it is
					case 'f':
						if (input != NULL) {
							printf("Duplicate flag: '%s'\nUsage: './mysort -f inputfile -h|q columnid [-h|q columnid]'\n", argv[i]);
							
							return 1;
						}

						else {
							input = argv[i+1];
							finput = fopen(input, "r");
							if (finput == NULL) {
								printf("Invalid input file: File does not exist.\n");

								return 2;
							}
						}

						break;

					case 'h': 
					case 'q':
						if (columnid0 == -1) {
							int num = atoi(argv[i+1]);

							if (num <= 0) {
								printf("Invalid parameter: columnid must be greater than or equal to 1.\n");

								return 1;
							}

							columnid0 = num;
							typeOfSort0 = argv[i][1];
						}

						else if (columnid1 == -1) {
							int num = atoi(argv[i+1]);

							if (num <= 0) {
								printf("Invalid parameter: columnid must be greater than or equal to 1.\n");

								return 1;
							}

							if (num == columnid0) { // Ignore same columnid

								break;
							}

							columnid1 = num;
							typeOfSort1 = argv[i][1];
						}

						else if (columnid2 == -1) {
							int num = atoi(argv[i+1]);

							if (num <= 0) {
								printf("Invalid parameter: columnid must be greater than or equal to 1.\n");

								return 1;
							}

							if ((num == columnid0) || (num == columnid1)) { // Ignore same columnid

								break;
							}

							columnid2 = num;
							typeOfSort2 = argv[i][1];
						}

						else if (columnid3 == -1) {
							int num = atoi(argv[i+1]);

							if (num <= 0) {
								printf("Invalid parameter: columnid must be greater than or equal to 1.\n");

								return 1;
							}

							if ((num == columnid0) || (num == columnid1) || (num == columnid2)) { // Ignore same columnid

								break;
							}

							columnid3 = num;
							typeOfSort3 = argv[i][1];
						}

						else {
							printf("Too many columnid flags (up to 4).\nUsage: './mysort -f inputfile -h|q columnid [-h|q columnid]'\n");

							return 1;
						}

						break;

					default:
						printf("Invalid flag: '%s'\nUsage: './mysort -f inputfile -h|q columnid [-h|q columnid]'\n", argv[i]);

						return 1;

				}
			}

			else {
				printf("Invalid argument: '%s'\nUsage: './mysort -f inputfile -h|q columnid [-h|q columnid]'\n", argv[i]);

				return 1;
			}
		}

		if ((input == NULL) || (finput == NULL)) { // Check if there was flag f which is required
			printf("Missing flag: 'f' (input file is required)\nUsage: './mysort -f inputfile -h|q columnid [-h|q columnid]'\n");

			return 1;
		}
	}

	int numOfRecords; // Number of records in input file
	long lSize;

	fseek(finput, 0, SEEK_END);
	lSize = ftell(finput);
	rewind(finput);
	numOfRecords = (int) lSize/sizeof(record);

	char numRecsBuff[SMALLBUFF]; // Buffer to convert int 'numOfRecords' to array
	sprintf(numRecsBuff, "%d", numOfRecords);

	if (fclose(finput) == EOF) { // Check fclose failure
		fprintf(stderr, "Error closing input file.\n");

		return 3;
	}

	finput = NULL;

	if (columnid1 == -1) { // Coach0 -------------------------------------------------------------
		if (columnid0 == -1) { // No h,q flags
			columnid0 = 1; // 1st column as default parameter
			typeOfSort0 = 'h'; // heapsort as default parameter
		}

		int fdcoach0[2];
		if (pipe(fdcoach0) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid0 = fork();
		if (sPid0 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid0 == 0) { // Child process
			close(fdcoach0[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach0[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff0[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff0, "%d", columnid0);

			char typeBuff0[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff0[0] = typeOfSort0;
			typeBuff0[1] = '\0';

			execlp("./coach0", "coach0", input, numRecsBuff, typeBuff0, colBuff0, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach0[WRITE]); // Parent can only read in pipe with sorter

		double tsortermin0; // Min time of sorters of coach0
		double tsortermax0; // Max time of sorters of coach0
		double tsorteravg0; // Average time of sorters of coach0
		int sigCounter0; // Counter of SIGUSR2 signals received from coach0

		double tcoachmin; // Min time of coaches
		double tcoachmax; // Max time of coaches
		double tcoachavg; // Average time of coaches

		double tcoach0;

		if (read(fdcoach0[READ], &tsortermin0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tsortermax0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tsorteravg0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &sigCounter0, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tcoach0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		tcoachmin = tcoach0;
		tcoachmax = tcoach0;
		tcoachavg = tcoach0;

		printf("Coach0: Min execution time of sorters: %lf\n", tsortermin0);
		printf("Coach0: Max execution time of sorters: %lf\n", tsortermax0);
		printf("Coach0: Avg execution time of sorters: %lf\n", tsorteravg0);
		printf("Coach0: Number of SIGUSR2 signals received: %d\n", sigCounter0);

		printf("Coordinator: Min execution time of coaches: %lf\n", tcoachmin);
		printf("Coordinator: Max execution time of coaches: %lf\n", tcoachmax);
		printf("Coordinator: Avg execution time of coaches: %lf\n", tcoachavg);
	}

	else if (columnid2 == -1) { // Coach0 and Coach1 -----------------------------------------------------
		int fdcoach0[2];
		if (pipe(fdcoach0) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid0 = fork();
		if (sPid0 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid0 == 0) { // Child process
			close(fdcoach0[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach0[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff0[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff0, "%d", columnid0);

			char typeBuff0[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff0[0] = typeOfSort0;
			typeBuff0[1] = '\0';

			execlp("./coach0", "coach0", input, numRecsBuff, typeBuff0, colBuff0, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach0[WRITE]); // Parent can only read in pipe with sorter

		int fdcoach1[2];
		if (pipe(fdcoach1) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid1 = fork();
		if (sPid1 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid1 == 0) { // Child process
			close(fdcoach1[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach1[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff1[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff1, "%d", columnid1);

			char typeBuff1[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff1[0] = typeOfSort1;
			typeBuff1[1] = '\0';

			execlp("./coach1", "coach1", input, numRecsBuff, typeBuff1, colBuff1, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach1[WRITE]); // Parent can only read in pipe with sorter

		double tcoachmin; // Min time of coaches
		double tcoachmax; // Max time of coaches
		double tcoachavg; // Average time of coaches

		double tsortermin0; // Min time of sorters of coach0
		double tsortermax0; // Max time of sorters of coach0
		double tsorteravg0; // Average time of sorters of coach0
		int sigCounter0; // Counter of SIGUSR2 signals received from coach0

		double tcoach0;

		if (read(fdcoach0[READ], &tsortermin0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tsortermax0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tsorteravg0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &sigCounter0, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tcoach0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		tcoachmin = tcoach0;
		tcoachmax = tcoach0;

		printf("Coach0: Min execution time of sorters: %lf\n", tsortermin0);
		printf("Coach0: Max execution time of sorters: %lf\n", tsortermax0);
		printf("Coach0: Avg execution time of sorters: %lf\n", tsorteravg0);
		printf("Coach0: Number of SIGUSR2 signals received: %d\n", sigCounter0);

		double tsortermin1; // Min time of sorters of coach0
		double tsortermax1; // Max time of sorters of coach0
		double tsorteravg1; // Average time of sorters of coach0
		int sigCounter1; // Counter of SIGUSR2 signals received from coach0

		double tcoach1;

		if (read(fdcoach1[READ], &tsortermin1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &tsortermax1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &tsorteravg1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &sigCounter1, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &tcoach1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (tcoach1 > tcoachmax) { // If we found new min time of sorters

			tcoachmax = tcoach1;
		}

		if (tcoach1 < tcoachmin) { // If we found new max time of sorters

			tcoachmin = tcoach1;
		}

		printf("Coach1: Min execution time of sorters: %lf\n", tsortermin1);
		printf("Coach1: Max execution time of sorters: %lf\n", tsortermax1);
		printf("Coach1: Avg execution time of sorters: %lf\n", tsorteravg1);
		printf("Coach1: Number of SIGUSR2 signals received: %d\n", sigCounter1);

		tcoachavg = (tcoach0 + tcoach1) / 2; // Calculate average time of sorters

		printf("Coordinator: Min execution time of coaches: %lf\n", tcoachmin);
		printf("Coordinator: Max execution time of coaches: %lf\n", tcoachmax);
		printf("Coordinator: Avg execution time of coaches: %lf\n", tcoachavg);
	}

	else if (columnid3 == -1) { // Coach0 and Coach1 and Coach2 ------------------------------------
		int fdcoach0[2];
		if (pipe(fdcoach0) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid0 = fork();
		if (sPid0 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid0 == 0) { // Child process
			close(fdcoach0[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach0[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff0[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff0, "%d", columnid0);

			char typeBuff0[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff0[0] = typeOfSort0;
			typeBuff0[1] = '\0';

			execlp("./coach0", "coach0", input, numRecsBuff, typeBuff0, colBuff0, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach0[WRITE]); // Parent can only read in pipe with sorter

		int fdcoach1[2];
		if (pipe(fdcoach1) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid1 = fork();
		if (sPid1 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid1 == 0) { // Child process
			close(fdcoach1[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach1[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff1[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff1, "%d", columnid1);

			char typeBuff1[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff1[0] = typeOfSort1;
			typeBuff1[1] = '\0';

			execlp("./coach1", "coach1", input, numRecsBuff, typeBuff1, colBuff1, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach1[WRITE]); // Parent can only read in pipe with sorter

		int fdcoach2[2];
		if (pipe(fdcoach2) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid2 = fork();
		if (sPid2 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid2 == 0) { // Child process
			close(fdcoach2[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach2[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff2[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff2, "%d", columnid2);

			char typeBuff2[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff2[0] = typeOfSort2;
			typeBuff2[1] = '\0';

			execlp("./coach2", "coach2", input, numRecsBuff, typeBuff2, colBuff2, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach2[WRITE]); // Parent can only read in pipe with sorter

		double tcoachmin; // Min time of coaches
		double tcoachmax; // Max time of coaches
		double tcoachavg; // Average time of coaches

		double tsortermin0; // Min time of sorters of coach0
		double tsortermax0; // Max time of sorters of coach0
		double tsorteravg0; // Average time of sorters of coach0
		int sigCounter0; // Counter of SIGUSR2 signals received from coach0

		double tcoach0;

		if (read(fdcoach0[READ], &tsortermin0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tsortermax0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tsorteravg0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &sigCounter0, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tcoach0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		tcoachmin = tcoach0;
		tcoachmax = tcoach0;

		printf("Coach0: Min execution time of sorters: %lf\n", tsortermin0);
		printf("Coach0: Max execution time of sorters: %lf\n", tsortermax0);
		printf("Coach0: Avg execution time of sorters: %lf\n", tsorteravg0);
		printf("Coach0: Number of SIGUSR2 signals received: %d\n", sigCounter0);

		double tsortermin1; // Min time of sorters of coach0
		double tsortermax1; // Max time of sorters of coach0
		double tsorteravg1; // Average time of sorters of coach0
		int sigCounter1; // Counter of SIGUSR2 signals received from coach0

		double tcoach1;

		if (read(fdcoach1[READ], &tsortermin1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &tsortermax1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &tsorteravg1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &sigCounter1, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &tcoach1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (tcoach1 > tcoachmax) { // If we found new min time of sorters

			tcoachmax = tcoach1;
		}

		if (tcoach1 < tcoachmin) { // If we found new max time of sorters

			tcoachmin = tcoach1;
		}

		printf("Coach1: Min execution time of sorters: %lf\n", tsortermin1);
		printf("Coach1: Max execution time of sorters: %lf\n", tsortermax1);
		printf("Coach1: Avg execution time of sorters: %lf\n", tsorteravg1);
		printf("Coach1: Number of SIGUSR2 signals received: %d\n", sigCounter1);

		double tsortermin2; // Min time of sorters of coach0
		double tsortermax2; // Max time of sorters of coach0
		double tsorteravg2; // Average time of sorters of coach0
		int sigCounter2; // Counter of SIGUSR2 signals received from coach0

		double tcoach2;

		if (read(fdcoach2[READ], &tsortermin2, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach2[READ], &tsortermax2, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach2[READ], &tsorteravg2, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach2[READ], &sigCounter2, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach2[READ], &tcoach2, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (tcoach2 > tcoachmax) { // If we found new min time of sorters

			tcoachmax = tcoach2;
		}

		if (tcoach2 < tcoachmin) { // If we found new max time of sorters

			tcoachmin = tcoach2;
		}

		printf("Coach2: Min execution time of sorters: %lf\n", tsortermin2);
		printf("Coach2: Max execution time of sorters: %lf\n", tsortermax2);
		printf("Coach2: Avg execution time of sorters: %lf\n", tsorteravg2);
		printf("Coach2: Number of SIGUSR2 signals received: %d\n", sigCounter2);

		tcoachavg = (tcoach0 + tcoach1 + tcoach2) / 3; // Calculate average time of sorters

		printf("Coordinator: Min execution time of coaches: %lf\n", tcoachmin);
		printf("Coordinator: Max execution time of coaches: %lf\n", tcoachmax);
		printf("Coordinator: Avg execution time of coaches: %lf\n", tcoachavg);
	}

	else { // Coach0 and Coach1 and Coach2 and Coach3 ----------------------------------------------
		int fdcoach0[2];
		if (pipe(fdcoach0) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid0 = fork();
		if (sPid0 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid0 == 0) { // Child process
			close(fdcoach0[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach0[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff0[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff0, "%d", columnid0);

			char typeBuff0[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff0[0] = typeOfSort0;
			typeBuff0[1] = '\0';

			execlp("./coach0", "coach0", input, numRecsBuff, typeBuff0, colBuff0, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach0[WRITE]); // Parent can only read in pipe with sorter

		int fdcoach1[2];
		if (pipe(fdcoach1) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid1 = fork();
		if (sPid1 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid1 == 0) { // Child process
			close(fdcoach1[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach1[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff1[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff1, "%d", columnid1);

			char typeBuff1[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff1[0] = typeOfSort1;
			typeBuff1[1] = '\0';

			execlp("./coach1", "coach1", input, numRecsBuff, typeBuff1, colBuff1, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach1[WRITE]); // Parent can only read in pipe with sorter

		int fdcoach2[2];
		if (pipe(fdcoach2) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid2 = fork();
		if (sPid2 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid2 == 0) { // Child process
			close(fdcoach2[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach2[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff2[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff2, "%d", columnid2);

			char typeBuff2[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff2[0] = typeOfSort2;
			typeBuff2[1] = '\0';

			execlp("./coach2", "coach2", input, numRecsBuff, typeBuff2, colBuff2, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach2[WRITE]); // Parent can only read in pipe with sorter

		int fdcoach3[2];
		if (pipe(fdcoach3) == -1) { // Check pipe failure
			perror("Coordinator-coach pipe");

			return 6;
		}

		pid_t sPid3 = fork();
		if (sPid3 < 0) { // Check fork failure
			perror("Fork");

			return 5;
		}

		else if (sPid3 == 0) { // Child process
			close(fdcoach3[READ]); // Sorter can only write in pipe with coach
			dup2(fdcoach3[WRITE], STDOUT_FILENO); // Set pipe as stdout for sorter

			char colBuff3[SMALLBUFF]; // Buffer to convert int 'columnid' to array
			sprintf(colBuff3, "%d", columnid3);

			char typeBuff3[2]; // Buffer to convert char 'typeOfSort' to array
			typeBuff3[0] = typeOfSort3;
			typeBuff3[1] = '\0';

			execlp("./coach3", "coach3", input, numRecsBuff, typeBuff3, colBuff3, (char*) NULL);
			
			perror("Exec*"); // This command runs only if execlp fails

			return 7;
		}

		// Parent process
		close(fdcoach3[WRITE]); // Parent can only read in pipe with sorter

		double tcoachmin; // Min time of coaches
		double tcoachmax; // Max time of coaches
		double tcoachavg; // Average time of coaches

		double tsortermin0; // Min time of sorters of coach0
		double tsortermax0; // Max time of sorters of coach0
		double tsorteravg0; // Average time of sorters of coach0
		int sigCounter0; // Counter of SIGUSR2 signals received from coach0

		double tcoach0;

		if (read(fdcoach0[READ], &tsortermin0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tsortermax0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tsorteravg0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &sigCounter0, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach0[READ], &tcoach0, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		tcoachmin = tcoach0;
		tcoachmax = tcoach0;

		printf("Coach0: Min execution time of sorters: %lf\n", tsortermin0);
		printf("Coach0: Max execution time of sorters: %lf\n", tsortermax0);
		printf("Coach0: Avg execution time of sorters: %lf\n", tsorteravg0);
		printf("Coach0: Number of SIGUSR2 signals received: %d\n", sigCounter0);

		double tsortermin1; // Min time of sorters of coach0
		double tsortermax1; // Max time of sorters of coach0
		double tsorteravg1; // Average time of sorters of coach0
		int sigCounter1; // Counter of SIGUSR2 signals received from coach0

		double tcoach1;

		if (read(fdcoach1[READ], &tsortermin1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &tsortermax1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &tsorteravg1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &sigCounter1, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach1[READ], &tcoach1, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (tcoach1 > tcoachmax) { // If we found new min time of sorters

			tcoachmax = tcoach1;
		}

		if (tcoach1 < tcoachmin) { // If we found new max time of sorters

			tcoachmin = tcoach1;
		}

		printf("Coach1: Min execution time of sorters: %lf\n", tsortermin1);
		printf("Coach1: Max execution time of sorters: %lf\n", tsortermax1);
		printf("Coach1: Avg execution time of sorters: %lf\n", tsorteravg1);
		printf("Coach1: Number of SIGUSR2 signals received: %d\n", sigCounter1);

		double tsortermin2; // Min time of sorters of coach0
		double tsortermax2; // Max time of sorters of coach0
		double tsorteravg2; // Average time of sorters of coach0
		int sigCounter2; // Counter of SIGUSR2 signals received from coach0

		double tcoach2;

		if (read(fdcoach2[READ], &tsortermin2, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach2[READ], &tsortermax2, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach2[READ], &tsorteravg2, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach2[READ], &sigCounter2, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach2[READ], &tcoach2, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (tcoach2 > tcoachmax) { // If we found new min time of sorters

			tcoachmax = tcoach2;
		}

		if (tcoach2 < tcoachmin) { // If we found new max time of sorters

			tcoachmin = tcoach2;
		}

		printf("Coach2: Min execution time of sorters: %lf\n", tsortermin2);
		printf("Coach2: Max execution time of sorters: %lf\n", tsortermax2);
		printf("Coach2: Avg execution time of sorters: %lf\n", tsorteravg2);
		printf("Coach2: Number of SIGUSR2 signals received: %d\n", sigCounter2);

		double tsortermin3; // Min time of sorters of coach0
		double tsortermax3; // Max time of sorters of coach0
		double tsorteravg3; // Average time of sorters of coach0
		int sigCounter3; // Counter of SIGUSR2 signals received from coach0

		double tcoach3;

		if (read(fdcoach3[READ], &tsortermin3, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach3[READ], &tsortermax3, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach3[READ], &tsorteravg3, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach3[READ], &sigCounter3, sizeof(int)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (read(fdcoach3[READ], &tcoach3, sizeof(double)) == -1) { // Check write failure

			perror("Read from coordinator-coach pipe");
		}

		if (tcoach3 > tcoachmax) { // If we found new min time of sorters

			tcoachmax = tcoach3;
		}

		if (tcoach3 < tcoachmin) { // If we found new max time of sorters

			tcoachmin = tcoach3;
		}

		printf("Coach3: Min execution time of sorters: %lf\n", tsortermin3);
		printf("Coach3: Max execution time of sorters: %lf\n", tsortermax3);
		printf("Coach3: Avg execution time of sorters: %lf\n", tsorteravg3);
		printf("Coach3: Number of SIGUSR2 signals received: %d\n", sigCounter3);

		tcoachavg = (tcoach0 + tcoach1 + tcoach2 + tcoach3) / 4; // Calculate average time of sorters

		printf("Coordinator: Min execution time of coaches: %lf\n", tcoachmin);
		printf("Coordinator: Max execution time of coaches: %lf\n", tcoachmax);
		printf("Coordinator: Avg execution time of coaches: %lf\n", tcoachavg);
	}

	t2 = (double) times(&tb2); // Stop timer

	dt = (t2 - t1) / ticspersec;
	printf("Coordinator: Turnaround time for completion: %lf\n", dt);
	// if (write(STDOUT_FILENO, &dt, sizeof(double)) == -1) { // Check write failure

	// 	perror("Write to coordinator-coach pipe");
	// }

	return 0;
}