#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <sys/times.h>  // times() function
#include <sys/types.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>     // signal and kill() function
#include <string.h>
#include <ctype.h>

#define MAXJOB 9
#define MAXCMD 20

static int sigchild = 0;
static char mixed[100];
static int jnum = 0;
static char gantt[100][100];
static int nk = 0;
static int rjob = -1;
static int fjob = 0;
static char* Cmdarray[20];
static int mix;

struct stu {
	int pid[MAXJOB];//store the job process pid
	int line[MAXJOB];//line number -> job id
	int start[MAXJOB];//arrival time
	char* cmd[MAXJOB][MAXCMD];//command 
	int duration[MAXJOB];//duration time
	int size[MAXJOB];//command size
	char* job[MAXJOB][500];//use for draw the gantt
	int ctime[50];
	int Remain[200];
	int in[200];//flag
	int Finish[200];
}sch, temp_sch;//token for store the each part in job description file

void Hsigchild(int sig) {
	sigchild = 1;
}
void hsigtstp(int sig) {

}

int Create(int job) {
	int pid, j;
	pid = fork();
	if (pid != 0) {
		sch.pid[job] = pid;//store monitor pid;
		return pid;
	}
	else {
		int size = sch.size[job];
		char* cmdarray[size];
		for (j = 0; j < size; j++) {
			cmdarray[j] = sch.cmd[job][j];
		}
		cmdarray[size] = NULL;
		execvp(cmdarray[0], cmdarray);
	}
	return 0;
}

void Print(int job, int total) {
	int t, j, i;
	int m = 0;
	for (t = 0; t < job; t++) {
		for (i = 0; i <= total; i++) {
			if (t == 0) {
				if (i < sch.start[t]) {
					sch.job[t + 1][i] = " ";
				}
				else if (i >= sch.start[t] && i < (sch.start[t] + sch.duration[t])) {
					sch.job[t + 1][i] = "#";
				}
				else {
					sch.job[t + 1][i] = " ";
				}
			}
			else if (t != 0) {
				if (i == 0)
					m = (sch.start[t - 1] + sch.duration[t - 1] + m) - sch.start[t];

				if (i < sch.start[t]) {
					sch.job[t + 1][i] = " ";
				}
				else if (i < (m + sch.start[t]) && i >= sch.start[t]) {
					sch.job[t + 1][i] = ".";
				}
				else if (i >= (sch.start[t] + m) && i < (sch.start[t] + sch.duration[t] + m)) {
					sch.job[t + 1][i] = "#";
				}
				else if (i >= (sch.start[t] + sch.duration[t] + m)) {
					sch.job[t + 1][i] = " ";
				}
			}
		}
	}
	int f;
	printf("Gantt Chart\n");
	printf("Time ");
	for (f = 0; f < total; f++) {
		if (f % 10 == 0)
			printf(" %d", f / 10);
		else {
			printf("  ");
		}
	}
	printf("\n     ");

	for (f = 0; f < total; f++) {
		printf(" %d", f % 10);
	}
	printf("\n");

	for (f = 1; f <= job; f++) {
		printf("Job%d  ", f);
		for (t = 0; t <= total; t++) {
			printf("%s ", sch.job[f][t]);
			if (sch.job[f][t] == "#") {
				mixed[t] = f;
			}
		}
		printf("\n");
	}
	printf("Mixed");
	for (i = 0; i < total; i++) {
		printf(" %d", mixed[i]);
	}
	printf("\n");
}

