#include <bits/stdc++.h>
#include <mpi.h>

using namespace std;

vector<string> split_by_newline(const string &data)
{
    vector<string> words;
    stringstream ss(data);
    string line;
    while (getline(ss, line))
    {
        if (!line.empty())
            words.push_back(line);
    }
    return words;
}

// Serialize map<string, int> into string
string serialize_map(const unordered_map<string, int> &map)
{
    stringstream ss;
    for (const auto &pair : map)
        ss << pair.first << " " << pair.second << "\n";
    return ss.str();
}

// Deserialize string to map<string, int>
unordered_map<string, int> deserialize_map(const string &data)
{
    unordered_map<string, int> map;
    istringstream iss(data);
    string word;
    int count;
    while (iss >> word >> count)
        map[word] += count;
    return map;
}

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Barrier(MPI_COMM_WORLD);        // Synchronize all processes
    double start_time = MPI_Wtime();    // Start timer

    string fileName = "words.txt";
    vector<string> allWords;

    int totalWords;

    vector<int> sendCounts(size), displs(size);
    vector<char> flatWords;

    if (rank == 0)
    {
        ifstream file(fileName);

        if (!file.is_open())
        {
            printf("Could not open the file\n");

            MPI_Finalize();
            return 1;
        }

        string word;
        while (file >> word)
        {
            allWords.push_back(word);
        }

        file.close();

        totalWords = allWords.size();

        //cout << "Total words: " << totalWords << "\n";

        int wordsPerProcess = totalWords / size;
        int extraWords = totalWords % size;

        vector<int> wordsPerProc(size);
        for (int i = 0; i < size; i++)
            wordsPerProc[i] = wordsPerProcess + (i < extraWords ? 1 : 0);

        vector<int> wordLengths(size);

        int index = 0;

        for (int i = 0; i < size; i++)
        {
            int count = wordsPerProc[i];
            string segment;

            for (int j = 0; j < count; j++)
            {
                segment += allWords[index + j] + '\n';
            }

            index += count;
            wordLengths[i] = segment.size();
            flatWords.insert(flatWords.end(), segment.begin(), segment.end());
        }

        sendCounts[0] = wordLengths[0];
        displs[0] = 0;
        for (int i = 1; i < size; i++)
        {
            sendCounts[i] = wordLengths[i];
            displs[i] = displs[i - 1] + wordLengths[i - 1];
        }
    }

    // Broadcast sendCounts/displs to all processes
    MPI_Bcast(sendCounts.data(), size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(displs.data(), size, MPI_INT, 0, MPI_COMM_WORLD);

    vector<char> localWordsBuffer(sendCounts[rank]);

    MPI_Scatterv(rank == 0 ? flatWords.data() : nullptr, sendCounts.data(), displs.data(), MPI_CHAR,
                 localWordsBuffer.data(), sendCounts[rank], MPI_CHAR, 0, MPI_COMM_WORLD);

    // Process local data
    string localData(localWordsBuffer.begin(), localWordsBuffer.end());
    //cout << "Process " << rank << " received " << localData.size() << " characters\n";
    vector<string> localWords = split_by_newline(localData);

    // Count local word frequencies
    unordered_map<string, int> local_map;
    for (const auto &w : localWords)
        local_map[w]++;

    // Serialize local map and send to root
    string serialized = serialize_map(local_map);
    int length = serialized.size();
    vector<int> lengths(size);

    MPI_Gather(&length, 1, MPI_INT, lengths.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);

    vector<int> displs_gather(size);
    int total_size = 0;
    string all_data;

    if (rank == 0)
    {
        for (int i = 0; i < size; i++)
        {
            displs_gather[i] = total_size;
            total_size += lengths[i];
        }
        all_data.resize(total_size);
    }

    MPI_Gatherv(serialized.data(), length, MPI_CHAR,
                rank == 0 ? &all_data[0] : nullptr,
                lengths.data(), displs_gather.data(), MPI_CHAR,
                0, MPI_COMM_WORLD);

    if (rank == 0)
    {
        // Merge maps
        unordered_map<string, int> global_map;
        for (int i = 0; i < size; i++)
        {
            string part = all_data.substr(displs_gather[i], lengths[i]);
            auto temp = deserialize_map(part);
            for (const auto &[word, count] : temp)
                global_map[word] += count;
        }

        // Sort by frequency (desc)
        vector<pair<string, int>> sorted_words(global_map.begin(), global_map.end());

        sort(sorted_words.begin(), sorted_words.end(),
             [](const auto &a, const auto &b)
             {
                 return a.second > b.second;
             });

        double end_time = MPI_Wtime();      // End timer

        cout << "\nTop 10 Frequent Words:\n";
        for (int i = 0; i < 10 && i < (int)sorted_words.size(); i++)
        {
            cout << sorted_words[i].first << ": " << sorted_words[i].second << "\n";
        }

        cout << "\nTotal Time: " << (end_time - start_time) << " seconds\n";
    }

    MPI_Finalize();

    return 0;
}