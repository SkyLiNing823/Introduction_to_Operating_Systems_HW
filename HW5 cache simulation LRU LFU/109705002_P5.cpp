#include <iostream>
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <iomanip>
#include <sys/time.h>
using namespace std;

int frames[4] = {64, 128, 256, 512};

// Hash Table
struct hashNode
{
    int name;
    int count;
    int idx;
    int seq;
};
class Hash
{
    int bucket;
    list<hashNode *> *table;

public:
    Hash();
    void insertItem(hashNode *node);
    void deleteItem(int key);
    int hashFunction(int key)
    {
        return (key % bucket);
    }
    int search(int key);
};
Hash::Hash()
{
    bucket = 10000019; // large odd
    table = new list<hashNode *>[bucket];
}
void Hash::insertItem(hashNode *node)
{
    int idx = hashFunction(node->name);
    table[idx].push_front(node);
}
void Hash::deleteItem(int key)
{
    int idx = hashFunction(key);
    list<hashNode *>::iterator i;
    for (i = table[idx].begin(); i != table[idx].end(); i++)
        if ((*i)->name == key)
            break;
    if (i != table[idx].end())
        table[idx].erase(i);
}
int Hash::search(int key)
{
    int idx = hashFunction(key);
    list<hashNode *>::iterator i;
    for (i = table[idx].begin(); i != table[idx].end(); i++)
        if ((*i)->name == key)
            return (*i)->idx;

    return -1;
}

class LFU
{
public:
    int bucket;
    int count;
    vector<hashNode *> heap;
    Hash hashTable;
    LFU(int n);
    int check(int key, int seq);
    void heapifyUp(unsigned int idx);
    void heapifyDown(unsigned int idx);
};

LFU::LFU(int n)
{
    bucket = n;
    count = 0;
}

int LFU::check(int key, int seq)
{
    int idx = hashTable.search(key);
    if (idx == -1)
    {
        if (count < bucket)
        {
            hashNode *newNode = new hashNode;
            newNode->name = key;
            newNode->count = 1;
            newNode->idx = count;
            newNode->seq = seq;
            heap.push_back(newNode);
            heapifyUp(count);
            hashTable.insertItem(newNode);
            count += 1;
        }
        else // count == bucket
        {
            // delete node
            hashTable.deleteItem(heap[0]->name);
            heap[0] = heap.back();
            heap.pop_back();
            heapifyDown(0);
            hashNode *newNode = new hashNode;
            newNode->name = key;
            newNode->count = 1;
            newNode->idx = count - 1;
            newNode->seq = seq;
            heap.push_back(newNode);
            heapifyUp(count - 1);
            hashTable.insertItem(newNode);
        }
        return 0;
    }
    else
    {
        heap[idx]->count += 1;
        heap[idx]->seq = seq;
        heapifyDown(idx);
        return 1;
    }
}

void LFU::heapifyUp(unsigned int idx)
{
    while (idx > 0 && (heap[(idx - 1) / 2]->count > heap[idx]->count ||
                       (heap[(idx - 1) / 2]->count == heap[idx]->count && heap[(idx - 1) / 2]->seq > heap[idx]->seq)))
    {
        // swap
        swap(heap[(idx - 1) / 2], heap[idx]);
        heap[idx]->idx = idx;
        heap[(idx - 1) / 2]->idx = (idx - 1) / 2;
        idx = (idx - 1) / 2;
    }
}

void LFU::heapifyDown(unsigned int idx)
{
    unsigned int left;
    unsigned int right;
    unsigned int smallest;
    while (true)
    {
        left = 2 * idx + 1;
        right = 2 * idx + 2;
        smallest = idx;
        if (left < heap.size() &&
            (heap[left]->count < heap[idx]->count ||
             (heap[left]->count == heap[idx]->count && heap[left]->seq < heap[idx]->seq)))
        {
            smallest = left;
        }

        if (right < heap.size() &&
            (heap[right]->count < heap[smallest]->count ||
             (heap[right]->count == heap[smallest]->count && heap[right]->seq < heap[smallest]->seq)))
        {
            smallest = right;
        }
        if (smallest != idx)
        {
            // swap
            swap(heap[smallest], heap[idx]);
            heap[smallest]->idx = smallest;
            heap[idx]->idx = idx;
            idx = smallest;
        }
        else
            break;
    }
}

