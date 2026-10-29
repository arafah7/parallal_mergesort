# README

## Assignment 3 - parallel mergesort

* Authors: Golam Mohammad Arafah, Adelle Ocampo, Vivian Chan
* Group 40

## Overview

This program sorts integers of a randomly generated array. It uses a mergesort algorithm which has logmarithmic time complexity. Now instead of one thread, doing all the work, the program splits the task into multiple threads. We make new threads untill a cutoff has been met.Threads at cutoff sort themselves with oridinary mergesort.The goal is sort the array faster using multithreading and examine how fast the sorting gets when we use more threads


## Manifest

* `mergesort.c` - our implementation of `merge`, `my_mergesort`,
  `parallel_mergesort` and `buildArgs`.
* `mergesort.h` - provided header, unmodified. Declares the four functions,
  `struct argument`, and the shared globals `A`, `B` and `cutoff`.
* `test-mergesort.c` - provided test harness. Builds the random array, times
  the sort and checks the result.
* `Makefile` - provided build rules.
* `README.md` - this file.

## Building the project

Open a terminal in the comp2002-os-mergesort folder and run make clean && make to compile the code. This compiles mergesort.c and test-mergesort.c using GCC and links them into an executable called test-mergesort, which you can run directly from the folder without installing anything. You just need GCC, Make, and the pthread library, which are commonly available on Linux. Make sure to run make clean before make, as the provided Makefile uses $? to determine which files to link. If you only edit mergesort.c and run make without cleaning first, the build may fail with an undefined reference error because test-mergesort.o is missing from the link command. Running make clean by itself simply removes the existing build files.




## Features and usage


the program can be run using ./test-mergesort <input size> <cutoff level> <seed>. For example, ./test-mergesort 100000000 4 1234 sorts 100 million integers. The input size must be at least 2. The cutoff level determines how many threads are used. A cutoff of 0 runs a normal single threaded mergesort, while a cutoff of c creates up to 2^c leaf threads to sort different parts of the array before merging the results. The seed controls the randomly generated input, so using the same seed will produce the same array each time.
If the sorting succeeds, the program prints how long it took to sort the elements. If the array is not sorted correctly, it prints sorting failed!!!! and exits with status 1.The implementation has two main functions. my_mergesort handles normal sequential sorting, while parallel_mergesort divides the work between threads using pthread_create and waits for them to finish using pthread_join. No locks are needed because each thread works on a separate part of the array, and the parent only merges the results after both child threads have finished. Memory is also managed carefully, with each argument structure freed exactly once by the thread that allocated it. If memory allocation or thread creation fails, the program falls back to sequential sorting instead of crashing. Any cutoff value is accepted, even if it is larger than log2(n), because the program stops splitting ranges when they contain fewer than two elements.




## Testing


We tested the final version of the code in total three ways ,does it sort correctly,is it safe with memory and threads, and is it fast enough with multithreading

For check that the array gets sorted properly and ensuring the correctness, we wrote a small shell loop that ran the program on many different inputs: array sizes from 2 up to 1,000,000 (including odd sizes like 3, 7 and 12345 so the two halves are not equal), cutoff levels from 0 up to 8, and three different seeds. That came to 231 runs, and every one of them printed the sorted message. 

Since our code does not use any locks, we wanted to be sure that two threads never write to the same part of the array. We asked Claude how to check this, and it suggested two tools that come with gcc: AddressSanitizer, which catches memory mistakes such as writing past the end of an array or freeing something twice, and ThreadSanitizer, which catches two threads touching the same data at the same time. We compiled the program with each tool and ran it on the small tricky inputs and on larger ones with up to 64 threads. And then there were no errors 

For checking the speed, we have  sorted 100,000,000 numbers with seed 1234 on my 12-core laptop. The single-threaded version took 25.3 seconds.But  With 4 threads it took 12.3 seconds,with 8 it took 9.6, with 16 it took 8.6, and with 64 it took 7.2.So the program runs more than twice as fast from cutoff 2 onward. With only 2 threads it took 16.7 seconds, which is faster but not twice as fast, because the final merge of the whole array still has to be done by one thread. After about 16 threads the gains get small, because the laptop only has 12 cores, so extra threads just take turns, and merging is limited more by how fast memory can be read than by how many cores there are. The times are from a single run each, so small differences such as cutoff 5 being a bit slower than cutoff 4 are just normal variation.








