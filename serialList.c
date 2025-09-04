#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VALUE 65536  // 2^16

// Node structure
typedef struct Node {
    int data;
    struct Node* next;
} Node;

Node* head = NULL;

// Function prototypes
int Member(int value);
int Insert(int value);
int Delete(int value);
void FreeList();
void runExperiment(int n, int m, double mMember, double mInsert, double mDelete, int runcount);

// Linked list operations
int Member(int value) {
    Node* curr = head;
    while (curr != NULL && curr->data < value) {
        curr = curr->next;
    }
    if (curr == NULL || curr->data > value)
        return 0; // not found
    else
        return 1; // found
}

int Insert(int value) {
    Node* curr = head;
    Node* pred = NULL;
    Node* temp;

    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }

    if (curr == NULL || curr->data > value) {
        temp = malloc(sizeof(Node));
        temp->data = value;
        temp->next = curr;
        if (pred == NULL) { // insert at head
            head = temp;
        } else {
            pred->next = temp;
        }
        return 1; // success
    } else {
        return 0; // already in list
    }
}

int Delete(int value) {
    Node* curr = head;
    Node* pred = NULL;

    while (curr != NULL && curr->data < value) {
        pred = curr;
        curr = curr->next;
    }

    if (curr != NULL && curr->data == value) {
        if (pred == NULL) {
            head = curr->next;
        } else {
            pred->next = curr->next;
        }
        free(curr);
        return 1; // success
    } else {
        return 0; // not found
    }
}

// Free the linked list
void FreeList() {
    Node* curr = head;
    while (curr != NULL) {
        Node* temp = curr;
        curr = curr->next;
        free(temp);
    }
    head = NULL;
}

// Utility to generate unique random numbers
void populateList(int n) {
    int count = 0;
    while (count < n) {
        int val = rand() % MAX_VALUE;
        if (Insert(val)) {
            count++;
        }
    }
}

void performOperations(int m, double mMember, double mInsert, double mDelete) {
    for (int i = 0; i < m; i++) {
        double prob = (double)rand() / RAND_MAX;
        int val = rand() % MAX_VALUE;

        if (prob < mMember) {
            Member(val);
        } else if (prob < mMember + mInsert) {
            Insert(val);
        } else {
            Delete(val);
        }
    }
}

// Run experiment with given runcount
void runExperiment(int n, int m, double mMember, double mInsert, double mDelete, int runcount) {
    double* results = malloc(runcount * sizeof(double));
    if (results == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }

    for (int r = 0; r < runcount; r++) {
        FreeList();
        populateList(n);

        clock_t start = clock();
        performOperations(m, mMember, mInsert, mDelete);
        clock_t end = clock();

        results[r] = ((double)(end - start)) / CLOCKS_PER_SEC;
    }

    // Print results in CSV format (easy for Excel)
    printf("\nRun,Time(seconds)\n");  // header
    for (int r = 0; r < runcount; r++) {
        printf("%.6f\n", results[r]);
    }

    free(results);
    FreeList();
}


int main() {
    srand(time(NULL));

    int n, m, runcount;
    double mMember, mInsert, mDelete;

    // Get inputs from terminal
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

    // Validate fractions
    if ((mMember + mInsert + mDelete) > 1.000001 || (mMember + mInsert + mDelete) < 0.999999) {
        printf("Error: Fractions must sum to 1.0\n");
        return 1;
    }

    while (1) {
        printf("Enter runcount (number of times to repeat experiment): ");
        scanf("%d", &runcount);

        runExperiment(n, m, mMember, mInsert, mDelete, runcount);

        int choice;
        printf("\nDo you want to:\n");
        printf("1. Quit\n");
        printf("2. Increase/change runcount and rerun\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            printf("Exiting program.\n");
            break;
        } else if (choice == 2) {
            continue; // loop again with new runcount
        } else {
            printf("Invalid choice, exiting.\n");
            break;
        }
    }

    return 0;
}
