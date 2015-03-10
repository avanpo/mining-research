#include "global.h"

int main(int argc, char *argv[])
{
	srand(time(NULL));

	printf("\nLoading frequent itemsets...\n\n");

	struct dataset *data = read_data("frequent-sets/mushroom-256.dat", 14);

	printf("Total num frequent itemsets: %d\n", data->total_sets);
	printf("Num itemsets of size %d: %d\n\n", data->size, data->num_sets);

	struct itemset *set = choose_rand_set(data);
	printf("Random frequent itemset chosen:\n");
	print_set(set);

	printf("\nSimilar frequent itemsets:\n\n");
	int *values = compare_against_all(set, data);
	print_matrix(values);

	return 0;
}
