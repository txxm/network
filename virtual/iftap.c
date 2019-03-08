#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if_tun.h>

int set_virt_network_attribute(char network_name[],
				char *addr, char *netmask)
{
	int ret;
	int sockfd;
	struct ifreq ifr;
	struct sockaddr_in virt_addr;

	if (strlen(network_name) > IFNAMSIZ || network_name == NULL ||
				addr == NULL || netmask == NULL)
	{
		printf("please input correct parm.\n");
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	memset(&virt_addr, 0, sizeof(virt_addr));

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd == -1) {
		return -1;
	}
	virt_addr.sin_family = AF_INET;
	inet_pton(AF_INET, addr, &virt_addr.sin_addr);

	memcpy(ifr.ifr_name, network_name, IFNAMSIZ);
	memcpy(&ifr.ifr_addr, &virt_addr, sizeof(virt_addr));
	ret = ioctl(sockfd, SIOCSIFADDR, (void *)&ifr);
	if (ret == -1) {
		printf("Create virtual address fail.\n");
		close(sockfd);
		return -1;
	}

	ret = ioctl(sockfd, SIOCGIFFLAGS, (void *)&ifr);
	if (ret == -1) {
		close(sockfd);
		return -1;
	}
	ifr.ifr_flags |= IFF_UP;
	ret = ioctl(sockfd, SIOCSIFFLAGS, (void *)&ifr);
	if (ret == -1) {
		printf("Setup virtual-network address fail.\n");
		close(sockfd);
		return -1;
	}

	inet_pton(AF_INET, netmask, &virt_addr.sin_addr);
	memcpy(&ifr.ifr_addr, &virt_addr, sizeof(virt_addr));
	ret = ioctl(sockfd, SIOCSIFNETMASK, (void *)&ifr);
	if (ret == -1) {
		printf("Set virtual-network netmask fail.\n");
		close(sockfd);
		return -1;
	}

	return 0;
}

int create_virt_network(char network_name[])
{
	int ret, fd;
	struct ifreq ifr;
	if (strlen(network_name) > IFNAMSIZ || network_name == NULL) {
		printf("please input correct parm.\n");
		return -1;
	}

	fd = open("/dev/net/tun", O_RDWR);
	if (fd == -1) {
		return -1;
	}

	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags |= IFF_TAP | IFF_NO_PI;
	if (*network_name) {
		memcpy(ifr.ifr_name, network_name, IFNAMSIZ);
	}

	ret = ioctl(fd, TUNSETIFF, (void *)&ifr);
	if (ret == -1)
	{
		close(fd);
		return -1;
	}
	memcpy(network_name, ifr.ifr_name, IFNAMSIZ);

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;
	char network_name[IFNAMSIZ];

	if (argc == 3)
	{
		memset(network_name, 0, IFNAMSIZ);
		ret = create_virt_network(network_name);
		if (ret == -1) {
			return -1;
		}

		ret = set_virt_network_attribute(network_name, argv[1], argv[2]);
		if (ret == -1) {
			return -1;
		}
	}

	while (1){;};

	return 0;
}
