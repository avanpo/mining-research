#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

struct dataset {
	int total_sets;
	int num_sets;
	int size;
	struct itemset *itemsets;
};

struct itemset {
	int size;
	uint64_t *items;
	int support;
};

/* utils.c */
int *compare_against_all(struct itemset *, struct dataset *);
int compare_set(struct itemset *, struct itemset *);
int count_bits(uint64_t);
int count_sparse_bits(uint64_t);
struct itemset *choose_rand_set(struct dataset *);
struct dataset *read_data(char *file, int size);
void parse_line(struct itemset *, char *);
void free_set(struct itemset *);
void print_matrix(int *);
void print_set(struct itemset *);
void print_set_bits(struct itemset *);
void print_uint64_bits(uint64_t);
