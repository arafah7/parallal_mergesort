/**
 * This file implements parallel mergesort.
 */

#include <stdio.h>
#include <string.h> /* for memcpy */
#include <stdlib.h> /* for malloc */
#include "mergesort.h"

/* this function will be called by mergesort() and also by parallel_mergesort(). */

/* this function merges the two sorted parts of array a and it temporarily stores it in arr B  and then we copy the result back 
*/

void merge(int leftstart, int leftend, int rightstart, int rightend){
    int i = leftstart;
    int j = rightstart;
    int k = leftstart;

    while(i<=leftend && j<=rightend){ 
        if(A[i]<=A[j]){ //compare the left side of array A with the right side of array A 
            B[k]=A[i]; // the smaller value is stored in B 
            i++;
            k++;

       } else{
            B[k] = A[j]; // if A[j] is smaller, it gets stored in B 
            j++;
            k++;

        }

    }

    while(i<=leftend){
        B[k]=A[i]; // if the right side elements are done, the left side is copied into array B
        i++;
        k++;
    }
        
    while(j<=rightend){
        B[k]=A[j]; // if the left side elements are done, the right side is copied into array B
        k++;
        j++;
    }

    memcpy(A + leftstart, B + leftstart, (rightend - leftstart + 1) * sizeof(int));
    // since B is a temporary array, the memory of the sorted array is copied back to A

}



/*  this is the normal mergesort function ,nothing special about it,
after cutoff level the threads use this  */

void my_mergesort(int left, int right){
    if (left >= right) { //checks subarray length 
        return;
    }
    
    int mid = left + (right-left)/2; //divide

    my_mergesort(left, mid); //conquers by recursively sorting left half
    my_mergesort(mid + 1, right);  //conquers by recursively sorting right half

    merge(left, mid, mid + 1, right); // joining two halves

}
/* this function will be called by the testing program. */

/* 

  it sorts the array using threads , and threads call this function when cutoff is not met and it starts a thread for every half ,the struct is passed onto this and we only free the two child structs after the joins 
  */

void * parallel_mergesort(void *arg){
    struct argument *myArg = (struct argument *) arg;

    int left = myArg->left; //taken from the argument
    int right = myArg->right;
    int level = myArg->level;

	if (left >= right) {
        return NULL;
    }

    if (level >= cutoff) { //prevents creating more threads than we need
        my_mergesort(left, right);
        return NULL;

    } else { 
        int mid = left + (right - left) / 2; //splits array in half
        struct argument *leftArg = buildArgs(left, mid, level + 1); // initualises range of left to mid, dynamically allocates memory
        struct argument *rightArg = buildArgs(mid + 1, right, level + 1); // initualises range of mid+1 to right dynamically allocates memory
		
        if (leftArg == NULL || rightArg == NULL) { //checks if both allocations failed
            free(leftArg); // free allocated heap memory
            free(rightArg); // free allocated heap memory
            my_mergesort(left, right); //if parallel mergesort fails it goes to single thread / sequential mergesort
            return NULL;
        }


        pthread_t t1, t2;
        int created1 = (pthread_create(&t1, NULL, parallel_mergesort, leftArg) == 0); // create thread t1 for left half
        int created2 = (pthread_create(&t2, NULL, parallel_mergesort, rightArg) == 0); // create thread t2 for right half

        //  parallel_mergesort is run when operating system cannot allocate threads
        if (!created1) {
            parallel_mergesort(leftArg);
        }

        if (!created2) {
            parallel_mergesort(rightArg);
        }

        // pthread_join ensures both halves are sorted before merging
        if (created1) {
            pthread_join(t1, NULL);
        }

        if (created2) {
            pthread_join(t2, NULL);
        }

        //frees heap allocated argument struct, to prevent memory leaks
		free(leftArg);
        free(rightArg);

        //combine the two sorted parts
        merge(left, mid, mid + 1, right);
        return NULL;

    }
}

/* we build the argument for the parallel_mergesort function. */


struct argument * buildArgs(int left, int right, int level) {
    struct argument *newArg = malloc(sizeof(struct argument)); // allocates struct on heap
		
    if (newArg == NULL) {
        return NULL; // if dynamic memory fails
    }
    
    newArg->left = left; //populates left side
    newArg->right = right; // populates right side
    newArg->level = level; // recursion level

    return newArg;
}
