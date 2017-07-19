#include <stdlib.h>
#include <stdio.h>
#include <omp.h>
#include <pthread.h>
#include "ga.h"


struct population_info
{
    int p_num;
    int evolution_size_loop;

    size_t size;
    size_t nextgen_size;
    size_t all_size;

    int **population;
    int **nextgen_population;
    int **all_population;
    int *pair_array;
};

void* evolution_loop(void* args)
{
    struct population_info *inf = (struct population_info *) args;

    int *seed = malloc(sizeof(int));
    seed[0] = inf->p_num;
    for (int l = 0; l < inf->evolution_size_loop; ++l)
    {
        generate_pair_array(inf->pair_array, inf->size, seed);

        crossover(inf->nextgen_population, inf->population, inf->pair_array, inf->size, seed);
        for (int i = 0; i < inf->size; ++i)
            for (int j = 0; j < NUM_X; ++j)
                inf->all_population[i][j] = inf->population[i][j];
        for (int i = 0; i < inf->nextgen_size; ++i)
            for (int j = 0; j < NUM_X; ++j)
                inf->all_population[i + inf->size][j] = inf->nextgen_population[i][j];

        mutation(inf->nextgen_population, inf->nextgen_size, seed);

        selection(inf->population, inf->all_population, inf->size, inf->all_size);
    }
    free(seed);

    return NULL;
}

int main()
{
    size_t size = 200; // Size default is 200
    size_t nextgen_size = size * 2;
    size_t all_size = nextgen_size + size;

    int default_loop_count = 10000; // Loop default is 10000
    size_t population_amount = 4;
    int migration_count = (default_loop_count / population_amount) / 500;
    int evolution_count = (default_loop_count / population_amount) / migration_count;

    int*** population = malloc(population_amount * sizeof(int**));
    int*** nextgen_population = malloc(population_amount * sizeof(int**));
    int*** all_population = malloc(population_amount * sizeof(int**));
    int** pair_array = malloc(population_amount * sizeof(int*));

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

    // Create tribes
    pthread_t tribes[population_amount]; // VLA
    struct population_info info[population_amount]; // VLA
    for(int i = 0; i < population_amount; ++i)
    {
        info[i].p_num = i;
        info[i].evolution_size_loop = evolution_count;
        info[i].size = size;
        info[i].nextgen_size = nextgen_size;
        info[i].all_size = all_size;
        info[i].population = population[i];
        info[i].nextgen_population = nextgen_population[i];
        info[i].all_population = all_population[i];
        info[i].pair_array = pair_array[i];
    }
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    double start = omp_get_wtime();
    for (int k = 0; k < migration_count; ++k)
    {
        for(int i = 0; i < population_amount; ++i)
        {
            pthread_create(tribes + i, &attr, evolution_loop, info + i);
        }

        void* status;
        for(int i = 0; i < population_amount; ++i)
        {
            pthread_join(tribes[i], &status);
        }

        migration(population, population_amount, size, common_seed);
    }
    double finish = omp_get_wtime();
    printf("Time: %lf \n", finish - start);

    // Print result
    for (int p_num = 0; p_num < population_amount; ++p_num)
        print_f(population[p_num], size);

    // Clean section
    pthread_attr_destroy(&attr);
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
