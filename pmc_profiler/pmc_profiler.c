#define _GNU_SOURCE
#define _POSIX_C_SOURCE 1
#define _XOPEN_SOURCE
#define _POSIX_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <sched.h>
#include <errno.h>

#define N 64
#define MAXLINELEN 1024
#define PIPE_FILENO 3
#define MAX_PMC_REC 128
int pid_record[N], app_pid;
int cid;
unsigned cpu_id, node_id;
cpu_set_t mask;
sigset_t sig_mask;
struct sigaction pmc_act;
char buf[MAXLINELEN];
unsigned long pmc_readings[6][MAX_PMC_REC];
int pmc_reading_count = 0;
int app_finished = 0;

inline uint64_t read_pmc(unsigned int n)
{
unsigned int a, d, c;
c = n;
__asm __volatile("rdpmc" : "=a" (a), "=d" (d) : "c" (c));
return ((unsigned long) a) | (((unsigned long) d) << 32);
}

void sigchld_app(int signo)
{
    unsigned int pid;
    int state;

    while ((pid = waitpid(-1, &state, WNOHANG)) > 0){
	if(pid == app_pid) {
	    app_finished++;
	    //fprintf(stderr, "The application with pid %d is terminated\n", app_pid);
	} else if(!app_finished) fprintf(stderr, "A child process is unexpectedly terminated\n");
		else {
			app_finished++;
			if(app_finished == N) exit(0);
		}
    }
    return;
}

void pmc_event(int signo)
{
    unsigned long tmp;
    int i;

    //fprintf(stderr, "Received signal #%d in child %d\n", signo, cid);
    /* read pmc counters here */
    if(signo == SIGUSR1){
        if(pmc_reading_count >= MAX_PMC_REC) 
	    fprintf(stderr, "pmc_readings is full!!!\n");
        tmp = pmc_readings[0][pmc_reading_count++] = read_pmc(0);
        //printf("%ld from child %d\n", tmp, cid);
    } else if (signo == SIGUSR2){
	printf("Process %d readings: ", cid);
	for(i=0; i<pmc_reading_count; i++) printf("%ld ", pmc_readings[0][i]);
	printf("\n");
	if(cid != 0) exit(0);
    }

    return;

}

void do_parent()
{
    pmc_act.sa_handler = SIG_IGN;
    sigemptyset(&pmc_act.sa_mask);
    pmc_act.sa_flags = 0;
    if( sigaction(SIGUSR1, &pmc_act, 0) < 0) {
        fprintf(stderr, "Failed installing sig handler for the parent ...\n");
        exit(1);
    }
    if( sigaction(SIGUSR2, &pmc_act, 0) < 0) {
        fprintf(stderr, "Failed installing sig handler for the parent ...\n");
        exit(1);
    }

}


void do_parent_again()
{
    CPU_ZERO(&mask);
    CPU_SET(0, &mask);
    if(sched_setaffinity(0, sizeof(mask), &mask)){
        fprintf(stderr, "Failed pinning process on CPU %d ...\n", 0);
        exit(1);
    }

    pmc_act.sa_handler = pmc_event;
    sigemptyset(&pmc_act.sa_mask);
    sigaddset(&pmc_act.sa_mask, SIGUSR2);
    sigaddset(&pmc_act.sa_mask, SIGCHLD);
    pmc_act.sa_flags = 0;
    if( sigaction(SIGUSR1, &pmc_act, 0) < 0) {
        fprintf(stderr, "Failed installing sig handler for the parent ...\n");
        exit(1);
    }

    sigemptyset(&pmc_act.sa_mask);
    sigaddset(&pmc_act.sa_mask, SIGUSR1);
    sigaddset(&pmc_act.sa_mask, SIGCHLD);
    if( sigaction(SIGUSR2, &pmc_act, 0) < 0) {
        fprintf(stderr, "Failed installing sig handler for the parent ...\n");
        exit(1);
    }

    sigemptyset(&pmc_act.sa_mask);
    sigaddset(&pmc_act.sa_mask, SIGUSR1);
    sigaddset(&pmc_act.sa_mask, SIGUSR2);
    pmc_act.sa_handler = sigchld_app;
    if( sigaction(SIGCHLD, &pmc_act, 0) < 0) {
        fprintf(stderr, "Failed installing sig handler for the parent ...\n");
        exit(1);
    }

    cid = 0;
    cpu_id = sched_getcpu();
    fprintf(stderr, "Parent is on %d of node %d\n", cpu_id, node_id);
}

