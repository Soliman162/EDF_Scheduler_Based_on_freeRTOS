# EDF_Scheduler_Based_on_freeRTOS
* In this project i have designed an EDF scheduler based on FreeRTOS


- Earliest Deadline First (EDF) is a shecduling algorithm that adopts a dynamic priority-based preemptive scheduling policy, meaning that the priority
of a task can change during its execution, and the processing of any task is interrupted by a request for any higher priority task.

## Read a thesis and implement the required changes
1. Download the following thesis: [Implementation and Test of EDF and LLREFSchedulers in FreeRTOS](https://thesis.unipd.it/bitstream/20.500.12608/25547/1/Implementation_and_Test_of_EDF_and_LLREF_Scgheduler_in_FreeRTOS.pdf).
2. Read chapter 2 : "FreeRTOS Task Scheduling". This is an important chapter to build a profound base before starting the project.
3. Read chapter 3 : "EDF Scheduler". This chapter is the main chapter you will use to implement the EDF scheduler using FreeRTOS.
4. Implement the changes mentioned in chapter 3.2.2 : "Implementation in FreeRTOS". The changes will be implemented in tasks.c source file only.

## Implement the missing changes from the thesis
1. In the `prvIdleTask` function:
- Modify the idle task to keep it always the farest deadline

2. In the `xTaskIncrementTick` function:
- In every tick increment, calculate the new task deadline and insert it in the correct position in the EDF ready list

3. In the `xTaskIncrementTick` function:
- Make sure that as soon as a new task is available in the EDF ready list, a context switching should take place. 
- Modify preemption way as any task with sooner deadline must preempt task with larger deadline instead of priority.

## Implement 4 tasks using EDF scheduler
1. Create 4 tasks with the following criteria:
- Task 1: `Button_1_Monitor`, {Periodicity: 50, Deadline: 50} This task will monitor rising and falling edge on button 1 and send this event to the consumer task. 
> Note: The rising and failling edges are treated as separate events, hence they have separate strings
- Task 2: `Button_2_Monitor`, {Periodicity: 50, Deadline: 50} This task will monitor rising and falling edge on button 2 and send this event to the consumer task. 
> Note: The rising and failling edges are treated as separate events, hence they have separate strings

- Task 3: `Periodic_Transmitter`, {Periodicity: 100, Deadline: 100} This task will send preiodic string every 100ms to the consumer task

- Task 4: `Uart_Receiver`, {Periodicity: 20, Deadline: 20} This is the consumer task which will write on UART any received string from other tasks

2. Add a 5th and 6th task to simulate a heavier load:

- Task 5: `Load_1_Simulation`, {Periodicity: 10, Deadline: 10}, Execution time: 5ms
- Task 6: `Load_2_Simulation`, {Periodicity: 100, Deadline: 100}, Execution time: 12ms

## Verifying the system implementation

- Using Simso offline simulator, simulate the given set of tasks assuming: Fixed priority rate monotonic scheduler 

- Using Keil simulator in run-time and the given set of tasks: Calculate the CPU usage time using timer 1 and trace macros Using trace macros and GPIOs, plot the execution of all tasks, tick, and the idle task on the logic analyzer
