# Scheduler --> Operating System
Simulate process scheduler
### Introduction
Processes are the basic execution entities in every modern operating system. As a result, process control and management is, therefore, an important topic.<br>
This project implements a workable process scheduler in Linux systems. It should read a description file which contains several jobs with different parameters (including arrival time, CPU requirements and the job commands)
<br>`Programming language: C`<br>
Platform: Linux -- `CentOS 6.0`<br>
Writers: `Chen Boyu` && `Liu Yubo`
### Task Allocations:
Chen: Monitor, FCFS, Non-preemptive SJF policy.<br>
Liu:  Monitor, Round-Robin, three Gantt charts.<br>
## Monitor
For the monitor process, it stands between the scheduler and the job process.<br>
1.  Goal: measure the actual (or elapsed) time, the user time, and the system time taken of the job process.<br>
2.  Receive the signals from the scheduler, then do the corresponding things to the job processes.
## Scheduler
`forks and executes` the monitor process
1.  Goal: Read the description file, then do the corresponding operations
2.  Algorithms: FCFS, None-preemptive SJF, Round-Robin
3.  Draw the Gantt charts
