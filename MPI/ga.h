#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <mpi.h>

enum
{
    NUM_X = 2,
    MAX_BASE = 10,
    MUTATION_CHANCE = 25,
    MIGRATION_CHANCE = 10
};

/****************************** TOOLS ************************************/
int chance(int p, unsigned int *seed)
{
    double pd = (double) p / 100.0;
    if (((double) rand_r(seed) / RAND_MAX) <= pd)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// Random int from [a, b]
int random_in(int a, int b, unsigned int *seed)
{

    return round(((b - a)) * (double)rand_r(seed) / RAND_MAX) + a;
}

int** init_array(size_t size)
{
    int **p = malloc(size * sizeof(int*));
    for (int i = 0; i < size; ++i)
    {
        p[i] = malloc(NUM_X * sizeof(int));
    }

    return p;
}

void free_array(int **p, size_t size)
{
    for (int i = 0; i < size; ++i)
        free(p[i]);
    free(p);

    return;
}
/*************************************************************************/


/***************************** GA TOOLS **********************************/
// Sphere function f(x1, x2, ..., xN). N = NUM_X.
int fitness(int* x)
{
    int f = 0;
    for (int j = 0; j < NUM_X; ++j)
    {
        int a = x[j] - (1 << (MAX_BASE - 1));
        f = f + a * a;
    }
    return f;
}
void mpi_print_f(int p_num, int p_size, int** p, size_t size)
{
    for(int k = 0; k < p_size; k++)
    {
        MPI_Barrier(MPI_COMM_WORLD);

        if (k == p_num)
        {
            printf("p_num: %d\n", p_num);
            for(int i = 0; i < size; ++i)
            {
                printf("%d ", fitness(p[i]));
            }
            printf("\n\n");
            fflush(stdout);
        }
    }
}
/*
void print_p(int** p, size_t size)
{
    for(int i = 0; i < size; ++i)
    {
       for(int j = 0; j < NUM_X; ++j)
        {
            int x = p[i][j];
            printf("%d ", x);
        }
        printf("\n");
    }
    printf("\n\n");
    return;
}
*/
void init_population(int **p, size_t size, int *seed)
{
    int mask = (1 << MAX_BASE) - 1;  // pow(2, MAX_BASE) - 1;

    for (int i = 0; i < size; ++i)
    {
        for(int j = 0; j < NUM_X; ++j)
        {
            p[i][j] = rand_r(seed) & mask;
        }
    }
    return;
}

// Simple random generate
void generate_pair_array(int *pair_arr, size_t size, unsigned int *seed)
{
    for (int i = 0; i < size; ++i)
    {
        //while ((pair_arr[i] = round((size - 1) * rand() / RAND_MAX)) == i)
        while ((pair_arr[i] = random_in(0, size - 1, seed)) == i)
            ;
    }
    return;
}

// Single-point crossover
void crossover(int **nextgen_p, int **p, int *pair_arr, size_t size, unsigned int *seed)
{
    size_t nextgen_size = 0;

    for (int i = 0; i < size; ++i)
    {
        //int point = round((MAX_BASE - 2) * (double)rand() / RAND_MAX) + 1; // point in [1, MAX_BASE-1]
        int point = random_in(1, MAX_BASE - 1, seed);
        int mask1 = (1 << point) - 1; // pow(2, point) - 1;
        int mask2 = (~mask1);

        for (int j = 0; j < NUM_X; ++j)
        {
            int a, b;
            a = p[i][j];
            b = p[pair_arr[i]][j];

            nextgen_p[nextgen_size][j] = (a & mask1) | ( b & mask2);

            nextgen_p[nextgen_size + 1][j] = (b & mask1) | (a & mask2);
        }

        nextgen_size = nextgen_size + 2;
    }

    return;
}

void mutation(int **p, size_t size, unsigned int *seed)
{
    for (int i = 0; i < size; ++i)
    {
        if (chance(MUTATION_CHANCE, seed))
        {
            for(int j = 0; j < NUM_X; ++j)
            {
                //int point = round((MAX_BASE - 1) * (double)rand() / RAND_MAX) + 1; // point in [1, MAX_BASE]
                int point = random_in(1, MAX_BASE, seed);
                int mask = 1 << (point - 1); // pow(2, point - 1);

                p[i][j] = p[i][j] ^ mask;
            }
        }
    }

    return;
}

int cmp_fit(const void *x1, const void *x2)
{
    int *a, *b;
    a = *(int**)x1;
    b = *(int**)x2;

    return fitness(a) - fitness(b);
}

void selection(int **new_p, int **old_p, size_t new_size, size_t old_size)
{
    qsort(old_p, old_size, sizeof(int*), cmp_fit);

    for (int i = 0; i < new_size; ++i)
        for (int j = 0; j < NUM_X; ++j)
            new_p[i][j] = old_p[i][j];

    return;
}

int generate_migration_array(int **m_a, int p_size, size_t size, unsigned int *seed)
{
    int size_m_a = 0;
    int j = 0;

    for(int i = 0; i < p_size * size; ++i)
    {
        if (chance(MIGRATION_CHANCE, seed))
        {
            int swap1_num = random_in(0, p_size - 1, seed);
            int swap2_num = random_in(0, p_size - 1, seed);
            if (swap1_num == swap2_num)
                swap2_num = (swap2_num + 1) % p_size;

            m_a[0][j] = swap1_num;
            m_a[1][j] = swap2_num;
            ++j;

            ++size_m_a;
        }
    }

    return size_m_a;
}

void migration(int p_num, int **population, int **m_a, size_t size, int size_m_a, unsigned int *seed)
{
    int *tmp = malloc(NUM_X * sizeof(int));
    for (int i = 0; i < size_m_a; ++i)
    {
        if (m_a[0][i] == p_num)
        {
            // Then p_num is first sender

            // generate number of swap element
            int swap_i = random_in(0, size - 1, seed);

            MPI_Send(population[swap_i], NUM_X, MPI_INT, m_a[1][i], 0, MPI_COMM_WORLD);
            MPI_Recv(tmp, NUM_X, MPI_INT, m_a[1][i], MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int nx = 0; nx < NUM_X; ++nx)
                population[swap_i][nx] = tmp[nx];
        }
        else if (m_a[1][i] == p_num)
        {
            // Then p_num is firsr reciver

            // generate number of swap element
            int swap_i = random_in(0, size - 1, seed);

            MPI_Recv(tmp, NUM_X, MPI_INT, m_a[0][i], MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            MPI_Send(population[swap_i], NUM_X, MPI_INT, m_a[0][i], 0, MPI_COMM_WORLD);
            for (int nx = 0; nx < NUM_X; ++nx)
                population[swap_i][nx] = tmp[nx];
        }
        else
        {
            // Then ingore

        }
    }
    free(tmp);

    return;
}
/*************************************************************************/
