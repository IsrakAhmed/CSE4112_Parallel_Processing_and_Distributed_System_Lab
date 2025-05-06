#include <bits/stdc++.h>
#include <mpi.h>

/*
    mpic++ phonebook.cpp -o pb
    mpirun -np 4 ./pb contact_list.txt Bob
*/

using namespace std;

struct Contact
{
    string name;
    string phone;
    int lineNumber;
};

// Converts a slice of Contact vector into a single string
string vector_to_string(const vector<Contact> &contacts, int start, int end)
{
    string result;
    for (int i = start; i < min((int)contacts.size(), end); i++)
    {
        result += contacts[i].name + "," + contacts[i].phone + "," + to_string(contacts[i].lineNumber) + "\n";
    }
    return result;
}

// Sends a string to a receiver process using MPI
void send_string(const string &text, int receiver)
{
    int len = text.size() + 1;                                          // Include null terminator
    MPI_Send(&len, 1, MPI_INT, receiver, 1, MPI_COMM_WORLD);            // Send length
    MPI_Send(text.c_str(), len, MPI_CHAR, receiver, 1, MPI_COMM_WORLD); // Send content
}

// Parses a string of "name, phone  and line number" lines into Contact vector
vector<Contact> string_to_contacts(const string &text)
{
    vector<Contact> contacts;
    istringstream iss(text);
    string line;
    while (getline(iss, line))
    {
        if (line.empty())
            continue;
        vector<string> tokens;
        istringstream line_ss(line);
        string token;
        while (getline(line_ss, token, ','))
        {
            tokens.push_back(token);
        }
        if (tokens.size() != 3)
            continue; // Skip malformed lines

        contacts.push_back({tokens[0], tokens[1], stoi(tokens[2])});
    }
    return contacts;
}

// Receives a string from a sender process using MPI
string receive_string(int sender)
{
    int len;
    MPI_Recv(&len, 1, MPI_INT, sender, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Receive length
    char *buf = new char[len];
    MPI_Recv(buf, len, MPI_CHAR, sender, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE); // Receive content
    string res(buf);
    delete[] buf;
    return res;
}

// Checks if the contact name contains the search term
string check(const Contact &c, const string &search)
{
    if (c.phone.find(search) != string::npos)
    {
        return "Line " + to_string(c.lineNumber) + " : " + c.name + " " + c.phone + "\n"; // match found
    }
    return "";
}

int main(int argc, char **argv)
{

    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get process ID
    MPI_Comm_size(MPI_COMM_WORLD, &size); // Get total number of processes

    string search_term = argv[1]; // Get search term from command line arguments
    if (search_term.empty())
    {
        printf("Search term cannot be empty.\n");
        MPI_Abort(MPI_COMM_WORLD, 1); // Abort if search term is empty
    }

    int search_term_len;
    double start, end;

    // string file_name = argv[1];     // Get file name from command line arguments
    string file_name = "contact_list.txt";

    vector<double> all_times;

    if (rank == 0)
    {

        all_times.resize(size);

        search_term_len = search_term.size(); // Get length of the search term
        if (search_term_len == 0)
        {
            printf("Search term cannot be empty.\n");
            MPI_Abort(MPI_COMM_WORLD, 1); // Abort if search term is empty
        }

        MPI_Bcast(&search_term_len, 1, MPI_INT, 0, MPI_COMM_WORLD); // Broadcast the length of the search term

        MPI_Bcast(&search_term[0], search_term.size() + 1, MPI_CHAR, 0, MPI_COMM_WORLD); // Broadcast the search term to all processes

        vector<Contact> contacts;

        ifstream file(file_name); // Open the file
        if (!file.is_open())
        {
            printf("Error opening file: %s\n", file_name.c_str());
            MPI_Abort(MPI_COMM_WORLD, 1); // Abort if file cannot be opened
        }

        string line;
        int i = 1;
        while (getline(file, line))
        {
            if (line.empty())
                continue;
            istringstream iss(line);
            string name, phone;
            iss >> name >> phone; // read space-separated name and phone
            contacts.push_back({name, phone, i});
            i++;
        }

        file.close();

        sort(contacts.begin(), contacts.end(), [](const Contact &a, const Contact &b)
             { return a.name < b.name; });

        int total = contacts.size();
        int chunk = (total + size - 1) / size; // Divide work among processes

        // Distribute contact chunks to worker processes
        for (int i = 1; i < size; i++)
        {
            string text = vector_to_string(contacts, i * chunk, (i + 1) * chunk);
            send_string(text, i);
        }

        // Root process does its own chunk
        start = MPI_Wtime();
        string result;
        for (int i = 0; i < min(chunk, total); i++)
        {
            string match = check(contacts[i], search_term);
            if (!match.empty())
                result += match;
        }
        end = MPI_Wtime();

        double my_time = end - start;

        // Collect result from other processes
        for (int i = 1; i < size; i++)
        {
            string recv = receive_string(i);
            if (!recv.empty())
                result += recv;
        }

        MPI_Gather(&my_time, 1, MPI_DOUBLE, all_times.data(), 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        /*if (result.empty())
            printf("\nNo matches found for '%s'.\n\n", search_term.c_str());
        else
            printf("\nSearch result: \n%s\n\n", result.c_str());

        printf("Process %d took %f seconds.\n", rank, end - start);*/

        ofstream outfile("output.txt");

        if (result.empty())
            outfile << "\nNo matches found for '" << search_term << "'.\n\n";
        else
            outfile << "\nSearch result: \n"
                    << result << "\n\n";

        outfile << "Process " << rank << " took " << (end - start) << " seconds.\n";

        // Write all process timings
        for (int i = 1; i < size; i++)
        {
            outfile << "Process " << i << " took " << all_times[i] << " seconds.\n";
        }

        outfile.close();
    }

    else
    {

        MPI_Bcast(&search_term_len, 1, MPI_INT, 0, MPI_COMM_WORLD); // Receive length of search term

        search_term.resize(search_term_len); // Resize search term to receive it from root

        MPI_Bcast(&search_term[0], search_term.size() + 1, MPI_CHAR, 0, MPI_COMM_WORLD); // Receive search term

        // Worker process receives chunk from root
        string recv_text = receive_string(0);
        vector<Contact> contacts = string_to_contacts(recv_text);

        // Perform search
        start = MPI_Wtime();
        string result;
        for (auto &c : contacts)
        {
            string match = check(c, search_term);
            if (!match.empty())
                result += match;
        }
        end = MPI_Wtime();

        double my_time = end - start;

        // Send results back to root
        send_string(result, 0);

        MPI_Gather(&my_time, 1, MPI_DOUBLE, NULL, 0, MPI_DOUBLE, 0, MPI_COMM_WORLD);

        // Report time taken
        // printf("Process %d took %f seconds.\n", rank, end - start);
    }

    MPI_Finalize();

    return 0;
}