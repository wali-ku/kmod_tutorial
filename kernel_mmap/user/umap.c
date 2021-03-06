#include "ku_map.h"

int main(void)
{
	int dev_fd = 0;
	void *shm_area = NULL;
	struct user_info *uinfo = NULL;

	dev_fd = open(DEV_FILE, O_RDONLY);
	if (dev_fd <= 0) {
		printf("Failed to open device file\n");
		return -1;
	}

	shm_area = mmap(NULL, MMAP_SIZE, PROT_READ | PROT_WRITE,
			MAP_PRIVATE, dev_fd, 0);

	if ((long)shm_area <= 0) {
		printf("Failed to mmap device file\n");
		close(dev_fd);
		return -2;
	}

	uinfo = (struct user_info *)shm_area;
	uinfo->budget = 20;

	munmap(shm_area, MMAP_SIZE);
	close(dev_fd);

	return 0;
}
