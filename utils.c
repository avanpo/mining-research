#include "global.h"

/* searching and comparing
 ********************************************************************/

int *compare_against_all(struct itemset *set, struct dataset *data)
{
	int *values = calloc(60 * 3, sizeof *values);

	int i, d, s;
	for (i = 0; i < data->num_sets; ++i) {
		d = data->size - compare_set(set, data->itemsets + i);
		if (d > 3 || d == 0)
			continue;
		s = ((data->itemsets+i)->support - set->support) / 10 + 30;
		if (s < 0)
			s = 0;
		if (s >= 60)
			s = 59;
		++values[(s * 3) + d - 1];
	}
	return values;
}

int compare_set(struct itemset *a, struct itemset *b)
{
	int c, i;
	for (c = 0, i = 0; i < 2; ++i)
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

/* reading
 ********************************************************************/
struct dataset *read_data(char *file, int size)
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
		parse_line(sets+i, line);

		if ((sets+i)->size == size)
			++i;
		++total;
	}

	fclose(fp);

	struct dataset *result = malloc(sizeof *result);
	
	result->total_sets = total;
	result->num_sets = i;
	result->size = size;
	result->itemsets = sets;
}

void parse_line(struct itemset *dest, char *src)
{
	free(dest->items);
	int count = 0;
	uint64_t *items = calloc(2, sizeof *items);
	
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
	dest->items = items;
	dest->support = support;
}

/* memory management
 ********************************************************************/

void free_set(struct itemset *set)
{
	free(set->items);
}

/* printing
 ********************************************************************/

void print_matrix(int *values)
{
	printf("+-----------------------------+   +-----------------------------+\n");
	printf("| Dist        1      2      3 |   | Dist        1      2      3 |\n");
	printf("+-----------------------------+   +-----------------------------+\n");
	int i;
	for (i = 0; i < 30; ++i) {
		printf("| %6d %6d %6d %6d |   | %6d %6d %6d %6d |\n", (i-30)*10, values[(i*3)], values[(i*3)+1], values[(i*3)+2], i*10, values[((i+30)*3)], values[((i+30)*3)+1], values[((i+30)*3)+2]);
	}
	printf("+-----------------------------+   +-----------------------------+\n");
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
