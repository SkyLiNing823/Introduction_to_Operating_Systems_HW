#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdio.h>
#include <iostream>
using namespace std;

void createAB(unsigned int *A, unsigned int *B, int dim)
{
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            A[dim * i + j] = dim * i + j;
            B[dim * i + j] = dim * i + j;
        }
    }
    return;
}

void initializeC(unsigned int *C, int dim)
{
    for (int i = 0; i < dim; i++)
    {
        for (int j = 0; j < dim; j++)
        {
            C[dim * i + j] = 0;
        }
    }
    return;
}

int main()
{
    int dim, idx, unit;
    struct timeval start, end;
    pid_t pid;
    printf("Please input the matrix dimension:");
    scanf("%d", &dim);
    int Aid = shmget(0, dim * dim * 4, IPC_CREAT | 0600);
    int Bid = shmget(0, dim * dim * 4, IPC_CREAT | 0600);
    int Cid = shmget(0, dim * dim * 4, IPC_CREAT | 0600);

    unsigned int *A = (unsigned int *)shmat(Aid, NULL, 0);
    unsigned int *B = (unsigned int *)shmat(Bid, NULL, 0);
    unsigned int *C = (unsigned int *)shmat(Cid, NULL, 0);
    createAB(A, B, dim);

    for (int i = 1; i <= 16; i++)
    {
        printf("Multiplying matrices using %d processes\n", i);
        initializeC(C, dim);
        gettimeofday(&start, 0);
        // do something
        unit = dim / i;
        idx = 0;

        for (int j = 1; j <= i; j++)
        {
            if (i == j)
                unit = dim - j * unit + unit;
            // printf("start: %d, end: %d\n", idx, idx + unit);
            pid = fork();
            if (pid < 0)
            {
                fprintf(stderr, "Fork Failed");
                exit(-1);
            }
            else if (pid == 0)
            {
                for (int x = idx; x < idx + unit; x++)
                    for (int y = 0; y < dim; y++)
                        for (int z = 0; z < dim; z++)
                        {
                            C[x * dim + y] += A[x * dim + z] * B[z * dim + y];
                        }
                exit(0);
            }
            idx += unit;
        }
        for (int j = 1; j <= i; j++)
        {
            wait(0);
        }
        gettimeofday(&end, 0);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;
        unsigned int sum = 0;
        for (int row = 0; row < dim; row++)
            for (int col = 0; col < dim; col++)
                sum += C[dim * row + col];
        printf("Elapsed time: %f sec, Checksum: %u\n", sec + (usec / 1000000.0), sum);
    }
    shmdt(A);
    shmdt(B);
    shmdt(C);
    shmctl(Aid, IPC_RMID, NULL);
    shmctl(Bid, IPC_RMID, NULL);
    shmctl(Cid, IPC_RMID, NULL);
    return 0;
}