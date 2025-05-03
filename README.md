# CSE4112_Parallel_Processing_and_Distributed_System_Lab



## 1. Matrix Multiplication

Write a program to multiply **K** different matrices **A** of dimension **M×N** with matrices **B** of dimension **N×P**.

- Constraints:  
  `K*M*N <= 10^6`,  
  `K*N*P <= 10^6`,  
  `K*M*P <= 10^6`

### Tasks:
- (a) Using **MPI**
- (b) Using **CUDA**

**Input:**  
`K, M, N, P`

**Output:**  
Time taken for multiplication

---

## 2. Word Frequency Count

Write a program to count the words in a file and sort them in **descending order of frequency** (highest occurring word first).

### Tasks:
- (a) Using **MPI**
- (b) Using **CUDA**

**Input:**  
Number of processes, (Text input from file)

**Output:**  
Total time, top 10 word occurrences

---

## 3. Phonebook Search

A phonebook is given as a file. Write a program to **search for all contacts matching a name**.

### Tasks:
- (a) Using **MPI**
- (b) Using **CUDA**

**Input:**  
Number of processes, (Phonebook from file)

**Output:**  
Total time, Matching names and contact numbers

---

## 4. Pattern Search in Paragraph

Given a paragraph and a pattern like `%x%`, write a program to find the **number of occurrences** of the given pattern inside the text.

### Tasks:
- (a) Using **MPI**
- (b) Using **CUDA**

**Input:**  
Number of processes, (Paragraph from file)

**Output:**  
Total time, Number of pattern occurrences
