#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include "ga.h"


int main()
{
    size_t size = 200; // Size default is 200
    size_t nextgen_size = size * 2;
    size_t all_size = nextgen_size + size;

    int default_loop_count = 10000; // Loop default is 10000
    size_t population_amount = 4;
    int migration_count = (default_loop_count / population_amount) / 500;
    int evolution_count = (default_loop_count / population_amount) / migration_count;

    int ***population = malloc(population_amount * sizeof(int**));
    int ***nextgen_population = malloc(population_amount * sizeof(int**));
    int ***all_population = malloc(population_amount * sizeof(int**));
    int **pair_array = malloc(population_amount * sizeof(int*));

    int *common_seed = malloc(sizeof(int));
    common_seed[0] = time(NULL);

    for (int p_num = 0; p_num < population_amount; ++p_num)
    {
        population[p_num] = init_array(size);
        nextgen_population[p_num] = init_array(nextgen_size);
        all_population[p_num] = init_array(all_size);
        pair_array[p_num] = malloc(size * sizeof(int));

        init_population(population[p_num], size, common_seed);
    }

    double start = omp_get_wtime();
    for (int k = 0; k < migration_count; ++k)
    {
        #pragma omp parallel num_threads(4)
        {
            #pragma omp for schedule (static, 1)
            for (int p_num = 0; p_num < population_amount; ++p_num)
            {
                int *seed = malloc(sizeof(int));
                seed[0] = p_num;

                for (int l = 0; l < evolution_count; ++l)
                {
                    generate_pair_array(pair_array[p_num], size, seed);

                    crossover(nextgen_population[p_num], population[p_num], pair_array[p_num], size, seed);
                    for (int i = 0; i < size; ++i)
                        for (int j = 0; j < NUM_X; ++j)
                            all_population[p_num][i][j] = population[p_num][i][j];
                    for (int i = 0; i < nextgen_size; ++i)
                        for (int j = 0; j < NUM_X; ++j)
                            all_population[p_num][i + size][j] = nextgen_population[p_num][i][j];

                    mutation(nextgen_population[p_num], nextgen_size, seed);

                    selection(population[p_num], all_population[p_num], size, all_size);

                }
                free(seed);
            }
        }

        migration(population, population_amount, size, common_seed);
    }
    double finish = omp_get_wtime();
    printf("Time: %lf \n", finish - start);

    // Print result
    for (int p_num = 0; p_num < population_amount; ++p_num)
        print_f(population[p_num], size);

    // Clean section
    for (int p_num = 0; p_num < population_amount; ++p_num)
    {
        free_array(population[p_num], size);
        free_array(nextgen_population[p_num], nextgen_size);
        free_array(all_population[p_num], all_size);
        free(pair_array[p_num]);
    }
    free(population);
    free(nextgen_population);
    free(all_population);
    free(pair_array);
    free(common_seed);

    return 0;
}
