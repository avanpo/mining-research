#include "global.h"

struct overlap_matrix {
	int n;
	struct itemset *sets;
	int *overlap_sets;
};

struct overlap_matrix *generate_overlap_matrix(char *file, int len)
{
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		fprintf(stderr, "Can't open input file.\n");
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
	int n = i;

	fclose(fp);

	printf("Finished reading file.\nGenerating overlap matrix...\n");

	int *overlap = (int *)malloc(i * i * sizeof (int));

	int j;
	for (i = 0; i < n; ++i) {
		overlap[i*n + i] = (sets+i)->size;
		for (j = i + 1; j < n; ++j) {
			int similar = cmpr_sets(sets+i, sets+j);
			overlap[i*n + j] = similar;
			overlap[j*n + i] = similar;
		}
	}

	printf("Finished generating overlap matrix.\n");
	
	struct overlap_matrix *rslt = malloc(sizeof *rslt);

	rslt->n = n;
	rslt->sets = sets;
	rslt->overlap_sets = overlap;

	return rslt;
}

void write_overlap_matrix(struct overlap_matrix *m, int x, int y)
{
	printf("    |");
	int i, j;
	for (i = 0; i < 30; ++i)
		printf("%3d", (m->sets + x + i)->size);
	printf("\n----+");
	for (i = 0; i < 3 * 30; ++i)
		putchar('-');

	for (j = 0; j < 30; ++j) {
		printf("\n%3d |", (m->sets + y + j)->size);
		for (i = 0; i < 30; ++i)
			printf("%3d", *(m->overlap_sets + (x + i) * m->n + (y + j)));
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	int len;
	sscanf(argv[2], "%d", &len);

	struct overlap_matrix *m = generate_overlap_matrix(argv[1], len);

	int x, y;

	printf("Enter an x and y offset between %d and %d inclusive, or any other key to exit.\n", 0, m->n - 31);
	while (scanf("%d %d", &x, &y) == 2)
		write_overlap_matrix(m, x, y);

	return 0;
}
