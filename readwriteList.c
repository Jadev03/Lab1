#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

#define MAX_VALUE 65536  // 2^16

typedef struct Node {
    int data;
    struct Node* next;
} Node;

Node* head = NULL;
pthread_rwlock_t list_rwlock; // read-write lock

pthread_mutex_t rand_mutex = PTHREAD_MUTEX_INITIALIZER;  // this mutext for  If multiple threads call it at the same time, they race on that state → results are not thread-safe.That means two threads might interfere with each other’s random number sequence → you can get repeated values, skipped values, or even undefined behavior depending on the implementation.

int thread_rand() {
    pthread_mutex_lock(&rand_mutex);
    int val = rand();
    pthread_mutex_unlock(&rand_mutex);
    return val;
}

// Thread argument struct
typedef struct ThreadArg {
    int m_ops;
    double mMember, mInsert, mDelete;
} ThreadArg;

// Linked list operations (protected by rwlock)
int Member(int value) {
    int found = 0;
    pthread_rwlock_rdlock(&list_rwlock); // shared/read lock
    Node* curr = head;
    while (curr != NULL && curr->data < value) curr = curr->next;
    found = (curr != NULL && curr->data == value);
    pthread_rwlock_unlock(&list_rwlock);
    return found;
}

int Insert(int value) {
    int inserted = 0;
    pthread_rwlock_wrlock(&list_rwlock); // exclusive/write lock
    Node* curr = head;
    Node* pred = NULL;
    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }
    if (curr == NULL || curr->data > value) {
        Node* temp = malloc(sizeof(Node));
        if (!temp) { // malloc fail guard
            pthread_rwlock_unlock(&list_rwlock);
            return 0;
        }
        temp->data = value;
        temp->next = curr;
        if (pred == NULL) head = temp;
        else pred->next = temp;
        inserted = 1;
    }
    pthread_rwlock_unlock(&list_rwlock);
    return inserted;
}

int Delete(int value) {
    int deleted = 0;
    pthread_rwlock_wrlock(&list_rwlock); // exclusive/write lock
    Node* curr = head;
    Node* pred = NULL;
    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }
    if (curr != NULL && curr->data == value) {
        if (pred == NULL) head = curr->next;
        else pred->next = curr->next;
        free(curr);
        deleted = 1;
    }
    pthread_rwlock_unlock(&list_rwlock);
    return deleted;
}

// Free the list
void FreeList() {
    pthread_rwlock_wrlock(&list_rwlock);
    Node* curr = head;
    while (curr != NULL) {
        Node* temp = curr;
        curr = curr->next;
        free(temp);
    }
    head = NULL;
    pthread_rwlock_unlock(&list_rwlock);
}

// Populate list with unique random numbers
void populateList(int n) {
    int count = 0;
    // Use local seed so that calls here are deterministic enough
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();
    while (count < n) {
        int val = thread_rand() % MAX_VALUE;
        if (Insert(val)) count++;
    }
}

// Operations performed by each thread
void* threadFunc(void* arg) {
    ThreadArg* tArg = (ThreadArg*)arg;
    int m = tArg->m_ops;
    double mMember = tArg->mMember;
    double mInsert = tArg->mInsert;
    // double mDelete = tArg->mDelete; // not needed separately

    // thread-local seed for rand_r
    unsigned int seed = (unsigned int)time(NULL) ^ (unsigned int)(uintptr_t)pthread_self();

    for (int i = 0; i < m; i++) {
        double prob = (double)thread_rand() / RAND_MAX;
        int val = thread_rand() % MAX_VALUE;

        if (prob < mMember) {
            Member(val);
        } else if (prob < mMember + mInsert) {
            Insert(val);
        } else {
            Delete(val);
        }
    }
    return NULL;
}

// Run parallel experiment
void runExperimentParallel(int n, int m, double mMember, double mInsert, double mDelete, int runcount, int numThreads) {
    double* results = malloc(runcount * sizeof(double));
    if (!results) {
        fprintf(stderr, "Memory alloc failed\n");
        return;
    }

    for (int r = 0; r < runcount; r++) {
        FreeList();
        populateList(n);

        pthread_t threads[numThreads];
        ThreadArg tArgs[numThreads];

        // distribute operations among threads (balance remainder)
        int base = m / numThreads;
        int rem = m % numThreads;
        for (int i = 0; i < numThreads; i++) {
            tArgs[i].m_ops = base + (i < rem ? 1 : 0);
            tArgs[i].mMember = mMember;
            tArgs[i].mInsert = mInsert;
            tArgs[i].mDelete = mDelete;
        }

        clock_t start = clock();

        for (int i = 0; i < numThreads; i++) {
            if (pthread_create(&threads[i], NULL, threadFunc, &tArgs[i]) != 0) {
                perror("pthread_create");
                // join already created threads
                for (int j = 0; j < i; j++) pthread_join(threads[j], NULL);
                free(results);
                return;
            }
        }

        for (int i = 0; i < numThreads; i++)
            pthread_join(threads[i], NULL);

        clock_t end = clock();
        results[r] = ((double)(end - start)) / CLOCKS_PER_SEC;
    }

    printf("\nTime(seconds)\n");
    for (int r = 0; r < runcount; r++) printf("%.6f\n", results[r]);

    free(results);
    FreeList();
}

int main() {
    srand((unsigned int)time(NULL));

    if (pthread_rwlock_init(&list_rwlock, NULL) != 0) {
        fprintf(stderr, "Failed to init rwlock\n");
        return 1;
    }

    int n, m, runcount, numThreads;
    double mMember, mInsert, mDelete;

    // Initial inputs
    printf("Enter n (initial number of unique nodes): ");
    if (scanf("%d", &n) != 1) return 1;
    printf("Enter m (number of operations): ");
    if (scanf("%d", &m) != 1) return 1;
    printf("Enter fraction of Member operations (0-1): ");
    if (scanf("%lf", &mMember) != 1) return 1;
    printf("Enter fraction of Insert operations (0-1): ");
    if (scanf("%lf", &mInsert) != 1) return 1;
    printf("Enter fraction of Delete operations (0-1): ");
    if (scanf("%lf", &mDelete) != 1) return 1;

    if ((mMember + mInsert + mDelete) > 1.000001 || (mMember + mInsert + mDelete) < 0.999999) {
        printf("Error: Fractions must sum to 1.0\n");
        pthread_rwlock_destroy(&list_rwlock);
        return 1;
    }

    while (1) {
        printf("Enter number of threads: ");
        if (scanf("%d", &numThreads) != 1) break;
        if (numThreads <= 0) {
            printf("Number of threads must be > 0\n");
            continue;
        }
        printf("Enter runcount (number of times to repeat experiment):");
        if (scanf("%d", &runcount) != 1) break;

        runExperimentParallel(n, m, mMember, mInsert, mDelete, runcount, numThreads);

        int choice;
        printf("\nDo you want to:\n");
        printf("1. Quit\n");
        printf("2. Change thread count and runcount and rerun\n");
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) break;

        if (choice == 1) {
            printf("Exiting program.\n");
            break;
        } else if (choice == 2) {
            continue; // loop again with new thread count and runcount
        } else {
            printf("Invalid choice, exiting.\n");
            break;
        }
    }

    pthread_rwlock_destroy(&list_rwlock);
    return 0;
}
