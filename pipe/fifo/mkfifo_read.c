#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
	int fd, nread;
	char buffer[1024] = {0};
	char *fifo_name = "/tmp/myfifo";

	umask(0);
	if (mkfifo(fifo_name, O_RDWR) == -1) {
		perror("mkfifo() error");
		return 0;
	}

	fd = open(fifo_name, 0644);
	if (fd == -1) {
		perror("open() error");
		return 0;
	}

	while (1)
	{
		memset(buffer, 0, sizeof(buffer));
		nread = read(fd, buffer, sizeof(buffer));
		if (nread == -1) {
			perror("read() error");
		} else if (nread == 0) {
			printf("read() finished.\n");
			return 0;
		}
		printf("%s\n", buffer);
	}
	close(fd);
	remove(fifo_name);

	return 0;
}