void do_child(int cn)
{
    CPU_ZERO(&mask);
    CPU_SET(cn, &mask);
    if(sched_setaffinity(0, sizeof(mask), &mask)){
        fprintf(stderr, "Failed pinning process on CPU %d ...\n", cn);
        exit(1);
    }

    pmc_act.sa_handler = pmc_event;
    sigemptyset(&pmc_act.sa_mask);
    sigaddset(&pmc_act.sa_mask, SIGUSR2);
    pmc_act.sa_flags = 0;
    if( sigaction(SIGUSR1, &pmc_act, NULL) < 0) {
        fprintf(stderr, "Failed installing sig handler for child %d ...\n", cn);
        exit(1);
    }
    sigemptyset(&pmc_act.sa_mask);
    sigaddset(&pmc_act.sa_mask, SIGUSR1);
    if( sigaction(SIGUSR2, &pmc_act, NULL) < 0) {
        fprintf(stderr, "Failed installing sig handler for child %d ...\n", cn);
        exit(1);
    }

    cid = cn;
    //if(syscall(SYS_getcpu, &cpu_id, &node_id, NULL) < 0)
	//perror("getcpu");
    cpu_id = sched_getcpu();
    fprintf(stderr, "Child %d is on %d of node %d\n", cid, cpu_id, node_id);

    sigemptyset(&sig_mask);
    for (;;){
        sigsuspend(&sig_mask);
    }
}


main(int argc, char** argv )
{
    char **argvp;
    int i, pid, cmd_pipe[2], n;
    FILE *fp;

    if(argc < 2){ 
	fprintf(stderr, "Usage: pmc_profiler <prog> <args>\n");
	exit(1);
    }

    for(i=1; i<N; i++){
       if((pid = fork()) == 0) do_child(i);
       else if(pid < 0) perror("Fork Error");
            else pid_record[i] = pid;
    }

    do_parent();

    if(pipe(cmd_pipe) < 0) perror("Command Pipe Error");
    if((pid = fork()) < 0)
	perror("Fork Error");
    else if(pid > 0) {
	app_pid = pid; /* remember the application process id */
	close(cmd_pipe[1]); /* close write end */
        do_parent_again();
        //sleep(1);
        for(;;){
            if((n = read(cmd_pipe[0], buf, 18)) > 0) {
		if(n == 18) buf[18] = 0;
                if (strcmp(buf, "PMC_CMD: read_pmc\n") == 0) kill(0, SIGUSR1);
                else printf("%s", buf);
	    } else if(app_finished) {
		//sleep(1);
		kill(0, SIGUSR2);
		break;
	    }
	    
    }
	while(1) sigsuspend(&sig_mask);
    } else {
        close(cmd_pipe[0]); /* close read end */
	if(cmd_pipe[1] != STDOUT_FILENO) {
	    if(dup2(cmd_pipe[1], PIPE_FILENO) != PIPE_FILENO)
		perror("dup2 error");
	    close(cmd_pipe[1]);
	}
        //fprintf(stderr, "argc=%d argv[1]=%s\n", argc, argv[1]);
        argvp = malloc((argc-1) * sizeof (char *));
        for(i=1; i<argc; i++) argvp[i-1] = argv[i];
        if(execvp(argv[1], argvp) < 0) perror(NULL);
    }

}
