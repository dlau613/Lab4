# UCLA CS 111 Winter 2016
## Lab 4 - Synchronization

###Part 1 – parallel updates to a shared variable
State of Deliverables
Source for a the source for a C program and Makefile that cleanly (no warnings) builds using gcc on an x86-64 GNU/Linux system, implements the (below) specified command line options, drives one or more parallel threads that do adds and subtracts to a shared variable, and reports on the final sum and performance. **source of C program is lab4.c, Makefile is Makefile. Makefile currently does not build cleanly**

graphs of: 
the average time per operation vs the number of iterations
the average time per operation vs the number of threads for all four versions of the add function.
**graphs currently not made. use gnuplot**

written brief (a few sentences per question) answers to questions 1.1, 1.2 and 1.3.
**questions listed below, please fill in**

QUESTIONS 1.1:
Why does it take this many threads or iterations?
Why does a significantly smaller number of iterations so seldom fail?

QUESTIONS 1.2
Why does the average cost per operation drop with increasing iterations?
How do we know what the “correct” cost is?
Why are the --yield runs so much slower? Where is the extra time going?
Can we get valid timings if we are using --yield? How, or why not?

QUESTIONS 1.3
Why do all of the options perform similarly for low numbers of threads?
Why do the three protected operations slow down as the number of threads rises?
Why are spin-locks so expensive for large numbers of threads?

###Part 2 – parallel updates to complex data structures
State of Deliverables
the source for a C module and Makefile that cleanly (no warnings) builds using gcc on an x86-64 GNU/Linux system, and implements insert, delete, lookup, and length methods for a sorted doubly linked list (described in a provided header file, including correct placement of pthread_yield calls).
the source for a C program and Makefile that cleanly (no warnings) builds using gcc on an x86-64 GNU/Linux system, implements the (below) specified command line options, drives one or more parallel threads that do operations on a shared linked list, and reports on the final list and performance.
**currently in sltest.c and sltest2.c, Makefile not implemented. future work: implement in already created Makefile**

graphs of:
average time per unprotected operation vs number of iteration (single thread)
(corrected) average time per operation (for none, mutex, and spin-lock) vs number of threads.
(corrected) average time per operation (for none, mutex, and spin-lock) vs the ratio of threads per list.
**graphs currently not made. use gnuplot**

written brief (a few sentences per question) answers to questions 2.1, 2.2 and 2.3.
**questions listed below, please fill in**

QUESTIONS 2.1
Explain the variation in time per operation vs the number of iterations? How would you propose to correct for this effect?

QUESTIONS 2.2
Compare the variation in time per protected operation vs the number of threads in Part 2 and in Part 1. Explain the difference.

QUESTIONS 2.3
Explain the the change in performance of the synchronized methods as a function of the number of threads per list.
Explain why threads per list is a more interesting number than threads (for this particular measurement).

###Part 3 – sleep/wakeup races
State of Deliverables
brief (a few sentences per question) answers to questions 3-1
**questions listed below, please fill in**

QUESTIONS 3-1
Why must the mutex be held when pthread_cond_wait is called?
Why must the mutex be released when the waiting thread is blocked?
Why must the mutex be reacquired when the calling thread resumes?
Why must this be done inside of pthread_cond_wait? Why can’t the caller simply release the mutex before calling pthread_cond_wait?
Can this be done in a user-mode implementation of pthread_cond_wait? If so, how? If it can only be implemented by a system call, explain why?
