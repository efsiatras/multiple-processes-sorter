#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "record.h"

recordPtr recordInit() { // Initialize empty record
	recordPtr r;
	r = malloc(sizeof(record)); // Malloc record struct
	if (r == NULL) { // Check malloc failure

		return NULL;
	}

	return r;
}

recordPtr recordInit2(long re, char *na, char *sur, char *hs, int hn, char *ci, char *postc, float sal) { // Initialize record ; Return pointer to record ; data as arguments
	recordPtr r;
	r = malloc(sizeof(record)); // Malloc record struct
	if (r == NULL) { // Check malloc failure

		return NULL;
	}

	r->regNumber = re;

	strcpy(r->name, na);
	strcpy(r->surname, sur);
	strcpy(r->houseStreet, hs);

	r->houseNumber = hn;

	strcpy(r->city, ci);
	strcpy(r->postcode, postc);

	r->salary = sal;

	return r;
}

void recordFree(recordPtr r) { // Free record
	free(r);

	r = NULL;
}

void recordFPrint(recordPtr r, FILE *foutput) { // Print data of record in file

	fprintf(foutput, "%ld %s %s  %s %d %s %s %f\n", r->regNumber, r->name, r->surname, r->houseStreet, r->houseNumber, r->city, r->postcode, r->salary);
}

int recordCmp(recordPtr r1, recordPtr r2, int columnid) { // Compare key of records and return 1 if they are the same
	switch (columnid) {
		case 1:

			return r1->regNumber - r2->regNumber;

		case 2:

			return strcmp(r1->name, r2->name);

		case 3:

			return strcmp(r1->surname, r2->surname);

		case 4:

			return strcmp(r1->houseStreet, r2->houseStreet);

		case 5:

			return r1->houseNumber - r2->houseNumber;

		case 6:

			return strcmp(r1->city, r2->city);

		case 7:

			return strcmp(r1->postcode, r2->postcode);

		case 8:

			return r1->salary - r2->salary;

		default:

			return 0;
	}
}

void recordSwap(recordPtr *a, recordPtr *b) { // Swap two records
	recordPtr temp = *a;
	*a = *b;
	*b = temp;
}

// size_t recordGetSize() { // Return size of struct record

// 	return sizeof(struct record);
// }