.

## Known Bugs
We ran into three bugs while building this,

The first was a double free. Our `parallel_mergesort` freed the struct it was given just before returning, but main  also frees that same pointer after the sort, so every single run aborted with "free(): double free detected". It took a while to see because the sort itself was correct; the crash came after. The fix was to decide clearly who owns each piece of memory: whoever calls `buildArgs` frees the result. So `main` frees the top-level struct, and each parent frees only the two child structs it created, after joining them.

The second was writing past the end of the array. It only happened with a small array and a large cutoff, for example 2 elements with cutoff 3. We had no check for an empty range, so a range like [2, 1] was split again and merged, and the merge read an element that did not exist. Sometimes this corrupted memory and crashed later, sometimes it just produced an unsorted array. Adding a check at the top of `parallel_mergesort` that returns immediately when the range has one element or none fixed it.

The third came while we were adding checks on the return value of `pthread_create`. We wrote new joins that only run for threads that were actually created, but left the old joins underneath, so every thread was joined twice. Joining a thread twice is undefined behaviour, and it caused segfaults and hangs, but only at cutoff 3 and above, because that is when thread IDs start getting reused quickly. Deleting the duplicate joins fixed it, and we learned to test at several cutoffs rather than just one.



## Reflection and Self Assessment

We learned a lot about operating systems while doing this assignment.We first started by looking at the address space of the process and threads and we built a mental model that helped us to visualise the whole structure and built really deep intuition. we listed down them ,what lies in each address space. We built the following mental map. Text holds the compiled code of main, merge, my_mergesort, parallel_mergesort and buildArgs. Data holds the three globals: cutoff, and the pointers A and B. Heap holds the two arrays of n ints that A and B point to, the root struct argument, and every child struct argument. Stack holds every local variable, on the stack of whichever thread is running: main's n,seed, start_time sorting_time and arg; parallel_mergesort's myArg, left, right, level, mid, leftArg, rightArg, t1, t2, created1 and created2; my_mergesort's left, right and mid; merge's i, j and k; and buildArgs's newArg. We also had a long discussion on why locks are not needed. We figured out that each thread is independent with no sharing resources, hence locks are not needed, we just needed to synchronise the threads so that the child threads finish first before their parents. Looking at the source code of linux on how task struct is implemented helped build a good intuition on this as well. 

In terms of parallel performance, we noticed that as cutoff increased, the time sorting the same input decreased intially. Though as the cutoff increased further, after a certian point, the time began to increase again, this could be due to memory bandwidth, limited CPU cores, thread creation or schduling. Furthermore, in terms of communication of the code, as a group we choose to have the child write their results into a shared arrays, so theres no need to send copies between thread, which cuts down the communication overhead. pthread_join() sychronises as it waits for the children to finish before the parent thread merges the sorted sections, this sychronisation involves overhead and may require the parent to wait which can limit paralisation, but ensures the results are ready before parents process them, which ties in with data integrity. Both sorting and independent merging can happen in parallel across different sections of the array. However, each of the individual merges are serial, and the final longest merge of all element is performed by one thread, this limits the overall speedup. Though we did attempt to optimise where we could, by using a cutoff and cutting down of communication overhead.

## Sources Used

We used the course textbook, Operating Systems: Three Easy Pieces, chapters 26 and 27, for the background on threads and the pthread API. We also read the Linux manual pages for pthread_create, pthread_join, malloc and memcpy to check what each function expects and returns.

We also looked at the linux source code on github and especially the task_struct struct helped us build a nice intuition and good mental model on how threads are created.

We asked Claude  to  help us understand the double free erro and it also  pointed us to the missing check for empty ranges, Claude also  suggested  us AddressSanitizer and ThreadSanitizer as a way to check for memory errors and data races since our code does not use locks and we found it very useful and we didnt knew something like threadsanitiser existed.

