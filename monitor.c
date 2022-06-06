/*
*CO004 Operating System Project Phase 1
*Final Aim:Use C to simulate the scheduler for Linux
*Linux system: CentOS 6 64-bit system
*Aim: To create a monitor process to get the time.
*	  From monitor. it will create a child process to do the job we want.
*	  User can pass the signal SIGTSTP, SIGTERM, SIGCONT to the monitor Process.
*	  Acoording to the signals, the job process will have a feedback.
*Group Member: Chen Boyu   Liu Yubo
*Student ID: 1809853V-I011-0036
*			 18098537-I011-0082
*/
#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/times.h>  // times() function
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h> 
#include<signal.h>     // signal and kill() function
int result;//Use to store the pid

/*
	from minitor jump to job process
*/
void to_exec(char *array[]){ 
	execvp(array[0], array);
	exit(1);
}

/*
	Mointor judge the signal type
	Then transpond the signal to job process
	Job process execute the signal.
*/
void signal_handle(int sig)   
{   
    if(sig == SIGTERM){
        kill(result,SIGTERM);  //kill job process
         }
     if(sig == SIGTSTP){
        kill(result,SIGTSTP);   //suspends child process
        printf("Child suspend\n");
     }
     if(sig == SIGCONT){
        kill(result,SIGCONT);  //continues child process
        printf("Child continues\n");
         }
}

int main (int argc, char* argv[])
{ 
	char* array[argc];
        struct tms start_t , end_t;//two struct to store the time from times()
        long   tck = 0;
        clock_t time_start , time_end ;//This is used to get the whole time of a process
        int i ;

        tck = sysconf (_SC_CLK_TCK );//To get the system clock 
        time_start = times ( & start_t );   //the time the process started executing
	
	/*
	*	Pass the job description to array[]
	*	Because of the exec* family, the last value must be NULL.
	*	We will read them form argv[]
	*/
	for(i = 1; i < argc; i++){
		array[i-1] = argv[i];	
	}
	array[argc - 1] = NULL;  

	result = fork();  //create a child process

	if(result == 0){
		to_exec(array);  //to to_exec() function jump to job process
	}
	else{
        signal(SIGTERM, signal_handle);  //handle SIGTERM signal
        signal(SIGTSTP, signal_handle);  //handle SIGTSTP signal
        signal(SIGCONT, signal_handle);  //handle SIGCONT signal
		wait(NULL);       		 //wait until child process died
        time_end = times ( & end_t );	 //the time the process ended executing
		double total = ((time_end - time_start) / (double)tck);	 //caculate the Total time
		printf ("PID %d:     time elapsed is: %f\n", result,total);   //print the total time
		printf ("	     User time: %0.5f\n" , ((end_t . tms_cutime - start_t .tms_cutime ) / (float)tck )); //caculate user time and print it
		printf ("            Sys time: %0.5f\n" , ((end_t . tms_cstime - start_t .tms_cstime ) / (float)tck )); //caculate system time and print it
		sleep(1);
	}

        return 0;
}