void FCFS(int line, int total) {
	int j, i;
	int time = 0;
	int check = 0;
	int fir = 1;
	total = total + 1;
	int run_job = 0;
	int change = 1;
	while (time < total) {
		int run = 0;
		int running[MAXJOB];
		int finish[MAXJOB];
		int temp_time[MAXJOB];
		int temp_dur[MAXJOB];
		int kpid[line];
		for (i = 0; i < line; i++) {
			if (sch.start[i] == time) {
				running[i] = 1;
				if (time == 0) {
					temp_time[i] = sch.duration[i] - 1;
				}
				else {
					temp_time[i] = temp_time[i - 1] + sch.duration[i];
				}
			}
		}
		if (run == 0) {
			for (i = 1; i <= line; i++) {
				if (check == 0 && time == sch.start[i - 1]) {
					run = i;
				}
			}
			for (i = 0; i < line; i++) {
				if (temp_time[i] != 0 && temp_time[i] + 1 == time && time > 0) {
					finish[i] = 0;//not do the job
				}
			}
			for (i = 0; i < line; i++) {
				if (sch.start[i] == 0 && fir != 0 && time > 0) {
					int pid = Create(i);
					sch.pid[i] = pid;
					fir = 0;
					running[i] = 0;
					finish[i] = 1;
				}
				if (running[i] > 0 && running[i - 1] == 0 && finish[i - 1] == 0) {
					running[i] = 0;
					finish[i] = 1;//do the job
					run_job = i;
				}
			}
			if (run_job != 0) {
				int pid = Create(run_job);
				sch.pid[run_job] = pid;
				run_job = 0;
			}
			for (i = 0; i < line; i++) {
				char* equ = "ls";
				if (sch.start[i] == 0) {
					if (time == sch.duration[i] - 1) {
						if (strcmp(equ, sch.cmd[i][1]) == 0 && (sch.start[i] == 0 && sch.duration[i] > 1)) {
							kpid[i] = 0;
						}
						else {
							kpid[i] = 1;
						}
					}
				}
				else {
					if (finish[i] == 1 && temp_time[i] > 0 && time == temp_time[i]) {
						kpid[i] = 1;
					}
				}
			}
			run = 0;
		}//No job do		
		time++;
		sleep(1);
		for (i = 0; i < line; i++) {
			if (kpid[i] == 1) {
				if (sch.start[i] == 0) {
					sleep(sch.duration[i] - 1);
					kill(sch.pid[i], SIGTERM);
				}
				else if (sch.start[i] > 0) {
					kill(sch.pid[i], SIGTERM);
				}
				else if (sch.start[i] == 0 && sch.duration[i] == 1) {
					printf("Not kill\n");
				}
				kpid[i] = 0;
			}
			else if (kpid[i] == 0) {
				//do nothing
			}
		}
	}
	int status;
	wait(sch.pid[line - 1], &status, 0);
	Print(line, total - 1);
}

void Print_SJF(int jobnum, int total) {
	int j, i, f, t, temp_arr, temp_dur;
	int m = 0;
	int job[jobnum][total];
	for (t = 0; t < jobnum; t++) {
		int num = t;
		int x, c;
		for (i = 0; i <= total; i++) {

			if (t == 0) {
				if (i < temp_sch.start[t]) {
					temp_sch.job[t + 1][i] = " ";
				}
				else if (i >= temp_sch.start[t] && i < (temp_sch.start[t] + temp_sch.duration[t])) {
					temp_sch.job[t + 1][i] = "#";
				}
				else {
					temp_sch.job[t + 1][i] = " ";
				}

			}
			else if (temp_sch.start[t] <= (temp_sch.start[t - 1] + temp_sch.duration[t - 1] + m)) {

				if (i == 0) {
					while (temp_sch.start[num] <= (temp_sch.start[t - 1] + temp_sch.duration[t - 1] + m) && num < jobnum) {
						num++;
					}
					for (x = 1; x < num - 1; x++) {
						for (c = t; c < num - x; c++) {
							if (temp_sch.duration[c] > temp_sch.duration[c + 1] && temp_sch.duration[c + 1] != 0) {
								temp_arr = temp_sch.start[c];
								temp_sch.start[c] = temp_sch.start[c + 1];
								temp_sch.start[c + 1] = temp_arr;
								temp_dur = temp_sch.duration[c];
								temp_sch.duration[c] = temp_sch.duration[c + 1];
								temp_sch.duration[c + 1] = temp_dur;
							}
						}
					}
				}
				if (i == 0) {
					m = (temp_sch.start[t - 1] + temp_sch.duration[t - 1] + m) - temp_sch.start[t];
				}
				if (i < temp_sch.start[t]) {
					temp_sch.job[t + 1][i] = " ";
				}
				else if (i < (m + temp_sch.start[t]) && i >= temp_sch.start[t]) {
					temp_sch.job[t + 1][i] = ".";
				}
				else if (i >= (temp_sch.start[t] + m) && i < (temp_sch.start[t] + temp_sch.duration[t] + m)) {
					temp_sch.job[t + 1][i] = "#";
				}
				else if (i >= (temp_sch.start[t] + temp_sch.duration[t] + m)) {
					temp_sch.job[t + 1][i] = " ";
				}
			}
			else {
				if (i == 0)
					m = (temp_sch.start[t - 1] + temp_sch.duration[t - 1] + m) - temp_sch.start[t];
				if (i < temp_sch.start[t]) {
					temp_sch.job[t + 1][i] = " ";
				}
				else if (i < (m + temp_sch.start[t]) && i >= temp_sch.start[t]) {
					temp_sch.job[t + 1][i] = ".";
				}
				else if (i >= (temp_sch.start[t] + m) && i < (temp_sch.start[t] + temp_sch.duration[t] + m)) {
					temp_sch.job[t + 1][i] = "#";
				}
				else if (i >= (temp_sch.start[t] + temp_sch.duration[t] + m)) {
					temp_sch.job[t + 1][i] = " ";
				}
			}
		}
	}

	wait(NULL);
	printf("Gantt Chart\n");
	printf("Time ");
	for (f = 0; f < total; f++) {
		if (f % 10 == 0)
			printf(" %d", f / 10);

		else {
			printf("  ");
		}
	}
	printf("\n     ");

	for (f = 0; f < total; f++) {
		printf(" %d", f % 10);
	}
	printf("\n");

	for (f = 1; f <= jobnum; f++) {
		printf("Job%d  ", f);
		for (t = 0; t <= total; t++) {
			printf("%s ", temp_sch.job[f][t]);
			if (temp_sch.job[f][t] == "#") {
				mixed[t] = temp_sch.line[f - 1];
			}
		}
		printf("\n");
	}
	printf("Mixed");
	for (i = 0; i < total; i++) {
		printf(" %d", mixed[i]);
	}
	printf("\n");
}

