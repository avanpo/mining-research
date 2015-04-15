#include "global.h"

struct overlap_matrix {
	int n;
	struct itemset *sets;
	int *overlap_sets;
	int set_size_max;
};

struct sum_matrix {
	int size;
	int *sums;
	int set_size_max;
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

	int set_size_max = 0, set_size_min = 10000;

	int j, val;
	for (val = -1, i = 0; i < n; ++i) {
		if (val != 100 * i / n) {
			val = 100 * i / n;
			print_progress_bar(val);
		}
		overlap[i*n + i] = (sets+i)->size;
		for (j = i + 1; j < n; ++j) {
			int similar = cmpr_sets(sets+i, sets+j);
			overlap[i*n + j] = similar;
			overlap[j*n + i] = similar;
			if ((sets+i)->size == (sets+j)->size && (sets+i)->size == similar) {
				printf("Error found.\n");
				print_set(sets+i);
				print_set(sets+j);
			}
		}
		if ((sets+i)->size > set_size_max)
			set_size_max = (sets+i)->size;
	}

	printf("\rFinished generating overlap matrix.                                          \n");
	
	struct overlap_matrix *rslt = malloc(sizeof *rslt);

	rslt->n = n;
	rslt->sets = sets;
	rslt->overlap_sets = overlap;
	rslt->set_size_max = set_size_max;

	return rslt;
}

struct sum_matrix *sum_overlap_matrix(struct overlap_matrix *m, int val)
{
	int *sums = (int *)calloc(m->set_size_max * m->set_size_max, sizeof (int));

	int i, j;
	for (i = 0; i < m->n; ++i) {
		for (j = i+1; j < m->n; ++j) {
			if ((m->sets+i)->size == val) {
				++sums[*(m->overlap_sets + i * m->n + j) * m->set_size_max + (m->sets+j)->size];
			}
		}
	}
	
	struct sum_matrix *rslt = calloc(1, sizeof *rslt);

	rslt->size = val;
	rslt->sums = sums;
	rslt->set_size_max = m->set_size_max;

	return rslt;
}

int *extract_histogram_eq(struct sum_matrix *sm)
{
	int *rslt = (int *)calloc(sm->set_size_max, sizeof (int));
	int *hist = rslt;

	int i;
	for (i = 0; i < sm->set_size_max; ++i, ++hist) {
		*hist = *(sm->sums + sm->set_size_max * i + sm->size);
	}
	return rslt;
}

int **extract_histograms_eq(struct sum_matrix **sm, int rng)
{
	int **rslt = (int **)calloc(rng, sizeof (int *));
	int **hist = rslt;

	int i;
	for (i = 0; i < rng; ++i, ++hist, ++sm) {
		*hist = (int *)calloc((*sm)->set_size_max + 1, sizeof (int));
		**hist = (*sm)->size;

		int *vals = *hist + 1;

		int j;
		for (j = 0; j < (*sm)->size; ++j, ++vals) {
			*vals = *((*sm)->sums + j * (*sm)->set_size_max + (*sm)->size);
		}
	}
	return rslt;
}

void print_histogram(int *hist)
{
	int i;
	for (i = 0; i < 10; ++i) {
		printf("%d ", *(hist + i));
	}
	printf("\n");
}

void write_histogram(FILE *fp, int *hist, int set_size_max)
{
	int i;
	for (i = 0; i < set_size_max; ++i) {
		fprintf(fp, "%d %d\n", i, *(hist + i));
	}
}

void write_histograms(int **hists, int rng, int set_size_max)
{
	FILE *fp = fopen("plots/hist-plot.txt", "w");
	if (fp == NULL) {
		fprintf(stderr, "Can't open output file hist-plot.txt.\n");
		exit(EXIT_FAILURE);
	}

	int **hist = hists;
	int *vals;
	int i, j;
	for (i = 0; i < set_size_max; ++i) {
		fprintf(fp, "%d", i);
		vals = *hist;
		for (j = 0; j < rng; ++j) {
			fprintf(fp, " %d", *(vals+1+i));
		}
		fprintf(fp, "\n");
		++hist;
	}
	fclose(fp);
}

void print_overlap_matrix(struct overlap_matrix *m, int x, int y)
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

void print_sum_matrix(struct sum_matrix *sm)
{
	printf("       |");
	int i, j;
	for (i = 0; i < sm->set_size_max; ++i)
		printf("%6d", i);
	printf("\n-------+");
	for (i = 0; i < 6 * sm->set_size_max; ++i)
		putchar('-');

	for (j = 0; j < sm->set_size_max; ++j) {
		printf("\n%6d |", j);
		for (i = 0; i < sm->set_size_max; ++i)
			printf("%6d", *(sm->sums + i * sm->set_size_max + j));
	}
	printf("\n");
}

int main(int argc, char *argv[])
{
	int len;
	sscanf(argv[2], "%d", &len);

	struct overlap_matrix *m = generate_overlap_matrix(argv[1], len);

	/*int x, y;

	printf("Enter an x and y offset between %d and %d inclusive, or any other key to exit.\n", 0, m->n - 31);
	while (scanf("%d %d", &x, &y) == 2)
		print_overlap_matrix(m, x, y);*/

	struct sum_matrix *sums = sum_overlap_matrix(m, 10);
	print_sum_matrix(sums);/*

	int *hist = extract_histogram_eq(sums);
	print_histogram(hist);*/
	
	int i;
	for (i = 1; i < 17; ++i) {
		char fname[24];
		sprintf(fname, "plots/hist-%d-plot.txt", i);
		FILE *fp = fopen(fname, "w");
		if (fp == NULL) {
			fprintf(stderr, "Can't open output file hist-plot.txt.\n");
			exit(EXIT_FAILURE);
		}
		struct sum_matrix *sm = sum_overlap_matrix(m, i);
		int *hist = extract_histogram_eq(sm);
		write_histogram(fp, hist, sm->set_size_max);
	}

	/*struct sum_matrix **arr = calloc(16, sizeof *arr);
	struct sum_matrix **cnt = arr;
	int i;
	for (i = 1; i < 17; ++i, ++cnt) {
		*cnt = sum_overlap_matrix(m, i);
	}

	int **hists = extract_histograms_eq(arr, 1);
	write_histograms(hists, 16, m->set_size_max);*/

	return 0;
}
