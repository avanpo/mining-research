#include "global.h"

struct histogram *get_histogram(struct dataset *data, int norm)
{
	struct histogram *hist = malloc(sizeof *hist);
	int *vals = calloc(data->max_set_size, sizeof *vals);

	int i;
	for (i = 0; i < data->num_sets; ++i) {
		++vals[(data->itemsets + i)->size];
	}

	hist->min = 0;
	hist->max = data->max_set_size;
	hist->step = 1;
	hist->vals = vals;

	return hist;
}

int main(int argc, char *argv[])
{
	int len;
	sscanf(argv[2], "%d", &len);

	struct dataset *data = read_data(argv[1], len);
	
	struct histogram *h = get_histogram(data);
	print_histogram(h);
	write_histogram(h);
}
