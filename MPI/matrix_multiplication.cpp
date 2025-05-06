#include <bits/stdc++.h>
#include <mpi.h>

int main(int argc, char **argv)
{

    // Initialize MPI environment
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int K, M, N, P;

    K = 100, M = 50, N = 50, P = 50;

    /*K = argv[1];
    M = argv[2];
    N = argv[3];
    P = argv[4];*/

    /*if(rank == 0) {
        // Get matrix dimensions from user
        printf("Enter the number of matrices (K): ");
        scanf("%d", &K);
        printf("Enter the number of rows in A (M): ");
        scanf("%d", &M);
        printf("Enter the number of columns in A and rows in B (N): ");
        scanf("%d", &N);
        printf("Enter the number of columns in B (P): ");
        scanf("%d", &P);
    }*/

    // Broadcast matrix dimensions to all processes
    MPI_Bcast(&K, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&M, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&N, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&P, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if (K % size != 0)
    {
        if (rank == 0)
        {
            printf("Number of matrices must be divisible by then number of processes.\n");
        }

        MPI_Finalize();

        return 1;
    }

    // Declare global matrices (only used in root process)
    // int A[K][M][N], B[K][N][P], Result[K][M][P];

    int (*A)[M][N] = (int (*)[M][N])malloc(K * sizeof(*A));
    int (*B)[N][P] = (int (*)[N][P])malloc(K * sizeof(*B));
    int (*Result)[M][P] = (int (*)[M][P])malloc(K * sizeof(*Result));

    // Initialize matrices A and B with random values in the root process
    if (rank == 0)
    {
        for (int i = 0; i < K; i++)
        {
            for (int j = 0; j < M; j++)
            {
                for (int l = 0; l < N; l++)
                {
                    A[i][j][l] = rand() % 100;
                }
            }
            for (int j = 0; j < N; j++)
            {
                for (int l = 0; l < P; l++)
                {
                    B[i][j][l] = rand() % 100;
                }
            }
        }
    }

    // Local buffers to store the portion of data each process will handle
    // int localA[K / size][M][N], localB[K / size][N][P], localResult[K / size][M][P];

    int (*localA)[M][N] = (int (*)[M][N])malloc((K / size) * sizeof(*localA));
    int (*localB)[N][P] = (int (*)[N][P])malloc((K / size) * sizeof(*localB));
    int (*localResult)[M][P] = (int (*)[M][P])malloc((K / size) * sizeof(*localResult));

    // Distribute matrix A and B among all processes
    MPI_Scatter(A, (K / size) * M * N, MPI_INT, localA, (K / size) * M * N, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(B, (K / size) * N * P, MPI_INT, localB, (K / size) * N * P, MPI_INT, 0, MPI_COMM_WORLD);

    // Ensure all process sync
    MPI_Barrier(MPI_COMM_WORLD);

    // Start timer for performance measurement
    double startTime = MPI_Wtime();

    // Perform matrix multiplication for assigned local blocks
    for (int i = 0; i < (K / size); i++)
    {
        for (int j = 0; j < M; j++)
        {
            for (int l = 0; l < P; l++)
            {
                localResult[i][j][l] = 0;

                for (int x = 0; x < N; x++)
                {
                    // Multiply A and B element-wise and add to sum
                    localResult[i][j][l] += (localA[i][j][x] * localB[i][j][x]) % 100;
                }

                localResult[i][j][l] %= 100;

                // Modulo 100 keeps numbers small and manageable (between 0 and 99)
            }
        }
    }

    // Stop Timer after computation
    double endTime = MPI_Wtime();

    // Gather comoputed results from all processes into final result matrix Result[K][M][P] in root
    MPI_Gather(localResult, (K / size) * M * P, MPI_INT, Result, (K / size) * M * P, MPI_INT, 0, MPI_COMM_WORLD);

    // Display all result matrices from root process
    /*if(rank == 0){
        for(int i = 0; i < K; i++) {
            printf("Result Matrix %d\n", i);

            for(int j = 0; j < M; j++) {
                for(int l = 0; l < P; l++) {
                    printf("%d ", Result[i][j][l]);
                }
                printf("\n");
            }
            printf("\n");
        }
    }*/

    // Ensure all processes finish before printing time
    MPI_Barrier(MPI_COMM_WORLD);

    // Print time taken by each process
    printf("Process %d: Time take = %f seconds\n", rank, endTime - startTime);

    free(A);
    free(B);
    free(Result);
    free(localA);
    free(localB);
    free(localResult);

    // Finalize MPI environment
    MPI_Finalize();

    return 0;
}