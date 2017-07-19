#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <mpi.h>
#include "ga.h"

int main(int argc, char** argv)
{
    size_t size = 200; // Size default is 200
    size_t nextgen_size = size * 2;
    size_t all_size = nextgen_size + size;

    int default_loop_count = 10000; // Loop default is 10000
    int migration_count;
    int evolution_count;

    /******************************************************************* Parallel section begin *******************************************************************/
    MPI_Init(&argc, &argv);

    int p_size, p_num;
    MPI_Comm_size(MPI_COMM_WORLD, &p_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &p_num);
    migration_count = (default_loop_count / p_size) / 500;
    evolution_count = (default_loop_count / p_size) / migration_count;
    //printf("size: %d num: %d \n", p_size, p_num);

    int **population = init_array(size);
    int **nextgen_population = init_array(nextgen_size);
    int **all_population = init_array(all_size);
    int *pair_array = malloc(size * sizeof(int));
    int *seed = malloc(sizeof(int));
    seed[0] = p_num;

    init_population(population, size, seed);

    double start = MPI_Wtime();
    for (int k = 0; k < migration_count; ++k)
    {
        for (int l = 0; l < evolution_count; ++l)
        {
            generate_pair_array(pair_array, size, seed);

            crossover(nextgen_population, population, pair_array, size, seed);
            for (int i = 0; i < size; ++i)
                for (int j = 0; j < NUM_X; ++j)
                    all_population[i][j] = population[i][j];
            for (int i = 0; i < nextgen_size; ++i)
                for (int j = 0; j < NUM_X; ++j)
                    all_population[i + size][j] = nextgen_population[i][j];

            mutation(nextgen_population, nextgen_size, seed);

            selection(population, all_population, size, all_size);
        }
        // Generate scheme for migration
        int **m_a = malloc(2 * sizeof(int*));
        int size_m_a;
        m_a[0] = malloc(p_size * size * sizeof(int));
        m_a[1] = malloc(p_size * size * sizeof(int));
        if (p_num == 0)
        {
            size_m_a = generate_migration_array(m_a, p_size, size, seed);

            MPI_Bcast(&size_m_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(m_a[0], size_m_a, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(m_a[1], size_m_a, MPI_INT, 0, MPI_COMM_WORLD);

        }
        else
        {
            MPI_Bcast(&size_m_a, 1, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(m_a[0], size_m_a, MPI_INT, 0, MPI_COMM_WORLD);
            MPI_Bcast(m_a[1], size_m_a, MPI_INT, 0, MPI_COMM_WORLD);
        }
        
        // Do migration
        //mpi_print_f(p_num, p_size, population, size);
        migration(p_num, population, m_a, size, size_m_a, seed);
        //mpi_print_f(p_num, p_size, population, size);

    }
    double finish = MPI_Wtime();
    printf("Time: %lf \n", finish - start);

    // Print result
    mpi_print_f(p_num, p_size, population, size);

    // Clean section
    free_array(population, size);
    free_array(nextgen_population, nextgen_size);
    free_array(all_population, all_size);
    free(pair_array);
    free(seed);

    MPI_Finalize();
    /******************************************************************* Parallel section end *******************************************************************/


    return 0;
}
