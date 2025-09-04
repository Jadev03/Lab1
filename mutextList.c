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
pthread_mutex_t list_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thread argument struct
typedef struct ThreadArg {
    int m_ops;
    double mMember, mInsert, mDelete;
} ThreadArg;

// Linked list operations (protected by mutex)
int Member(int value) {
    pthread_mutex_lock(&list_mutex);
    Node* curr = head;
    while (curr != NULL && curr->data < value) curr = curr->next;
    int found = (curr != NULL && curr->data == value);
    pthread_mutex_unlock(&list_mutex);
    return found;
}

int Insert(int value) {
    pthread_mutex_lock(&list_mutex);
    Node* curr = head;
    Node* pred = NULL;
    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }
    if (curr == NULL || curr->data > value) {
        Node* temp = malloc(sizeof(Node));
        temp->data = value;
        temp->next = curr;
        if (pred == NULL) head = temp;
        else pred->next = temp;
        pthread_mutex_unlock(&list_mutex);
        return 1;
    } else {
        pthread_mutex_unlock(&list_mutex);
        return 0;
    }
}

int Delete(int value) {
    pthread_mutex_lock(&list_mutex);
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
        pthread_mutex_unlock(&list_mutex);
        return 1;
    } else {
        pthread_mutex_unlock(&list_mutex);
        return 0;
    }
}

// Free the list
void FreeList() {
    pthread_mutex_lock(&list_mutex);
    Node* curr = head;
    while (curr != NULL) {
        Node* temp = curr;
        curr = curr->next;
        free(temp);
    }
    head = NULL;
    pthread_mutex_unlock(&list_mutex);
}

// Populate list with unique random numbers
void populateList(int n) {
    int count = 0;
    while (count < n) {
        int val = rand() % MAX_VALUE;
        if (Insert(val)) count++;
    }
}

// Operations performed by each thread
void* threadFunc(void* arg) {
    ThreadArg* tArg = (ThreadArg*)arg;
    int m = tArg->m_ops;
    double mMember = tArg->mMember;
    double mInsert = tArg->mInsert;
    double mDelete = tArg->mDelete;

    for (int i = 0; i < m; i++) {
        double prob = (double)rand() / RAND_MAX;
        int val = rand() % MAX_VALUE;

        if (prob < mMember) Member(val);
        else if (prob < mMember + mInsert) Insert(val);
        else Delete(val);
    }
    return NULL;
}

// Run parallel experiment
void runExperimentParallel(int n, int m, double mMember, double mInsert, double mDelete, int runcount, int numThreads) {
    double* results = malloc(runcount * sizeof(double));

    for (int r = 0; r < runcount; r++) {
        FreeList();
        populateList(n);

        pthread_t threads[numThreads];
        ThreadArg tArg;
        tArg.m_ops = m / numThreads;  // divide operations
        tArg.mMember = mMember;
        tArg.mInsert = mInsert;
        tArg.mDelete = mDelete;

        clock_t start = clock();

        for (int i = 0; i < numThreads; i++)
            pthread_create(&threads[i], NULL, threadFunc, &tArg);

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
    srand(time(NULL));

    int n, m, runcount, numThreads;
    double mMember, mInsert, mDelete;

    // Initial inputs
    printf("Enter n (initial number of unique nodes): ");
    scanf("%d", &n);
    printf("Enter m (number of operations): ");
    scanf("%d", &m);
    printf("Enter fraction of Member operations (0-1): ");
    scanf("%lf", &mMember);
    printf("Enter fraction of Insert operations (0-1): ");
    scanf("%lf", &mInsert);
    printf("Enter fraction of Delete operations (0-1): ");
    scanf("%lf", &mDelete);

    if ((mMember + mInsert + mDelete) > 1.000001 || (mMember + mInsert + mDelete) < 0.999999) {
        printf("Error: Fractions must sum to 1.0\n");
        return 1;
    }

    while (1) {
        printf("Enter number of threads: ");
        scanf("%d", &numThreads);
        printf("Enter runcount (number of times to repeat experiment):");
        scanf("%d", &runcount);

        runExperimentParallel(n, m, mMember, mInsert, mDelete, runcount, numThreads);

        int choice;
        printf("\nDo you want to:\n");
        printf("1. Quit\n");
        printf("2. Change thread count and runcount and rerun\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

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

    pthread_mutex_destroy(&list_mutex);
    return 0;
}
