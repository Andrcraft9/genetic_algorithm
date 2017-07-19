#include <math.h>
#include <stdio.h>
#include <stdlib.h>


enum
{
    NUM_X = 2,
    MAX_BASE = 10,
    MUTATION_CHANCE = 25
};


/****************************** TOOLS ************************************/
int chance(int p)
{
    double pd = (double) p / 100.0;
    if (((double) rand() / RAND_MAX) <= pd)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// Random int from [a, b]
int random_in(int a, int b)
{

    return round(((b - a)) * (double)rand() / RAND_MAX) + a;
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

void init_population(int **p, size_t size)
{
    int mask = (1 << MAX_BASE) - 1;  // pow(2, MAX_BASE) - 1;
    
    for (int i = 0; i < size; ++i)
    {
        for(int j = 0; j < NUM_X; ++j)
        {
            p[i][j] = rand() & mask;
        }
    }
    return;
}

// Simple random generate
void generate_pair_array(int *pair_arr, size_t size)
{
    for (int i = 0; i < size; ++i)
    {
        //while ((pair_arr[i] = round((size - 1) * rand() / RAND_MAX)) == i)
        while ((pair_arr[i] = random_in(0, size - 1)) == i)
            ;
    }
    return;
}

// Single-point crossover
void crossover(int **nextgen_p, int **p, int *pair_arr, size_t size)
{
    size_t nextgen_size = 0;

    for (int i = 0; i < size; ++i)
    {
        //int point = round((MAX_BASE - 2) * (double)rand() / RAND_MAX) + 1; // point in [1, MAX_BASE-1]
        int point = random_in(1, MAX_BASE - 1);        
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

void mutation(int **p, size_t size)
{
    for (int i = 0; i < size; ++i)
    {
        if (chance(MUTATION_CHANCE))
        {
            for(int j = 0; j < NUM_X; ++j)
            {
                //int point = round((MAX_BASE - 1) * (double)rand() / RAND_MAX) + 1; // point in [1, MAX_BASE]
                int point = random_in(1, MAX_BASE);                
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
