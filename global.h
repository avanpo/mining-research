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
	int min_supp;
};

struct itemset {
	int size;
	int data_len;
	uint64_t *items;
	int support;
};

struct cmpr_results {
	int dist_range;
	int supp_range;
	int supp_step;
	int *counts;
	int *t_counts;
};

/* utils.c */
struct cmpr_results *compare_all(struct dataset *, int, int, int, int);
void compare_against_all(struct cmpr_results *, struct itemset *, struct dataset *);
int compare_set(struct itemset *, struct itemset *);
int count_bits(uint64_t);
int count_sparse_bits(uint64_t);
struct itemset *choose_rand_set(struct dataset *);
struct cmpr_results *init_cmpr_rslt(int, int, int);
struct dataset *read_data(char *, int, int);
void parse_line(struct itemset *, char *, int);
void free_set(struct itemset *);
void print_cmpr_rslt(struct cmpr_results *);
void print_line(int, int);
void print_set(struct itemset *);
void print_set_bits(struct itemset *);
void print_uint64_bits(uint64_t);
