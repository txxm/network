#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
	int fd, nwrite;
	char buffer[1024] = {0};
	char *fifo_name = "/tmp/myfifo";

	fd = open(fifo_name, O_RDWR);
   	if (fd == -1) {
		perror("open() error");
		return 0;
	}

	while (1)
	{
		nwrite = write(fd, "writing is doing...", 20);
		if (nwrite == -1) {
			perror("write() error");
		}
		sleep(1);
	}
	close(fd);

	return 0;
}
