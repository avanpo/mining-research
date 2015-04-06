#include "global.h"

/* Gets maximal frequent itemsets from a file containing all closed frequent
 * itemsets, and writes these to a file.
 * ***************************************************************************/

void get_max_from_closed_data(char *file, int len)
{
	FILE *fp = fopen(file, "r");
	FILE *fp_out = fopen("out.dat", "w");
	if (fp == NULL || fp_out == NULL) {
		fprintf(stderr, "Can't open input and/or output file.\n");
		exit(EXIT_FAILURE);
	}

	int max = 1024;
	struct itemset *sets = malloc(max * sizeof *sets);

	int i;
	char line[255] = {0};
	for (i = 0; fgets(line, 255, fp) != NULL; ++i) {
		if (i == max) {
			max *= 2;
			sets = realloc(sets, max * sizeof *sets);
		}
		parse_set(sets+i, line, len);
	}

	fclose(fp);

	int j, k;
	for (j = 0; j < i; ++j) {
		int flag = 1;
		for (k = j+1; k < i; ++k) {
			if (cmpr_sets(sets+j, sets+k) == (sets+j)->size) {
				flag = 0;
				break;
			}
		}
		if (flag) {
			write_set(sets+j, fp_out);
		}
	}

	fclose(fp_out);
}

int main(int argc, char *argv[])
{
	srand(time(NULL));

	int len;
	sscanf(argv[2], "%d", &len);

	get_max_from_closed_data(argv[1], len);

	return 0;
}
