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

	int *overlap = malloc(i * i * sizeof (int));

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
		}
		if ((sets+i)->size > set_size_max)
			set_size_max = (sets+i)->size;
	}

	printf("\rFinished generating overlap matrix.                                          \n");
	
	struct overlap_matrix *rslt = malloc(sizeof *rslt);

	rslt->n = n;
	rslt->sets = sets;
	rslt->overlap_sets = overlap;
	rslt->set_size_max = set_size_max + 1;

	return rslt;
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

struct sum_matrix *sum_overlap_matrix(struct overlap_matrix *m, int val)
{
	int *sums = calloc(m->set_size_max * m->set_size_max, sizeof (int));

	int i, j;
	for (i = 0; i < m->n; ++i) {
		if ((m->sets+i)->size != val) {
			continue;
		}
		for (j = i+1; j < m->n; ++j) {
			++sums[(m->sets+j)->size * m->set_size_max + m->overlap_sets[i * m->n + j]];
		}
	}
	
	struct sum_matrix *rslt = malloc(sizeof *rslt);

	rslt->size = val;
	rslt->sums = sums;
	rslt->set_size_max = m->set_size_max;

	return rslt;
}

void print_sum_matrix(struct sum_matrix *sm)
{
	int i, j;
	printf("           Set size vs. Overlap size (Against size %d)\n", sm->size);
	printf("---------+");
	for (j = 0; j < 6 * sm->set_size_max; ++j)
		putchar('-');
	printf("\n         | Overlap size\n");
	printf("Set size |");
	for (j = 0; j < sm->set_size_max; ++j)
		printf("%6d", j);
	printf("\n---------+");
	for (j = 0; j < 6 * sm->set_size_max; ++j)
		putchar('-');

	for (i = 0; i < sm->set_size_max; ++i) {
		printf("\n  %6d |", i);
		for (j = 0; j < sm->set_size_max; ++j)
			printf("%6d", *(sm->sums + i * sm->set_size_max + j));
	}
	printf("\n---------+");
	for (j = 0; j < 6 * sm->set_size_max; ++j)
		putchar('-');
	printf("\n");
}

int *extract_histogram_eq(struct sum_matrix *sm)
{
	int *rslt = calloc(sm->set_size_max, sizeof *rslt);
	int *hist = rslt;

	int i;
	for (i = 0; i < sm->set_size_max; ++i, ++hist) {
		*hist = *(sm->sums + sm->size * sm->set_size_max + i);
	}
	return rslt;
}

int *extract_histogram_leq(struct sum_matrix *sm)
{
	int *rslt = calloc(sm->set_size_max, sizeof *rslt);
	int *hist = rslt;

	int i, j;
	for (i = 0; i < sm->set_size_max; ++i, ++hist) {
		for (j = 0; j <= sm->size; ++j) {
			*hist += *(sm->sums + j * sm->set_size_max + i);
		}
	}
	return rslt;
}

int *extract_histograms_eq(struct sum_matrix **sm, int l, int r, int norm)
{
	int *rslt = calloc((*sm)->set_size_max * (r - l), sizeof *rslt);
	int *hist = rslt;

	int i, j;
	for (i = 0; i < (r - l); ++i, ++sm) {
		for (j = 0; j < (*sm)->set_size_max; ++j) {
			rslt[i*(*sm)->set_size_max + j] = *((*sm)->sums + (*sm)->size * (*sm)->set_size_max + j);
			if (norm) {
				rslt[i * (*sm)->set_size_max + j] /= choose(i + l, j);
			}
		}
	}
	return rslt;
}

int *extract_histograms_leq(struct sum_matrix **sm, int l, int r, int norm)
{
	int *rslt = calloc((*sm)->set_size_max * (r - l), sizeof *rslt);
	int *hist = rslt;

	int i, j, k;
	for (i = 0; i < (r - l); ++i, ++sm) {
		for (j = 0; j < (*sm)->set_size_max; ++j) {
			for (k = 0; k <= (*sm)->size; ++k) {
				rslt[i * (*sm)->set_size_max + j] += *((*sm)->sums + k * (*sm)->set_size_max + j);
			}
			if (norm) {
				rslt[i * (*sm)->set_size_max + j] /= choose(i + l, j);
			}
		}
	}
	return rslt;
}

void print_hist(int *hist)
{
	int i;
	for (i = 0; i < 19; ++i) {
		printf("%d ", *(hist + i));
	}
	printf("\n");
}

void print_histograms(int *hists, int max, int l, int r)
{
	int i, j;
	printf("    |");
	for (j = 0; j < max; ++j)
		printf("%6d", j);
	printf("\n----+");
	for (j = 0; j < max; ++j)
		printf("------");
	printf("\n");
	for (i = 0; i < (r - l); ++i) {
		printf("%3d |", i + l);
		for (j = 0; j < max; ++j) {
			printf("%6d", *(hists + i * max + j));
		}
		printf("\n");
	}
	printf("----+");
	for (j = 0; j < max; ++j)
		printf("------");
	printf("\n");
}

void write_histograms(int *hists, int max, int l, int r)
{
	FILE *fp = fopen("plots/plot.txt", "w");
	if (fp == NULL) {
		fprintf(stderr, "Can't open output file plot.txt.\n");
		exit(EXIT_FAILURE);
	}

	int i, j;
	for (i = 0; i < r; ++i) {
		fprintf(fp, "%d", i);
		for (j = 0; j < (r - l); ++j) {
			fprintf(fp, " %d", *(hists + j * max + i));
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

int main(int argc, char *argv[])
{
	int len;
	sscanf(argv[2], "%d", &len);

	struct overlap_matrix *m = generate_overlap_matrix(argv[1], len);

	// Create histogram range
	int i, l = 12, r = 19;
	struct sum_matrix **sm = malloc((r - l) * sizeof *sm);
	for (i = 0; i < (r - l); ++i) {
		sm[i] = sum_overlap_matrix(m, i + l);
	}
	int *hists = extract_histograms_eq(sm, l, r, 0);
	int *hists2 = extract_histograms_eq(sm, l, r, 1);
	print_histograms(hists, m->set_size_max, l, r);
	print_histograms(hists2, m->set_size_max, l, r);
	//write_histograms(hists, m->set_size_max, l, r);

	return 0;
}