class LRU
{
public:
    int bucket;
    int count;
    vector<hashNode *> linkedlist;
    Hash hashTable;
    LRU(int n);
    int check(int key);
    void idxFix();
};

LRU::LRU(int n)
{
    bucket = n;
    count = 0;
}

int LRU::check(int key)
{
    int idx = hashTable.search(key);
    if (idx == -1)
    {
        if (count < bucket)
        {
            hashNode *newNode = new hashNode;
            newNode->name = key;
            newNode->idx = 1;
            linkedlist.push_back(newNode);
            hashTable.insertItem(newNode);
            count += 1;
        }
        else // count == bucket
        {
            // delete node
            hashTable.deleteItem((*linkedlist.begin())->name);
            linkedlist.erase(linkedlist.begin());
            hashNode *newNode = new hashNode;
            newNode->name = key;
            newNode->idx = 1;
            linkedlist.push_back(newNode);
            hashTable.insertItem(newNode);
        }
        return 0;
    }
    else
    {
        int i;
        for (i = 0; linkedlist[i]->name != key; i++)
            ;
        hashNode *tmp = linkedlist[i];
        linkedlist.erase(linkedlist.begin() + i);
        linkedlist.push_back(tmp);
        return 1;
    }
}

int main(int argc, char **argv)
{
    struct timeval start, end;
    int frame, hit, miss, check;
    int seq;
    (void)argc;
    // LFU's part
    cout << "LFU policy:" << endl;
    cout << left << setw(10) << "Frame" << left << setw(10) << "Hit" << left << setw(10) << "Miss" << left << setw(20) << "page fault ratio" << endl;
    for (int i = 0; i < 4; i++)
    {
        frame = frames[i];
        LFU lfu(frame);
        hit = 0, miss = 0, seq = 0;
        gettimeofday(&start, 0);
        ifstream inputFile(argv[1]);
        string line;
        while (getline(inputFile, line))
        {
            check = lfu.check(stoll(line), seq);
            if (check)
                hit++;
            else
                miss++;
            seq += 1;
        }
        inputFile.close();
        double PFratio = (double)miss / (hit + miss);
        cout << left << setw(10) << frame << left << setw(10) << hit << left << setw(10) << miss << left << setw(20) << fixed << setprecision(10) << PFratio << endl;
    }
    gettimeofday(&end, 0);
    int sec = end.tv_sec - start.tv_sec;
    int usec = end.tv_usec - start.tv_usec;
    float elapsed_time = sec + (usec / 1000000.0);
    cout << "Total elapsed time " << elapsed_time << " sec" << endl
         << endl;

    // LRU's part
    cout << "LRU's policy:" << endl;
    cout << left << setw(10) << "Frame" << left << setw(10) << "Hit" << left << setw(10) << "Miss" << left << setw(20) << "page fault ratio" << endl;
    for (int i = 0; i < 4; i++)
    {
        frame = frames[i];
        LRU lru(frame);
        hit = 0, miss = 0, seq = 0;
        gettimeofday(&start, 0);
        ifstream inputFile(argv[1]);
        string line;
        while (getline(inputFile, line))
        {
            check = lru.check(stoll(line));
            if (check)
                hit++;
            else
                miss++;
            seq += 1;
        }
        inputFile.close();
        double PFratio = (double)miss / (hit + miss);
        cout << left << setw(10) << frame << left << setw(10) << hit << left << setw(10) << miss << left << setw(20) << fixed << setprecision(10) << PFratio << endl;
    }
    gettimeofday(&end, 0);
    sec = end.tv_sec - start.tv_sec;
    usec = end.tv_usec - start.tv_usec;
    elapsed_time = sec + (usec / 1000000.0);
    cout << "Total elapsed time " << elapsed_time << " sec" << endl;
    return 0;
}