void Sort(int job) {
	char* temp_cmd[MAXJOB][MAXCMD];
	int temp_line;
	int temp1, temp2, i, j, k;
	for (i = 0; i < job; i++) {
		if (sch.start[i] > 0) {
			for (j = i + 1; j < job; j++) {
				if ((sch.start[j] >= sch.start[i]) && (sch.duration[i] > sch.duration[j])) {
					temp1 = sch.start[j];//change the arrive time 
					sch.start[j] = sch.start[i];
					sch.start[i] = temp1;
					for (k = 0; k < MAXCMD; k++) {
						temp_cmd[i][k] = sch.cmd[j][k];
						sch.cmd[j][k] = sch.cmd[i][k];
						sch.cmd[i][k] = temp_cmd[i][k];
					}//change the command
					temp2 = sch.duration[j];//change the duration time 
					sch.duration[j] = sch.duration[i];
					sch.duration[i] = temp2;
					temp_line = temp_sch.line[j];//change the job num 
					temp_sch.line[j] = temp_sch.line[i];
					temp_sch.line[i] = temp_line;
				}
			}
		}
	}
}

void SJF(int job, int total)
{
	Sort(job);
}

void RR(char* command_array[MAXCMD]) {
	nk -= 1;
	int lastjob = -1;
	int slice = 0;
	int k;
	while (1) {
		for (k = 0; k < jnum; k++) {
			if (gantt[nk][k] != '#') {
				gantt[nk][k] = ' ';
			}
			if (sch.Finish[k] == 0 && sch.in[k] == 1) {
				gantt[nk][k] = '.';
			}
			gantt[nk][rjob] = '#';

		}
		sleep(1);
		nk += 1;
		if (slice == 2 && rjob != -1) {
			kill(sch.pid[rjob], SIGTSTP);
			lastjob = rjob;
			rjob = -1;
			slice = 0;
		}
		for (k = 0; k < jnum; k++) {
			if (nk == sch.start[k] && sch.in[k] == 0) {
				int id;
				sch.in[k] = 1;
				sch.pid[k] = fork();
				if (sch.pid[k] != 0) {
					if (sch.duration[k] != -1) {
						sleep(sch.duration[k] - 1);
						kill(sch.pid[k], SIGTERM);
						wait(NULL);
					}
					else
						wait(NULL);
				}
				else {
					execvp(command_array[0], command_array);
				}

			}
		}
		if (rjob == -1) {
			for (k = jnum; k > lastjob; k--) {
				if (sch.in[k] == 1 && sch.Finish[k] == 0) {
					rjob = k;
				}
			}
			if (rjob == -1) {
				if (lastjob > 0) {
					for (k = lastjob - 1; k > -1; k--) {
						if (sch.in[k] == 1 && sch.Finish[k] == 0) {
							rjob = k;
						}
					}
				}
				if (rjob == -1) {
					if (sch.Finish[lastjob] == 0) {
						rjob = lastjob;
					}
					else {
						rjob = -1;
					}
				}
			}
			if (rjob != -1) {
				kill(sch.pid[rjob], SIGCONT);
				slice = 0;
				if (sch.duration[rjob] != -1) {
					sch.duration[rjob] -= 1;
					slice += 1;
				}
				if (sch.duration[rjob] == 0 || (sigchild == 1 && sch.duration[rjob] == -1)) {
					gantt[nk][rjob] = '#';
					kill(sch.pid[rjob], SIGTERM);
					sch.Finish[rjob] = 1;
					lastjob = rjob;
					rjob = -1;
					sigchild = 0;
					fjob = fjob + 1;
				}
			}
		}
		else {
			if (sch.duration[rjob] != -1) {
				sch.duration[rjob] -= 1;
				slice += 1;
			}
			if (sch.duration[rjob] == 0 || (sigchild == 1 && sch.duration[rjob] == -1)) {
				gantt[nk][rjob] = '#';
				kill(sch.pid[rjob], SIGTERM);
				sch.Finish[rjob] = 1;
				lastjob = rjob;
				rjob = -1;
				sigchild = 0;
				fjob = fjob + 1;
				continue;
			}
		}
		if (fjob == jnum) {
			int k;
			int i;
			for (k = 0; k <= jnum; k++) {
				if (gantt[nk][k] != '#')
					gantt[nk][k] = ' ';
				if (sch.Finish[k] == 0 && sch.in[k] == 1) {
					gantt[nk][k] = '.';
				}
				gantt[nk][rjob] = '#';
			}
			printf("Gantt Chart\n");
			printf("tick: ");
			for (k = 0; k <= nk; k++) {
				if (k % 10 == 0) {
					printf(" %d", k / 10);
				}
				else {
					printf("  ");
				}
			}
			printf("\n      ");

			for (k = 0; k <= nk; k++) {
				printf(" %d", k % 10);
			}
			printf("\n");
			for (k = 0; k < jnum; k++) {
				printf("job%d  ", k + 1);
				for (i = 0; i <= nk; i++) {
					printf(" %c", gantt[i][k]);
					if (gantt[i][k] == '#') {
						mixed[i] = k + 1;
					}
				}
				printf("\n");
			}
			printf("Mixed ");
			for (i = 0; i <= nk; i++) { printf(" %d", mixed[i]); };
			printf("\n");
			return;
		}

	}
}

