#include <mpi.h>
#include <bits/stdc++.h>


/*

mpic++ pattern_search.cpp -o pattern_search
mpirun -np 4 ./pattern_search <filename> <pattern>
Example: mpi run -np 4 ./pattern_search text.txt "pattern"

*/


using namespace std;

// Count how many times pattern appears in text
int count_pattern(const string &text, const string &pattern)
{
    int count = 0;
    size_t pos = 0;
    while ((pos = text.find(pattern, pos)) != string::npos)
    {
        ++count;
        pos += pattern.length();
    }
    return count;
}

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc != 3)
    {
        if (rank == 0)
            cerr << "Usage: mpirun -np <num_processes> " << argv[0] << " <filename> <pattern>\n";
        MPI_Finalize();
        return 1;
    }

    string filename = argv[1];
    string pattern = argv[2];
    string text;

    double start_time, end_time;

    if (rank == 0)
    {
        // Read file content
        ifstream file(filename);
        if (!file)
        {
            cerr << "Failed to open file: " << filename << "\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        string line;
        while (getline(file, line))
        {
            text += line + '\n';
        }
        file.close();
    }

    // Broadcast pattern length and content
    int pattern_len = pattern.length();
    MPI_Bcast(&pattern_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0)
        pattern.resize(pattern_len);
    MPI_Bcast(&pattern[0], pattern_len, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Broadcast full text length and content
    int text_len;
    if (rank == 0)
        text_len = text.length();
    MPI_Bcast(&text_len, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0)
        text.resize(text_len);
    MPI_Bcast(&text[0], text_len, MPI_CHAR, 0, MPI_COMM_WORLD);

    start_time = MPI_Wtime();

    // Calculate local range for each process
    int chunk_size = text_len / size;
    int start = rank * chunk_size;
    int end = (rank == size - 1) ? text_len : start + chunk_size;

    // Handle overlapping regions to avoid missing patterns across chunk borders
    if (rank != 0)
        start -= pattern_len - 1;
    if (rank != size - 1)
        end += pattern_len - 1;

    string local_text = text.substr(start, end - start);
    int local_count = count_pattern(local_text, pattern);

    int total_count;
    MPI_Reduce(&local_count, &total_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();

    if (rank == 0)
    {
        cout << "Total occurrences of pattern '" << pattern << "': " << total_count << endl;
        cout << "Total time taken: " << (end_time - start_time) << " seconds\n";
    }

    MPI_Finalize();
    return 0;
}
