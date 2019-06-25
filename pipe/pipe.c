#include <stdio.h>
#include <unistd.h>

/* 匿名管道 */
int main(int argc, char *argv[])
{
	pid_t pid;
	int fd[2] = {0};

	if (pipe(fd) == -1) {
		perror("pipe() error");
		return 0;
	}

	pid = fork();
	if (pid == -1) {
		return 0;
	}
	else if (pid == 0)
	{
		close(fd[0]);
		while (1)
		{
			write(fd[1], "child is writing.", 17);
			sleep(1);
		}
	}
	else
	{
		close(fd[1]);
		while (1)
		{
			char buffer[32] = {0};
			int nread = read(fd[0], buffer, 32);
			if (nread == 0) {
				printf("read done. finished\n");
				return 0;
			} else if (nread > 0) {
				printf("%s\n", buffer);
			}
		}
	}

	return 0;
}