void Policy(int job, int total_time, char* policy) {
	int i, j;
	if (strcmp(policy, "FCFS") == 0) {
		if (sch.duration[0] > 1)
			FCFS(job, total_time);
		else {
			for (i = 0; i < job; i++) {
				sch.pid[i] = fork();
				if (sch.pid[i] != 0) {
					if (sch.duration[i] != -1) {
						sleep(sch.duration[i]);
						kill(sch.pid[i], SIGTERM);
						wait(NULL);
					}
					else
						wait(NULL);
				}
				else {
					int size = sch.size[i];
					char* cmdarray[size];
					for (j = 0; j < size; j++) {
						cmdarray[j] = sch.cmd[i][j];
					}
					cmdarray[size] = NULL;
					execvp(cmdarray[0], cmdarray);
				}
			}
			Print(job, total_time);
		}
	}
	else if (strcmp(policy, "SJF") == 0) {
		SJF(job, total_time);
		for (i = 0; i < job; i++) {
			sch.pid[i] = fork();
			if (sch.pid[i] != 0) {
				if (sch.duration[i] != -1) {
					sleep(sch.duration[i]);
					kill(sch.pid[i], SIGTERM);
					wait(NULL);
				}
				else
					wait(NULL);
			}
			else {
				int size = sch.size[i];
				char* cmdarray[size];
				for (j = 0; j < size; j++) {
					cmdarray[j] = sch.cmd[i][j];
				}
				cmdarray[size] = NULL;
				execvp(cmdarray[0], cmdarray);
			}
		}
		Print_SJF(job, total_time);
	}
	else if (strcmp(policy, "RR") == 0) {
		int size;
		for (i = 0; i < job; i++) {
			size = sch.size[i];
			for (j = 0; j < size; j++) {
				Cmdarray[j] = sch.cmd[i][j];
				Cmdarray[size] = NULL;
			}
		}
		jnum = job;
		RR(Cmdarray);
	}
}

int main(int argc, char* argv[]) {
	char* filename = argv[1];
	char* policy = argv[2];
	FILE* fp;
	fp = fopen(filename, "r");

	if (fp == NULL) {
		printf("Can't open!\n");
		exit(1);
	}
	int line = 0, i, j;
	char temp[MAXJOB][MAXCMD];
	char* token;//delete the space
	int total_time = 0;//cacluate the total time of the processes.
	int chno = 0;//the number of the cmd
	char* del = "\t ";

	while (fgets(temp[line], 100, fp)) {//read by one line
		token = strtok(temp[line], "\n");
		sch.line[line] = line;//get the job id
		temp_sch.line[line] = line + 1;//For SJF
		token = strtok(token, del);
		sch.start[line] = atoi(token);// start time
		temp_sch.start[line] = atoi(token);// start time for SJF

		sch.cmd[line][0] = "./monitor";
		while (token != NULL) {
			if (chno > 0) {
				sch.cmd[line][chno] = token;
			}
			chno++;
			token = strtok(NULL, del);
		}//store the cmd
		sch.duration[line] = atoi(sch.cmd[line][chno - 1]);// duration time
		temp_sch.duration[line] = atoi(sch.cmd[line][chno - 1]);// duration time for SJF
		sch.size[line] = chno - 1;//get the cmd size
		total_time += sch.duration[line];
		chno = 0;//Reset the number 
		line++;//Read next line
	}
	Policy(line, total_time, policy);

	fclose(fp);
	return 0;
}
