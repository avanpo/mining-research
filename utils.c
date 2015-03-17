#include "global.h"

/* searching and comparing
 ********************************************************************/

struct cmpr_results *compare_all(struct dataset *data, int n, int d_rng, int s_rng, int s_stp)
{
	struct cmpr_results *rslt = init_cmpr_rslt(d_rng, s_rng, s_stp);

	struct itemset *set;
	int i;
	for (i = 0; i < n; ++i) {
		set = choose_rand_set(data);
		compare_against_all(rslt, set, data);
	}
	return rslt;
}

void compare_against_all(struct cmpr_results *rslt, struct itemset *set, struct dataset *data)
{
	int s_rng = rslt->supp_range;
	int s_stp = rslt->supp_step;
	int i, d, s;
	for (i = 0; i < data->num_sets; ++i) {
		d = data->size - compare_set(set, data->itemsets + i);
		if (d > rslt->dist_range || d == 0)
			continue;
		s = ((data->itemsets+i)->support - set->support) / s_stp + (s_rng / 2);
		if (s < 0)
			s = 0;
		if (s >= s_rng)
			s = s_rng - 1;
		++(rslt->counts[(s * rslt->dist_range) + d - 1]);
		++(rslt->t_counts[d - 1]);
	}
}

int compare_set(struct itemset *a, struct itemset *b)
{
	int c, i;
	for (c = 0, i = 0; i < a->data_len; ++i)
		c += count_bits(a->items[i] & b->items[i]);
	return c;
}

int count_bits(uint64_t x)
{
	x -= (x >> 1) & 0x5555555555555555ULL;
	x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
	x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0fULL;
	return (x * 0x0101010101010101ULL) >> 56;
}

int count_sparse_bits(uint64_t x)
{
	int c;
	for (c = 0; x; ++c)
		x &= x-1;
	return c;
}

/* random choose
 ********************************************************************/

struct itemset *choose_rand_set(struct dataset *data)
{
	int r = rand() % data->num_sets;
	return data->itemsets + r;
}

/* constructors
 ********************************************************************/

struct cmpr_results *init_cmpr_rslt(int d_rng, int s_rng, int s_stp)
{
	int *counts = calloc(d_rng * s_rng, sizeof *counts);
	int *t_counts = calloc(d_rng, sizeof *t_counts);

	struct cmpr_results *rslt = malloc(sizeof *rslt);
	
	rslt->dist_range = d_rng;
	rslt->supp_range = s_rng;
	rslt->supp_step  = s_stp;
	rslt->counts     = counts;
	rslt->t_counts   = t_counts;
}

/* memory management
 ********************************************************************/

void free_set(struct itemset *set)
{
	free(set->items);
}

/* reading
 ********************************************************************/
struct dataset *read_data(char *file, int size, int len)
{
	FILE *fp = fopen(file, "r");
	if (fp == NULL) {
		fprintf(stderr, "Can't open input file %s.\n", file);
		exit(EXIT_FAILURE);
	}

	int max = 1024;

	struct itemset *sets = malloc(max * sizeof *sets);

	int i = 0, total = 0;
	char line[255] = {0};
	while (fgets(line, 255, fp) != NULL) {
		if (i == max) {
			max *= 2;
			sets = realloc(sets, max * sizeof *sets);
		}
		parse_line(sets+i, line, len);

		if ((sets+i)->size == size)
			++i;
		++total;
	}

	fclose(fp);

	struct dataset *data = malloc(sizeof *data);
	
	data->total_sets = total;
	data->num_sets = i;
	data->size = size;
	data->itemsets = sets;
}

void parse_line(struct itemset *dest, char *src, int len)
{
	free(dest->items);
	int count = 0;
	uint64_t *items = calloc(len, sizeof *items);
	
	char *token = strtok(src, " ");

	int val;
	while (token && token[0] != '(') {
		sscanf(token, "%d", &val);
		items[(val-1)/64] |= 0x0000000000000001ULL << ((val-1)%64);
		count++;
		token = strtok(NULL, " ");
	}

	int support;
	sscanf(token, "(%d)", &support);

	dest->size = count;
	dest->data_len = len;
	dest->items = items;
	dest->support = support;
}

/* printing
 ********************************************************************/

void print_cmpr_rslt(struct cmpr_results *rslt)
{
	int d_rng = rslt->dist_range;
	int s_rng2 = rslt->supp_range / 2;
	int s_stp = rslt->supp_step;
	print_line(d_rng, 0);
	print_line(d_rng, 1);
	print_line(d_rng, 0);
	int i, d;
	for (i = 0; i < s_rng2; ++i) {
		printf("| %7d", (i-s_rng2)*s_stp);
		for (d = 0; d < d_rng; ++d) {
			printf(" %7d", rslt->counts[(i*d_rng) + d]);
		}
		printf(" |   | %7d", i*s_stp);
		for (d = 0; d < d_rng; ++d) {
			printf(" %7d", rslt->counts[((i+s_rng2)*d_rng) + d]);
		}
		printf(" |\n");
	}
	print_line(d_rng, 0);
	printf("|        ");
	for (i = 0; i < d; ++i)
		printf("        ");
	printf(" |   |   Total");
	for (i = 0; i < d; ++i)
		printf(" %7d", rslt->t_counts[i]);
	printf(" |\n");
	print_line(d_rng, 0);
}

void write_cmpr_rslt(struct cmpr_results *rslt)
{
	FILE *fp = fopen("plots/plot.txt", "w");
	if (fp == NULL) {
		fprintf(stderr, "Can't open output file plot.txt.\n");
		exit(EXIT_FAILURE);
	}

	int d_rng = rslt->dist_range;
	int s_rng2 = rslt->supp_range / 2;
	int s_stp = rslt->supp_step;

	int i, d;
	for (i = 0; i < rslt->supp_range; ++i) {
		fprintf(fp, "%d", (i-s_rng2)*s_stp);
		for (d = 0; d < d_rng; ++d) {
			fprintf(fp, " %d", rslt->counts[(i*d_rng) + d]);
		}
		fprintf(fp, "\n");
	}
	fclose(fp);
}

void print_line(int d, int head)
{
	head ? printf("|    Dist") : printf("+--------");
	int i;
	for (i = 0; i < d; ++i)
		head ? printf(" %7d", i+1) : printf("--------");
	head ? printf(" |   |    Dist") : printf("-+   +--------");
	for (i = 0; i < d; ++i)
		head ? printf(" %7d", i+1) : printf("--------");
	head ? printf(" |\n") : printf("-+\n");
}

void print_set(struct itemset *set)
{
	int i, j;
	uint64_t z;
	for (i = 0, j = 1; i < 2; ++i) {
		for (z = 1; z != 0; z <<= 1, ++j) {
			if (set->items[i] & z)
				printf("%d ", j);
		}
	}
	printf("(%d)\n", set->support);
}

void print_set_bits(struct itemset *set)
{
	print_uint64_bits(set->items[1]);
	print_uint64_bits(set->items[0]);
	printf(" (%d)\n", set->support);
}

void print_uint64_bits(uint64_t x)
{
	char b[65];
	b[64] = '\0';
	char *p = b;
	int i;
	uint64_t z;
	for (z = 0x8000000000000000ULL; z > 0; z >>= 1)
		*p++ = (x & z) ? '1' : '0';
	printf("%s", b);
}
