#include <libc.h>

static int   open_safe(const char *filepath, int flags, mode_t mode);
static int   fstat_safe(int fd, struct stat *buf);
static int   ftruncate_safe(int fd, off_t len);
static void *mmap_safe(void *addr, size_t len, int prot, int flags, int fd,
		       off_t offset);
static int   msync_safe(void *addr, size_t len, int flags);

/**
 * Error-checked wrapper for open(). No good way to pass forwarding variadic
 * arguments used by open(), so pass 0 for mode if unneeded.
 */
static int open_safe(const char *filepath, int flags, mode_t mode) {
	int fd = open(filepath, flags, mode);

	if (fd == -1) {
		perror("open");
		exit(EXIT_FAILURE);
	}

	return fd;
}

/**
 * Error-checked wrapper for fstat().
 */
static int fstat_safe(int fd, struct stat *buf) {
	int status = fstat(fd, buf);

	if (status == -1) {
		perror("fstat");
		exit(EXIT_FAILURE);
	}

	return status;
}

/**
 * Error-checked wrapper for ftruncate()
 */
static int ftruncate_safe(int fd, off_t len) {
	int status = ftruncate(fd, len);

	if (status == -1) {
		perror("ftruncate");
		exit(EXIT_FAILURE);
	}

	return status;
}

/**
 * Error-checked wrapper for mmap()
 */
static void *mmap_safe(void *addr, size_t len, int prot, int flags, int fd,
		       off_t offset) {
	void *map_addr = mmap(addr, len, prot, flags, fd, offset);

	if (map_addr == MAP_FAILED) {
		perror("mmap");
		exit(EXIT_FAILURE);
	}

	return map_addr;
}

/**
 * Error-checked wrapper for msync()
 */
static int msync_safe(void *addr, size_t len, int flags) {
	int status = msync(addr, len, flags);

	if (status == -1) {
		perror("msync");
		exit(EXIT_FAILURE);
	}

	return status;
}

int main(int argc, char *argv[]) {
	if (argc < 3) {
		fprintf(stderr, "%s: <source path> <destination path>\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}

	/* Get args*/
	char *src_path = argv[1];
	char *dst_path = argv[2];

	/* Open src fd*/
	int src_fd = open_safe(src_path, O_RDONLY, 0);

	/* Read src file size and permissions (mode) */
	struct stat st;
	fstat_safe(src_fd, &st);
	off_t  size = st.st_size;
	mode_t mode = st.st_mode;

	/* Open dst fd and fit to size */
	int dst_fd =
	    open_safe(dst_path, O_RDWR | O_CREAT,
		      mode); /* create if necessary, copy src permissions */
	ftruncate_safe(dst_fd, size);

	/* Map to memory */
	void *src_addr =
	    mmap_safe(NULL, size, PROT_READ, MAP_PRIVATE, src_fd, 0);
	void *dst_addr =
	    mmap_safe(NULL, size, PROT_WRITE, MAP_SHARED, dst_fd, 0);

	/* Copy and sync */
	memcpy(dst_addr, src_addr, size);
	msync_safe(dst_addr, size, MS_SYNC);

	exit(EXIT_SUCCESS);
}
