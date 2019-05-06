#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

FILE* output;
void cprintf(const char* format, ...){
	va_list args;
	va_start(args, format);

	if (output != NULL)
		vfprintf(output, format, args);

	va_start(args, format);
	vprintf(format, args);
	
	va_end(args);
}

typedef struct interrupt {
	bool running;
	int ticks;
	void (*callback)(int);
} interrupt;

struct timeval *samples;
void sampling_func(int index)
{
	int res = gettimeofday(&samples[index], NULL);
	
	cprintf("Tick ");
	cprintf("%ld.", samples[index].tv_sec);
	cprintf("%06ld\n", samples[index].tv_usec);
}

interrupt polling_it;
void polling_handler() {
	static int i = 0;
	i++;
	cprintf("%d: ", i);
	polling_it.callback(i-1);
	if (i >= polling_it.ticks) {
		polling_it.running = false;
	}
}
void polling_setup(time_t period, suseconds_t interval, void (*callback)(int)) {
	polling_it.running = true;
	polling_it.ticks = period * (1000000.0 / interval);
	polling_it.callback = callback;

	samples = (struct timeval*)malloc(polling_it.ticks * sizeof(struct timeval));

	struct timeval goal;
	goal.tv_usec = interval%1000000;
	goal.tv_sec = interval/1000000;

	struct timeval duration = goal;	
	
	struct timeval start, end, diff;
	int res = gettimeofday(&start, NULL);
	while(polling_it.running) {
		if (duration.tv_usec>0)
			usleep(duration.tv_usec);
		if (duration.tv_sec>0)
			sleep(duration.tv_sec);
		
		polling_handler();

		res = gettimeofday(&end, NULL);
		duration.tv_usec += goal.tv_usec - (end.tv_usec - start.tv_usec);
		duration.tv_sec += goal.tv_sec - (end.tv_sec - start.tv_sec);
		if (duration.tv_usec < 0) {
			if (duration.tv_sec == 0) {
				duration = goal;
			} else {
				duration.tv_usec += 1000000;
				duration.tv_sec -= 1;
			}
		} else if (duration.tv_sec < 0) {
			if (duration.tv_usec > 1000000) {
				duration.tv_usec -= 1000000;
				duration.tv_sec += 1;
			} else {
				duration = goal;
			}
		}
		start = end;
	}
}


interrupt timer_it;
void timer_handler(int sig) {
	static int i = 0;
	i++;
	cprintf("%d: ", i);
	timer_it.callback(i-1);
	if (i >= timer_it.ticks) {
		struct itimerval temp = {0};
		setitimer(ITIMER_REAL, &temp, NULL);
		timer_it.running = false;
	}
}
void timer_setup(time_t period, suseconds_t interval, void (*callback)(int)) {
	timer_it.callback = callback;
	timer_it.ticks = period * (1000000.0 / interval);
	timer_it.running = true;
	
	samples = (struct timeval*)malloc(timer_it.ticks * sizeof(struct timeval));

	signal(SIGALRM, timer_handler);
	
	struct itimerval temp;
	temp.it_interval.tv_usec = interval%1000000;
	temp.it_interval.tv_sec = interval/1000000;
	temp.it_value = temp.it_interval;

	if (setitimer(ITIMER_REAL, &temp, NULL) != 0)
		return;
	
	while(timer_it.running)
		pause();
}

void quit_catch(int sig) {
	cprintf("Terminating...\n");
	exit(0);
}

enum modes {timer_mode, polling_mode};
int main( int argc, const char* argv[] ) {
	if (argc != 4){
		cprintf("Set Arguments!\n");
		return 1;
	}
	signal(SIGINT, quit_catch);

	cprintf("Starting...\n");

	int mode = atoi(argv[1]);
	float period = atof(argv[2]);
	float dt = atof(argv[3]);
	useconds_t interval = (useconds_t) 1000000.0 * dt;
	
	char name[10];
	sprintf(name, "time%d.txt", mode);
	output = fopen(name, "w");
	
	switch(mode) {
		case timer_mode:
			timer_setup((time_t) period, interval, sampling_func);
			break;
		case polling_mode:
			polling_setup((time_t) period, interval, sampling_func);
			break;
		default:
			cprintf("Wrong Mode!\n");
			break;
	}

	fclose(output);
	output = NULL;
	
	cprintf("Ending...\n");
	
	if (samples != NULL)
		free(samples);
	
	return 0;
}
