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

    while (i<=leftend && j<=rightend) {
        if(A[i]<=A[j]){
            B[k]=A[i];
            i++;
            k++;

        } else {
            B[k] = A[j];
            j++;
            k++;

        }

    }

    while(i<=leftend){
        B[k]=A[i];
        i++;
        k++;
    
    }
    
    while(j<=rightend){
        B[k]=A[j];
        k++;
        j++;
    }

    memcpy(A + leftstart, B + leftstart, (rightend - leftstart + 1) * sizeof(int));

}



/*  this is the normal mergesort function ,nothing special about it,
after cutoff level the threads use this  */

void my_mergesort(int left, int right){
    if (left >= right) {
        return;
    }
    
    int mid = left + (right-left)/2;

    my_mergesort(left, mid);
    my_mergesort(mid + 1, right);

    merge(left, mid, mid + 1, right);

}
/* this function will be called by the testing program. */

/* 

  it sorts the array using threads , and threads call this function when cutoff is not met and it starts a thread for every half ,the struct is passed onto this and we only free the two child structs after the joins 
  */

void * parallel_mergesort(void *arg){
    struct argument *myArg = (struct argument *) arg;

    int left = myArg->left;
    int right = myArg->right;
    int level = myArg->level;

	if (left >= right) {
        return NULL;
    }

    if (level >= cutoff) {
        my_mergesort(left, right);
        return NULL;

    } else {
        int mid = left + (right - left) / 2;
        struct argument *leftArg = buildArgs(left, mid, level + 1);
        struct argument *rightArg = buildArgs(mid + 1, right, level + 1);
		
        if (leftArg == NULL || rightArg == NULL) {
            free(leftArg);
            free(rightArg);
            my_mergesort(left, right);
            return NULL;
        }


        pthread_t t1, t2;
        int created1 = (pthread_create(&t1, NULL, parallel_mergesort, leftArg) == 0);
        int created2 = (pthread_create(&t2, NULL, parallel_mergesort, rightArg) == 0);

        if (!created1) {
            parallel_mergesort(leftArg);
        }

        if (!created2) {
            parallel_mergesort(rightArg);
        }

        if (created1) {
            pthread_join(t1, NULL);
        }

        if (created2) {
            pthread_join(t2, NULL);
        }


		free(leftArg);
        free(rightArg);

        merge(left, mid, mid + 1, right);
        return NULL;

    }
}

/* we build the argument for the parallel_mergesort function. */


struct argument * buildArgs(int left, int right, int level) {
    struct argument *newArg = malloc(sizeof(struct argument));
		
    if (newArg == NULL) {
        return NULL;
    }
    
    newArg->left = left;
    newArg->right = right;
    newArg->level = level;

    return newArg;
}
