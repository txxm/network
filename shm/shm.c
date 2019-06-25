#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/ipc.h>

int main(int argc, char *argv[])
{
	pid_t pid;
	int shm_fd;
	char *p_buffer, *c_buffer;

	shm_fd = shmget(IPC_PRIVATE, 4096, IPC_CREAT);
	if (shm_fd == -1) {
		perror("shmget() error");
		return 0;
	}

	pid = fork();
	if (pid == -1) {
		return 0;
	} else if (pid > 0) {
		p_buffer = shmat(shm_fd, NULL, 0);
		memset(p_buffer, 0, sizeof(p_buffer));
		memcpy(p_buffer, "guoyongxin", 12);
		while(1);
	} else {
		sleep(1);
		c_buffer = shmat(shm_fd, NULL, 0);
		printf("c_buffer=%s\n", c_buffer);
		while(1);
	}

	return 0;
}
