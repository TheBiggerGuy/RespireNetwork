#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>

#include "efm32.h"

#undef errno
extern int errno;

#define HEAP_SIZE 1024

#define STDOUT 1
#define STDIN  2
#define STDERR 3

unsigned char heap[HEAP_SIZE];

/**
 * via: http://e2e.ti.com/support/microcontrollers/stellaris_arm_cortex-m3_microcontroller/f/473/t/44452.aspx
 * TODO: Need to check this works !!!
 */
// get current stack pointer

/**
 * Exit a program without cleaning up files.
 * If your system doesn't provide this, it is best to avoid linking with subroutines that require it (exit, system).
 */
void _exit(int code) {
	NVIC_SystemReset();
}

/**
 * Close a file.
 * Minimal implementation
 */
int _close(int file) {
	return -1;
}

/**
 * A pointer to a list of environment variables and their values.
 * For a minimal environment, this empty list is adequate
 */
char *__env[1] = { 0 };
char **environ = __env;

/**
 * Transfer control to a new process.
 * Minimal implementation (for a system without processes)
 */
int _execve(char *name, char **argv, char **env) {
	errno = ENOMEM;
	return -1;
}

/**
 * Create a new process.
 * Minimal implementation (for a system without processes)
 */
int _fork(void) {
	errno = EAGAIN;
	return -1;
}

/**
 * Status of an open file.
 * For consistency with other minimal implementations in these examples, all files are regarded as character special devices.
 * The sys/stat.h header file required is distributed in the include subdirectory for this C library.
 */
int _fstat(int file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}

/**
 * Process-ID; this is sometimes used to generate strings unlikely to conflict with other processes.
 * Minimal implementation, for a system without processes
 */
int _getpid(void) {
	return 1;
}

/**
 * Query whether output stream is a terminal.
 * For consistency with the other minimal implementations, which only support output to stdout, this minimal implementation is suggested
 */
int _isatty(int file) {
	return 1;
}
/**
 * Send a signal.
 * Minimal implementation
 */
int _kill(int pid, int sig) {
	errno = EINVAL;
	return -1;
}

/**
 * Establish a new name for an existing file.
 * Minimal implementation
 */
int _link(char *old, char *new) {
	errno = EMLINK;
	return -1;
}

/**
 * Set position in a file.
 * Minimal implementation
 */
int _lseek(int file, int ptr, int dir) {
	return 0;
}

/**
 * Open a file.
 * Minimal implementation
 */

int _open(const char *name, int flags, int mode) {
	return -1;
}

/**
 * Read from a file.
 * Minimal implementation
 */
int _read(int file, char *ptr, int len) {
	return 0;
}

/**
 * Increase program data space.
 * As malloc and related functions depend on this, it is useful to have a working implementation.
 * The following suffices for a standalone system.
 */
caddr_t _sbrk(int incr) {
	static unsigned char *heap_end;
	unsigned char *prev_heap_end;

	if (heap_end == 0) {
		heap_end = heap;
	}
	prev_heap_end = heap_end;
	if (heap_end + incr - heap > HEAP_SIZE) {
		write(STDOUT, "Heap overflow\n", 14);
		abort();
	}

	heap_end += incr;
	return (caddr_t) prev_heap_end;
}

///**
// * Increase program data space.
// * As malloc and related functions depend on this, it is useful to have a working implementation.
// * The following suffices for a standalone system.
// */
//caddr_t _sbrk(int incr) {
//	extern char heap_low; /* Defined by the linker */
//	extern char heap_top; /* Defined by the linker */
//	char *prev_heap_end;
//	char *heap_end = 0;
//
//	if (heap_end == 0) {
//		heap_end = &heap_low;
//	}
//	prev_heap_end = heap_end;
//	if (heap_end + incr > &heap_top) {
//		/* Heap and stack collision */
//		write(STDOUT, "Heap overflow\n", 14);
//		abort();
//	}
//	heap_end += incr;
//
//	return (caddr_t) prev_heap_end;
//}


/**
 * Status of a file (by name).
 * Minimal implementation
 */
int _stat(char *file, struct stat *st) {
	st->st_mode = S_IFCHR;
	return 0;
}
/**
 * Timing information for current process.
 * Minimal implementation
 */
int _times(struct tms *buf) {
	return SysTick->VAL;
}

/**
 * Remove a file's directory entry.
 * Minimal implementation
 */
int _unlink(char *name) {
	errno = ENOENT;
	return -1;
}

/**
 * Wait for a child process.
 * Minimal implementation
 */
int _wait(int *status) {
	errno = ECHILD;
	return -1;
}

/**
 * Write to a file.
 * libc subroutines will use this system routine for output to all files, including stdout—so if you need to generate any output, for example to a serial port for debugging, you should make your minimal write capable of doing this.
 * The following minimal implementation is an incomplete example; it relies on a outbyte subroutine (not shown; typically, you must write this in assembler from examples provided by your hardware manufacturer) to actually perform the output.
 */
int _write(int file, char *ptr, int len) {
	int i;

	switch (file) {
	case STDOUT:
		//memcpy(debug_string, ptr, len);
		for (i = 0; i < len; i++) {
			ITM_SendChar(*ptr++);
		}
		return len;
		break;
	default:
		return -1;
	}
}

