#include "global.h"

int main(int argc, char *argv[])
{
	srand(time(NULL));

	printf("\nLoading frequent itemsets...\n\n");

	struct dataset *data = read_data("datasets/connect-all-30000.dat", 18, 3);

	printf("Total num frequent itemsets: %d\n", data->total_sets);
	printf("Num itemsets of size %d: %d\n\n", data->size, data->num_sets);

	//struct itemset *set = choose_rand_set(data);
	//printf("Random frequent itemset chosen:\n");
	//print_set(set);

	printf("\nSimilar frequent itemsets:\n\n");
	struct cmpr_results *rslt;

	rslt = compare_all(data, 100, 3, 80, 100);

	print_cmpr_rslt(rslt);
	write_cmpr_rslt(rslt);

	return 0;
}
