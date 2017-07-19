#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "ga.h"


int main()
{
    size_t size = 200; // Default is 200
    size_t nextgen_size = size * 2;
    size_t all_size = nextgen_size + size;

    int default_loop_count = 10000; // Loop default is 10000

    int **population = init_array(size);
    int **nextgen_population = init_array(nextgen_size);
    int **all_population = init_array(all_size);
    int *pair_array = malloc(size * sizeof(int));

    srand(time(NULL));

    clock_t start = clock();
    init_population(population, size);
    for (int k = 0; k < default_loop_count; ++k)
    {
        generate_pair_array(pair_array, size);

        crossover(nextgen_population, population, pair_array, size);

        for (int i = 0; i < size; ++i)
            for (int j = 0; j < NUM_X; ++j)
                all_population[i][j] = population[i][j];
        for (int i = 0; i < nextgen_size; ++i)
            for (int j = 0; j < NUM_X; ++j)
                all_population[i + size][j] = nextgen_population[i][j];

        mutation(nextgen_population, nextgen_size);

        selection(population, all_population, size, all_size);
    }
    clock_t finish = clock(); printf("Time: %lf\n", (double) (finish - start) / CLOCKS_PER_SEC);

    print_f(population, size);

    free_array(population, size);
    free_array(nextgen_population, nextgen_size);
    free_array(all_population, all_size);
    free(pair_array);

    return 0;
}
