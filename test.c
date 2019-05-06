#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main( int argc, char* const* argv) {
	pid_t pid = fork();
	if (pid == 0) {
		execvp("./program", argv);
		return 1;
	}
	
	char command[40];
	sprintf(command, "top -b -p %d -d 1 > log%s.txt", pid, argv[1]);
	if (fork() == 0)
		execl("/bin/sh", "sh", "-c", command, NULL);
	
	int status;
	waitpid(pid, &status, 0);

	signal(SIGTERM, SIG_IGN);
	kill(0, SIGTERM);
	
	return 0;
}