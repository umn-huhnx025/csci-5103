###############################################################
#
# CSCI 5103
# Lab 1 - User-level Thread Library
# 
# MEMBERS
#   Steven Storla - storl060
#   Jon Huhn - huhnx025
#   Paul Rheinberger - rhein055
#
###############################################################


###############################################################
#
# Table of Contents
#
###############################################################
DESIGN DECISIONS
  a. What (additional) assumptions did you make?
  b. Describe your API, including input, output, return value.
  c. Did you add any customized APIs? What functionalities do they provide?
  d. How did your library pass input/output parameters to a thread entry function?
  e. How did you implement context switch? E.g. using sigsetjmp or getcontext?
  f. How do different lengths of time slice affect performance?
  g. What are the critical sections your code has protected against interruptions and why?
  h. If you have implemented more advanced scheduling algorithm, describe them.
  i. How did you implement asynchronous I/O? E.g. through polling, or asynchronous signals?
  i. If your asynchronous read is based on asynchronous signals, how did you implement it? Does it support concurrent signals, e.g. when multiple fds are ready at the same time?
  j. Does your library allow each thread to have its own signal mask, or do all threads share the same signal mask? How did you guarantee this?
  
TESTING



###############################################################
#
# Design Decisions
#
###############################################################
a. What (additional) assumptions did you make?
    A number of assumptions were made about the system and the uthread library 
  while developing our uthread library. We are assuming that either the system
  will have enough memory to store every uthread we create or that we will not
  create enough uthreads to fill the system's memory. Again, in respect to memory,
  we are assuming that each uthread will not need more than 4kB of memory for its
  stack since stack size is hardcoded into our library. 
    Throughout our uthread library, we are assuming that one and only one uthread
  will be executing on the processor at any given point in time. We are also 
  making the assumption that calls to our uthread library functions have an 
  existing thread ID. If this were not the case, the mapping function would 
  return an error. In uthread_yield(), we assume that SIGVTALARM is the only 
  signal that can interrupt a thread. In async_read(), we assume that a write 
  operation is not happening while we are also also reading the file.
    We believe the assumptions listed above are reasonable to have designed into
  our library. If the system did not have enough memory to store all of the 
  created uthreads, there would not be much that the library could do to fix this
  problem. Therefore, the library assumes that it will not happen. Our 
  assumptions about the library are justifiable because if we have implemented 
  out functions correctly and the library functions are used appropriately, 
  thread ID's should be valid and only one uthread will be executing at a given 
  point in time. Additionally, reading a file that is currently being written 
  would affect other types of read as well, not just uthread's asynchronous read.
  Although there are a number of assumptions we have made about our uthread 
  library and the system it is running on, we believed these assumptions to be 
  justifiable by the reasons stated above.

b. Describe your API, including input, output, return value.
   The API is a set of thread functions that takes a thread id as its parameter. 
   Each function starts by first making sure the id is in the valid range for the given list. 
   Next, it will find the desired thread from the id passed in by using the TCBMap, and finally
   execute the body of the function. Each function will return 0 on success and -1 on failure.

c. Did you add any customized APIs? What functionalities do they provide?
   No customized API's were added.

d. How did your library pass input/output parameters to a thread entry function?
    Our thread entry functions operate similarly to pthread entry functions. 
    Arguments are passed as type void* so they can easily be converted to any 
    other type of the same size. This means complex types can be placed into a 
    struct and a pointer to that struct can be passed to the entry function to 
    access multiple values.

e. How did you implement context switch? E.g. using sigsetjmp or getcontext?
    We opted to use setcontext/getcontext for context switches.  These functions 
    make context switching and saving easy by wrapping everything up into the 
    uncontext_t struct and not relying on inline assembly code.

f. How do different lengths of time slice affect performance?
    At first I thought the length of time slices given to each uthread by the 
  scheduler could dramatically change the overall performance. When time slices
  are shorter, the processor spend more time context switching and less time
  actually running the uthreads. Whereas when time slices are longer, there is 
  less overhead spent on context switching, so uthreads spend more time executing
  instructions in the processor. However, after many different tests, it seems that 
  there is not a difference in performance even with different lengths of time slice.

g. What are the critical sections your code has protected against interruptions and why?
    We protect each occurrence of user state being saved or restored. These 
    operations must be atomic so that the saved or restored state matches 
    exactly with the actual context it refers to. If one of our threads were 
    preempted in the middle of saving state, for example, then the resulting 
    saved state may have the correct stack pointer, but still has the old 
    program counter. While this would only happen extremely rarely, we should 
    still protect against it as best we can.
    
    We also protect each instance where we modify the lists of threads we 
    maintain, including a ready list, terminated list, and suspended list. If an 
    interrupt were to occur in the middle of updating these shared objects, then 
    one thread may be reading incorrect information. Disabling interrupts 
    ensures that no thread will get left behind as the result of an incomplete 
    operation.


h. If you have implemented more advanced scheduling algorithm, describe them.
   We did not implement more advanced scheduling algorithms. However, we did 
   implement a list data structure. It works by pushing the newest thread 
   to the back of the ready list, and popping the next thread to run from the 
   front; a sort of FIFO ordering with exceptions after deletions occur.

