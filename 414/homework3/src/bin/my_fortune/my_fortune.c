/***************************************************************** 
 * Student: Jason Rice
 * 
 * Course: 605.414 System Development in a Unix Environment
 * 
 * File: my_fortune.c
 *
 * Description:
 *    This program is a random fortune generator that demonstates
 * the application of pipes by applying the popen() and pclose()
 * functions and implements nonblocking IO. To exit type 'q' or
 * initiate a SIGINT (ctrl+c) and this will be caught and the
 * nonblocking flags reset before the process terminates (exit).
 ****************************************************************/
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include "log_mgr.h"
#include <math.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define QUITCHAR 'q'

const int MAX = 256;

int OrigFlags = 0;

void strupper(char * str){
	int i = 0;
	while(str[i]){
		str[i] = toupper(str[i]);
		i++;
	}
}

void read_data (FILE * stream){ 
	char buffer[MAX];

	printf("%s","\n");
	log_event(INFO,"%s","Printing fortune:\n");
	while (fgets(buffer, MAX, stream) != NULL){
		strupper(buffer);
		printf("%s", buffer);
		log_event(INFO,"%s",buffer);
	}
	printf("%s","\n");
	log_event(INFO,"%s","*****************************************************\n");
	
	if (ferror (stream)){
		const char * errmsg = "Reading from stream failed\n";
		fprintf (stderr, "%s", errmsg);
		log_event(FATAL, "%s", errmsg);
		exit(EXIT_FAILURE);
	}
}

int run(const char * hostname, FILE * stream){
	if (strcmp(hostname, "pchan") == 0){
		stream = popen ("/home/unix_dev/NOTES/fortune_pchan/fortune", "r");
	}
	else if (strcmp(hostname, "dev3") == 0){
		stream = popen ("/home/unix_dev/NOTES/fortune/fortune", "r");
	}
	else {
		log_event(FATAL, "my_fortune:run() The following host name is unknown: '%s' Exiting program.", hostname);
		exit(EXIT_FAILURE);
	}

	if (!stream){
		log_event(FATAL, "my_fortune:run() Incorrect parameters or too many files.\n");
		exit(EXIT_FAILURE);
	}

	read_data(stream);

	// Close the pipe with the input (read) stream.
	if (pclose (stream) != 0){
		log_event(FATAL,"my_fortune:run() Error closing the pipe stream.\n");
		exit(EXIT_FAILURE);
	}
}

int setnonblocking(){
	log_event(INFO, "my_fortune:setnonblock() Setting up non-blocking I/O");
	int readrtn    = 0;
	int return_val = 0;
	int origflags  = 0;
	int newflags   = 0;

	// This should never fail but just in case handle it by exiting.
	if ((origflags = fcntl(0, F_GETFL, 0)) < 0){
		perror("my_fortune:setnonblock() fcntl F_GETFL errlr");
		log_event(FATAL, "my_fortune:setnonblock() fcntl F_GETFL failed. Preempting program.");
		exit(EXIT_FAILURE);
	}

	// Set the nonblocking flag to a new flag bit map (int).
	newflags = origflags|O_NONBLOCK;

	// Set up nonblocking I/O on STDIN and ensure original flags persist.
	if ((return_val = fcntl (0, F_SETFL, newflags)) < 0){
		perror ("my_fortune:setnonblock() fcntl");
		log_event(FATAL, "my_fortune:setnonblock() fcntl returned -1");
		exit(EXIT_FAILURE);
	}
	log_event(INFO, 
				 "my_fortune:setnonblock() Finished setting up non-blocking I/O. origflags -> '%d'", origflags);

	OrigFlags = origflags;
	return origflags;
}

void resetflags(const int flags){
	log_event(INFO, "my_fortune:resetflags() BEGIN");
	if (fcntl (0, F_SETFL, flags) < 0){
		perror ("my_fortune:resetflags() Flag reset failed.");
		log_event(FATAL, "my_fortune:resetflags() fcntl returned -1. Flag reset failed.");
	}
	else {
		log_event(INFO, "my_fortune:resetflags() Reset IO flags to %d",flags);
	}
	log_event(INFO, "my_fortune:resetflags() END");
}

