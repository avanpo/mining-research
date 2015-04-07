#include "global.h"

/* Gets maximal frequent itemsets from a file containing all closed frequent
 * itemsets, and writes these to a file.
 * ***************************************************************************/

void get_max_from_closed_data(char *file, int len, char *file_out)
{
	FILE *fp = fopen(file, "r");
	FILE *fp_out = fopen(file_out, "w");
	if (fp == NULL || fp_out == NULL) {
		fprintf(stderr, "Can't open input and/or output file.\n");
		exit(EXIT_FAILURE);
	}

	printf("Reading file...\n");

	int max = 1024;
	struct itemset *sets = malloc(max * sizeof *sets);

	int i;
	char line[1024] = {0};
	for (i = 0; fgets(line, 255, fp) != NULL; ++i) {
		if (i == max) {
			max *= 2;
			sets = realloc(sets, max * sizeof *sets);
		}
		parse_set(sets+i, line, len);
	}

	fclose(fp);

	printf("Finished reading file.\n");

	int j, k, val;

	int *possible_maxset = malloc(i * sizeof (int));
	for (j = 0; j < i; ++j) {
		possible_maxset[j] = 1;
	}

	printf("Generating maximal frequent itemsets...\n");

	for (val = -1, j = 0; j < i; ++j) {
		if (val != 100 * j / i) {
			val = 100 * j / i;
			print_progress_bar(val);
		}
		if (!possible_maxset[j])
			continue;

		int flag = 1;
		for (k = 0; k < i; ++k) {
			if (!possible_maxset[k]) {
				continue;
			}
			int smlr = cmpr_sets(sets + j, sets + k);
			if (smlr == (sets+j)->size && j != k) {
				flag = 0;
				break;
			}
			if (smlr == (sets+k)->size && j != k) {
				possible_maxset[k] = 0;
			}
		}
		if (flag) {
			write_set(sets+j, fp_out);
		}
		else {
			possible_maxset[j] = 0;
		}
	}

	printf("\rFinished generating maximal frequent itemsets.                               \n");

	fclose(fp_out);
}

int main(int argc, char *argv[])
{
	int len;
	sscanf(argv[2], "%d", &len);

	get_max_from_closed_data(argv[1], len, argv[3]);

	return 0;
}