i. How did you implement asynchronous I/O? E.g. through polling, or asynchronous signals?
    Since our uthread library runs at the user level, when any uthread makes a 
  system call that blocks, the whole process suspends and therefore all uthreads 
  in that process stop executing as well. In order to make an asynchronous I/O 
  call, we implemented a wrapper around the aio_read() system call. This wrapper
  ensures that asynchronous I/O reads can be executing by multiple concurrent 
  uthreads without suspending the process. It is important to note that the call
  to async_read() appears as blocking to the calling uthread, because it will not
  return until the read is completed, but does not block any other uthreads in 
  the process.
    Our implementation of the async_read() function uses polling to determine the 
  status of the read file descriptor and yields the uthread if the read is still 
  in progress. A more efficient alternative to this implementation is to use SIGIO
  signals to signal that the asynchronous read has been completed. In this 
  implementation, no polling would be done and thus reduces the overhead of context
  switches. The uthread would be suspended after the aio_read() call and a signal 
  that the read has been completed would place the uthread back on the ready queue
  to be scheduled.
    Although our polling implementation of asynchronous I/O is much less efficient
  than using signals, we believe that this design decision is justifiable based on
  the context of other project requirements. It is mentioned in the scheduler 
  requirements that if a running thread is suspended or yields, its remaining time
  slice will be abandoned. That is, no uthreads will be executing on the processor
  until the next time slice begins. It is clear that efficiency is not a priority 
  for this library. If it was, the requirement would be changed to have the next 
  uthread scheduled to run immediately after the current uthread suspends or 
  yields. Since efficiency is not a main priority for this project, we decided to
  use polling as it is much simpler to implement and maintain. The justification 
  for this simpler decision is further supported by the principle of economy of 
  mechanism, described by Saltzer and Schroeder. Even though this library is 
  relatively small, notice that the only asynchronous I/O ability is to read, if 
  plans are made to extend the libraries capabilities in the future, a simpler, 
  easy to read code base will significantly help developers secure, maintain, and
  extend the uthread library.


i. If your asynchronous read is based on asynchronous signals, how did you implement it? Does it support concurrent signals, e.g. when multiple fds are ready at the same time?
    Our implementation of asynchronous read was not based on asynchronous signals. See section (i) above.


j. Does your library allow each thread to have its own signal mask, or do all threads share the same signal mask? How did you guarantee this?
    Each thread has its own signal mask. We use a ucontext_t object to store the user thread context, which has a signal mask in a sigset_t field. This guarantees that each thread has its own signal mask, so they could be changed independently.



###############################################################
#
# Testing
#
###############################################################
    There are four main testing modules used for testing critical functionality of
  the uthread library. The four modules are test_yield_main, test_async_read_main, 
  test_create_main, and test_scheduler_main. These modules can be compiled by 
  executing the `make` command. All four testing module executables are created by
  the make command.
       
  test_yield_main:
    This module tests the yield and join functions in the uthread library. The 
    main thread creates two child threads. The first child thread then yields 
    the CPU in the middle of its execution. This results in manually interleaved 
    execution of the two threads. The expected output is as follows:
    `
    thread 1: Before yield. Yielding...
    thread 2: This should happen in between.
    thread 1: After yield. Done.
    `

    If the uthread_yield statement is commented out, the threads will almost 
    always run sequentially, resulting in the following output:
    `
    thread 1: Before yield. Yielding...
    thread 1: After yield. Done.
    thread 2: This should happen in between.
    `

  test_async_read_main:
    The test_async_read_main test module tests the functionality of the asynchronous 
  read library function async_read() by first creating six distinct files each
  containing the string "Hello, World!". Next, six uthread are created, one for each
  file. The uthreads call async_read() on their respective file and printout thier 
  thread ID followed by the contents of the files to standard output. An example output
  of test_async_read_main is as follows.
      `Thread 1: File Contents: Hello, World!
      
       Thread 3: File Contents: Hello, World!
      
       Thread 5: File Contents: Hello, World!
      
       Thread 6: File Contents: Hello, World!
      
       Thread 4: File Contents: Hello, World!
      
       Thread 2: File Contents: Hello, World!
      `
    These results are what is expected to be outputted. Each uthread successfully reads
  its respective file and prints its thread ID and file contents to standard output. 
  Additionally, the uthread do not write to output in the same order they were created 
  due to when the scheduler decides to run each uthread. This is why the threads do not 
  always output in the same order. This is to be expected though and a good indication 
  that the both the scheduler and the async_read() function are performing correctly.

  test_create_main:
    This module tests the most basic functionality of the uthread library. The 
    main thread creates a child thread, passing an argument, and waits for its 
    completion with uthread_join. The thread's entry function will then read the
    argument, and return some value. Then, the main thread will read the value
    that the function returned. We also identify each thread by the uthread_self
    routine. The expected output is as follows:
    `
    thread 0: Creating new thread. Arg is 0x42424242
    thread 1: Arg is 0x42424242.
    thread 1: Returning 0xdeadbeef
    thread 0: New thread returned value 0xdeadbeef
    `

  test_scheduler_main:
      This test will show the user the use of the suspend, resume, and terminate thread functions.
    It is also designed in such a way, that the user can sit back and follow the terminal
    output. This is done by using a serious of long loops and thread sleeps. 
      The function will start by creating one thread who’s function is "Suspend_resume_terminate."
     Next, inside the function, two threads will be created and they will call the "spin" function. 
     As a thread in the spin function executes, it will print it's id in a loop every 199999980
     iterations, and will never exit until terminate is called. 
      The first thread will start by running both threads, suspend the second thread, run a bit longer, 
      resume the second thread, run a bit longer, terminate the second thread, run a bit longer, 
      terminate the third thread, and finally exit. I strongly encourage
      you to run the test as it is very easy to follow and shows exactly what is going on.

    
