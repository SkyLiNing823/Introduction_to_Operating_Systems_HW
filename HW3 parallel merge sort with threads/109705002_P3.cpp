#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <sys/time.h>
#include <queue>
using namespace std;

int N = 1;
long int *A, arrayLen;
sem_t jobStart, jobEnd, mutexLock, queueLock, statusLock;
struct Job
{
    char task;
    int n;
};
queue<Job> jobQueue;
int jobStatus[15];

void BubbleSort(int start, int end)
{
    for (int i = start; i < end - 1; i++)
    {
        for (int j = start; j < end - i - 1 + start; j++)
        {
            if (A[j] > A[j + 1])
                swap(A[j], A[j + 1]);
        }
    }
}

void merge(int start, int end)
{
    int mid = (start + end) / 2;
    int leftIdx = 0, rightIdx = 0, newIdx = start;
    int *left = new int[mid - start + 1];
    int *right = new int[end - mid];
    for (int i = 0; i < mid - start + 1; i++)
        left[i] = A[start + i];
    for (int i = 0; i < end - mid; i++)
        right[i] = A[mid + 1 + i];
    while (leftIdx < (mid - start + 1) && rightIdx < (end - mid))
    {
        if (left[leftIdx] < right[rightIdx])
            A[newIdx++] = left[leftIdx++];
        else
            A[newIdx++] = right[rightIdx++];
    }
    while (leftIdx < (mid - start + 1))
        A[newIdx++] = left[leftIdx++];
    while (rightIdx < (end - mid))
        A[newIdx++] = right[rightIdx++];
}

void QueueInsert(char c, int n)
{
    Job tmp;
    tmp.task = c;
    tmp.n = n;
    jobQueue.push(tmp);
}

void *Dispatch(void *)
{
    for (int i = 1; i <= 8; i++)
    {
        sem_wait(&queueLock);
        QueueInsert('B', i);
        sem_post(&queueLock);
    }
    for (int i = 1; i <= N; i++)
        sem_post(&jobStart);
    while (1)
    {
        sem_wait(&jobEnd);
        sem_wait(&statusLock);
        if (jobStatus[11] == 0 && jobStatus[0] + jobStatus[1] == 2)
        {
            sem_wait(&queueLock);
            QueueInsert('M', 4);
            jobStatus[11] = -1;
            sem_post(&queueLock);
        }
        else if (jobStatus[12] == 0 && jobStatus[2] + jobStatus[3] == 2)
        {
            sem_wait(&queueLock);
            QueueInsert('M', 5);
            jobStatus[12] = -1;
            sem_post(&queueLock);
        }
        else if (jobStatus[13] == 0 && jobStatus[4] + jobStatus[5] == 2)
        {
            sem_wait(&queueLock);
            QueueInsert('M', 6);
            jobStatus[13] = -1;
            sem_post(&queueLock);
        }
        else if (jobStatus[14] == 0 && jobStatus[6] + jobStatus[7] == 2)
        {
            sem_wait(&queueLock);
            QueueInsert('M', 7);
            jobStatus[14] = -1;
            sem_post(&queueLock);
        }
        else if (jobStatus[9] == 0 && jobStatus[11] + jobStatus[12] == 2)
        {
            sem_wait(&queueLock);
            QueueInsert('M', 2);
            jobStatus[9] = -1;
            sem_post(&queueLock);
        }
        else if (jobStatus[10] == 0 && jobStatus[13] + jobStatus[14] == 2)
        {
            sem_wait(&queueLock);
            QueueInsert('M', 3);
            jobStatus[10] = -1;
            sem_post(&queueLock);
        }
        else if (jobStatus[8] == 0 && jobStatus[9] + jobStatus[10] == 2)
        {
            sem_wait(&queueLock);
            QueueInsert('M', 1);
            jobStatus[8] = -1;
            sem_post(&queueLock);
        }
        sem_post(&statusLock);
        sem_post(&jobStart);
        if (jobStatus[8] == 1)
            break;
    }
    pthread_exit(NULL);
}

void *ThreadJob(void *)
{
    while (jobStatus[8] == 0)
    {
        sem_wait(&jobStart);
        if (jobStatus[8] == 1)
            break;
        if (jobQueue.size() == 0)
            continue;
        sem_wait(&queueLock);
        Job job = jobQueue.front();
        jobQueue.pop();
        sem_post(&queueLock);
        int n = job.n;
        int slice = arrayLen / 8;
        if (job.task == 'B')
        {
            if (n < 8)
                BubbleSort(slice * (n - 1), slice * n);
            else
                BubbleSort(slice * 7, arrayLen);
            sem_wait(&statusLock);
            jobStatus[n - 1] = 1;
            sem_post(&statusLock);
        }
        else
        {
            switch (n)
            {
            case 1:
                merge(0, arrayLen - 1);
                break;
            case 2:
                merge(0, slice * 4 - 1);
                break;
            case 3:
                merge(slice * 4, arrayLen - 1);
                break;
            case 4:
                merge(0, slice * 2 - 1);
                break;
            case 5:
                merge(slice * 2, slice * 4 - 1);
                break;
            case 6:
                merge(slice * 4, slice * 6 - 1);
                break;
            default:
                merge(slice * 6, arrayLen - 1);
                break;
            }
            sem_wait(&statusLock);
            jobStatus[n + 7] = 1;
            sem_post(&statusLock);
        }
        sem_post(&jobEnd);
    }
    pthread_exit(NULL);
}

int main()
{
    // start threading
    struct timeval start, end;
    while (N <= 8)
    {
        pthread_t dispatcher, threadPool[N];
        sem_init(&jobStart, 0, 0);
        sem_init(&jobEnd, 0, 0);
        sem_init(&mutexLock, 0, 1);
        sem_init(&queueLock, 0, 1);
        sem_init(&statusLock, 0, 1);
        for (int i = 0; i < 15; i++)
            jobStatus[i] = 0;
        //  read file
        ifstream ifs;
        ifs.open("input.txt");
        ifs >> arrayLen;
        A = new long int[arrayLen];
        for (int i = 0; i < arrayLen; i++)
            ifs >> A[i];
        ifs.close();
        pthread_create(&dispatcher, NULL, Dispatch, NULL);
        for (int i = 0; i < N; i++)
            pthread_create(&threadPool[i], NULL, ThreadJob, NULL);
        gettimeofday(&start, 0);
        pthread_join(dispatcher, NULL);
        gettimeofday(&end, 0);
        int sec = end.tv_sec - start.tv_sec;
        int usec = end.tv_usec - start.tv_usec;
        printf("worker thread #%d, elapsed %f ms\n", N, (sec + (usec / 1000000.0)) * 1000);
        // save file
        ofstream ofs;
        string fileName = "output_" + to_string(N) + ".txt";
        ofs.open(fileName);
        for (int i = 0; i < arrayLen; i++)
        {
            ofs << A[i];
            if (i != arrayLen - 1)
                ofs << " ";
        }
        ofs.close();
        sem_destroy(&jobStart);
        sem_destroy(&jobEnd);
        sem_destroy(&mutexLock);
        sem_destroy(&queueLock);
        sem_destroy(&statusLock);
        N++;
    }
    return 0;
}