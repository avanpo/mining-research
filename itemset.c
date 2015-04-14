#include "global.h"

/* Main definition for an itemset. Stored as active bits in a uint64_t array.
 * ****************************************************************************/

/* operations */

int cmpr_sets(struct itemset *a, struct itemset *b)
{
	int c, i;
	for (c = 0, i = 0; i < a->data_len; ++i)
		c += count_bits(a->items[i] & b->items[i]);
	return c;
}

/* constructor / destructor */

void parse_set(struct itemset *dest, char *src, int data_len)
{
	free(dest->items);
	int count = 0;
	uint64_t *items = calloc(data_len, sizeof *items);
	
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
	dest->data_len = data_len;
	dest->items = items;
	dest->support = support;
}

void free_set(struct itemset *set)
{
	free(set->items);
}

/* printing / writing */

void print_set(struct itemset *set)
{
	int i, j;
	uint64_t z;
	for (i = 0, j = 1; i < set->data_len; ++i) {
		for (z = 1; z != 0; z <<= 1, ++j) {
			if (set->items[i] & z)
				printf("%d ", j);
		}
	}
	printf("(%d)\n", set->support);
}

void write_set(struct itemset *set, FILE *fp)
{
	int i, j;
	uint64_t z;
	for (i = 0, j = 1; i < set->data_len; ++i) {
		for (z = 1; z != 0; z <<= 1, ++j) {
			if (set->items[i] & z)
				fprintf(fp, "%d ", j);
		}
	}
	fprintf(fp, "(%d)\n", set->support);
}

void print_set_bits(struct itemset *set)
{
	int i;
	for (i = set->data_len - 1; i >= 0; --i) {
		print_bits(set->items[i]);
	}
	printf(" (%d)\n", set->support);
}