const int checkquit(const int quitFlagSet, const int origflags){
	if (quitFlagSet){
		log_event(INFO, "my_fortune:checkquit() Quit flag set. QUITCHAR is: '%c'", QUITCHAR);
		log_event(INFO, "my_fortune:checkquit() Reseting STDIN flags");
		resetflags(origflags);
		/* return 1 for true (found quit char) */
		return 1;
	}
	return 0;
}

const char querystdin(int origFlags){
	log_event(INFO, "my_fortune:querystdin() Querying STDIN BEGIN");
	char c;
	int readrtn = 0;
	int quitFlagSet = 0;
	int i = 0;
	
	// See if there's anything on STDIN
   for (;;++i) {
		c = '\0';
		if ((readrtn = read(0, &c, 1)) < 0){
			if (errno == EWOULDBLOCK){
				log_event(INFO,"my_fortune:querystdin() read would have blocked\n");
				if (checkquit(quitFlagSet, origFlags)){
					return 1;
				}
				return -1;
			}
			// Reset flags since we know that the read failed and the program is going to exit.
			resetflags(origFlags);

			// The normal way to q uite is to type 'q' User unable to do this because the read
			// failed from STDIN. Best option at this point to be safe is to preempt process.
			log_event(FATAL, "my_fortune:querystdin() Error reading from STDIN.");
			exit(EXIT_FAILURE);
		}
		// The buffer is flushed if it returns a 0 (e.g. EOF)
		else if (readrtn == 0){
			log_event(INFO,"my_fortune:querystdin() EOF upon read\n");
			if (checkquit(quitFlagSet,origFlags)){
				log_event(INFO, "my_fortune:querystdin() Querying STDIN END");
				return 1;
			}
			break;
		}
		else {
			// If 'q' is the first item in the buffer then raise the quit flag.
			if (i == 0 && QUITCHAR == c){
				quitFlagSet = 1; 
			}
			log_event(INFO, "my_fortune:querystdin() Found the following on STDIN '%c'", c);
		}
	}
	log_event(INFO, "my_fortune:querystdin() Querying STDIN END");
	return 0;
}

void sighandle(int signo){
	log_event(INFO, "my_fortune:sighandle() Signal handler method BEGIN");
	if (signo == SIGINT){
		log_event(INFO, "my_fortune:sighandle() Resetting the blocking IO flags to original state");
		// Reset flags to original flags on STDIN
		resetflags(OrigFlags);
		log_event(INFO, "my_fortune:sighandle() Signal handler method END - Exiting program");
		exit(EXIT_SUCCESS);
	}
	else {
		log_event(WARNING, 
					 "my_fortune:sighandle() Only registered SIGINT signal. Instead found sig: %d", signo);
	}
}

void asleep(unsigned int secs){
	log_event(INFO, "my_fortune:asleep() Sleeping for '%d' seconds", secs);
	sleep(secs);
}

void registersighandle(void (*handle)(int)){
	log_event(INFO, "my_fortune:registersighandle() Registering SIGINT BEGIN");
	if (signal(SIGINT, handle) == SIG_ERR){
		log_event(WARNING, "my_fortune:registersighandle() Unable to handle SIGINT Registration failed.");
	}
	else{
		log_event(INFO, "my_fortune:registersighandle() SIGINT registered.");
	}
	log_event(INFO, "my_fortune:registersighandle() Registering SIGINT END");
}

int main(){
	set_logfile("my_fortune.log");
	log_event(INFO, "my_fortune:main() Fortune Generator: BEGIN");
	FILE *stream;

	char hostname[MAX];
	gethostname(hostname, sizeof(hostname));
	log_event(INFO, "my_fortune:main() Host name is: '%s'", hostname);

	// There is a small chance that a signal could be received 
   // before the signal handler is installed.  The signal 
   // handler should be set prior to calling non-blocking.
	registersighandle(&sighandle);
	int origflags = setnonblocking();	

	// Each call to run opens and closes a pipe through the popen and pclose functions.
	// To quite type 'q' and enter (carriage return) or ctrl+c (SIGINT). Enforces non-blocking I/O
	do {
		run(hostname, stream);
		asleep(rand() % 15 + 1);
	} 	while(querystdin(origflags) <= 0);

	log_event(INFO, "my_fortune:main() Fortune Generator: END");
	close_logfile();
	exit(EXIT_SUCCESS);
}
