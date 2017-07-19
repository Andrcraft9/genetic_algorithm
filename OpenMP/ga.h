#include <stdlib.h>
#include <stdio.h>
#include <math.h>


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

void print_f(int** p, size_t size)
{
    for(int i = 0; i < size; ++i)    
        printf("%d ", fitness(p[i]));
    
    printf("\n\n");
    return;
}

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

void migration(int ***p, size_t p_amount, size_t size, unsigned int *seed)
{
    for (int p_num = 0; p_num < p_amount; ++p_num)
    {
        for (int i = 0; i < size; ++i)
        {
            if (chance(MIGRATION_CHANCE, seed))
            {
                int swap_num = random_in(0, p_amount - 1, seed);
                if (swap_num == p_num)
                    swap_num = (swap_num + 1) % p_amount;
                
                int swap_i = random_in(0, size - 1, seed);
                
                // swap
                int* tmp = p[p_num][i];
                p[p_num][i] = p[swap_num][swap_i];
                p[swap_num][swap_i] = tmp;
            }
        }
    }
    
    return;
}
/*************************************************************************/
