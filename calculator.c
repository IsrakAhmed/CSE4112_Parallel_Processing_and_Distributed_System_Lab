#include <stdio.h>
#include <mpi.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    MPI_Init(&argc, &argv);

    int rank, size;

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int num1 = argc > 2 ? atoi(argv[1]) : 0;
    int num2 = argc > 2 ? atoi(argv[2]) : 0;

    if (rank == 1)
    {
        printf("Rank = %d & addition = %d\n", rank, num1 + num2);
    }

    else if (rank == 2)
    {
        printf("Rank = %d & subtraction  = %d\n", rank, num1 - num2);
    }

    else if (rank == 3)
    {
        printf("Rank = %d & division  = %d\n", rank, num1 / num2);
    }

    else if (rank == 4)
    {
        printf("Rank = %d & multiplication   = %d\n", rank, num1 * num2);
    }
    
    else
    {
        printf("Rank = %d & I've nothing to do!\n", rank);
    }

    MPI_Finalize();

    return 0;
}