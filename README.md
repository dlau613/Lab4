# UCLA CS 111 Winter 2016
## Lab 4 - Synchronization

###Part 1 – parallel updates to a shared variable
State of Deliverables
Source for a the source for a C program and Makefile that cleanly (no warnings) builds using gcc on an x86-64 GNU/Linux system, implements the (below) specified command line options, drives one or more parallel threads that do adds and subtracts to a shared variable, and reports on the final sum and performance. **source of C program is lab4.c, Makefile is Makefile. Makefile currently does not build cleanly**

graphs of: 
the average time per operation vs the number of iterations
the average time per operation vs the number of threads for all four versions of the add function.
**graphs currently not made. use gnuplot**

Run your program for a range of threads and iterations values, and note how many threads and iterations it takes to (fairly consistently) result in a failure (non-zero sum).

**QUESTIONS 1.1:  
Why does it take this many threads or iterations?**  
It takes this many thread or iterations because the execution and switching among threads is governed by the scheduler. Therefore, as the number of threads we have increases, the probability of the resulting in a failure should increase as well. This is also the case where increasing the number of iterations increases the probability of resulting in a failure, since there are more chances for the thread switching to occur in the middle of an add operation.

**Why does a significantly smaller number of iterations so seldom fail?**  
A significantly smaller number of iterations so seldomly fails because there are less opportunities for a thread performing an add operation to be 


Graph the average cost per operation (non-yield) as a function of the number of iterations, with a single thread. You should note that the average cost per operation goes down as the number of iterations goes up.

**QUESTIONS 1.2  
Why does the average cost per operation drop with increasing iterations?**  
The average cost per opration drops with increasing iteration because the overhead of the scheduler preemptively scheduling another thread (which happens to be the same one, since we are single-threaded) will be more evenly distributed as part of the average cost per operation at large iterations. At lower iterations, the overhead will result in a higher cost per operation since its overhead cost is distributed among fewer threads.

**How do we know what the “correct” cost is?**  
The "correct" cost is closer to the cost a higher iterations since we are looking for the cost associated with each add operation, not scheduling. Therefore, we want the scheduling time to be diluted over a larger number of iterations. We can alternatively get the cost per operation without the preemptive scheduling by measuring the user run time, as the scheduling is part of system time.

**Why are the --yield runs so much slower? Where is the extra time going?**  
Each time a thread yields, that thread gives up the CPU and is "is placed at the end of the run queue for its static priority and another thread is scheduled to run." (http://man7.org/linux/man-pages/man3/pthread_yield.3.html) This means that for every add() function call, the scheduler is forced to run and pick a thread. However, since in this specific testing case we only have one thread, the scheduler will pick the same thread that yielded and continue operating. This is in contrast to the preemptive scheduling case where the scheduler does not run during every single add() function call. The only difference between the two cases is that the --yield runs must yield in every single operation, so the extra time must be coming from context switching, i.e. yielding to the scheduler to pick another thread.

**Can we get valid timings if we are using --yield? How, or why not?**  
Yes, we can still get valid timings. Since yielding consumes system time rather than user time, if we want to know the actual cost per operation we can ignore the system time and grab the total user run time instead and divide it among the number of operations for the valid cost per operation.

**CONTINUE EDITING FROM HERE**

**QUESTIONS 1.3  
Why do all of the options perform similarly for low numbers of threads?**  
All of the options (mutex, spin-lock, and compare_and_swap) have approximately equivalent overheads for small nnumbers of threads. In the mutex's case, etc etc. In the spin-lock's case, etc etc. In the compare_and_swap case, etc etc.

**Why do the three protected operations slow down as the number of threads rises?**  
The mutex idk...
Spin-locks have a larger number of threads that can be selected and be spinning, attempting to acquire the resource. We must return to the original thread that locked the resource to unlock it.
The compare_and_swap synchronization method has a large number of threads that can interrupt it.

**Why are spin-locks so expensive for large numbers of threads?**  
Spin-locks are so expensive for large numbers of threads because each thread invoked by the scheduler must check if it is allowed access to the resource before it accesses it. This means that if a particular thread has a lock on the resource and yields or is otherwise forced to yield to a different thread, control must return to the original thread for the resource to be unlocked. During that time period, all other threads the scheduler switches to will perform a lock checking instruction, thereby consuming additional CPU time. If the scheduler is not aware, it may even check all other threads before finally returning to the original thread.

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
The mutex must be held when pthread_cond_wait is called because pthread_cond_wait is intended for a process currently holding the mutex (and therefore operating on a specific resource) to wait until a certain condition is fulfilled before continuing.

Why must the mutex be released when the waiting thread is blocked?
The mutex must be released when the waiting thread is blocked because otherwise other threads cannot access the resource or get work done. The conditional variable may never be unblocked if other threads are still waiting on the mutex, resulting in the calling thread infinitely waiting.

Why must the mutex be reacquired when the calling thread resumes?
The mutex must be reacquired when the calling thread resumes because the calling thread is still in its critical section and thus needs to continue operating on the designated resource.

Why must this be done inside of pthread_cond_wait? Why can’t the caller simply release the mutex before calling pthread_cond_wait?
I interpret this question to both refer to releasing the mutex and reacquiring the mutex. Both releasing and reacquiring the mutex must be done inside pthread_cond_wait since there is a potential race condition in both cases. If we do not atomically release and acquire the lock and instead release before calling pthread_cond_wait and acquire after finishing pthread_cond_wait, the scheduler could force a yield, resulting in a different thread acquiring the mutex when in fact we wish to return control back to the calling function.

Can this be done in a user-mode implementation of pthread_cond_wait? If so, how? If it can only be implemented by a system call, explain why?
No, because in a user-mode implementation we cannot guarantee any atomicity. It can only be implemented by a system call, since to absolutely guarantee atomicity we need to issue machine instructions which are typically not allowed in user-mode programs and must be done as a kernel program.