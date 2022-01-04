/**
  * \file syscalls_stm32.c
  *
  * Implementation of newlib syscall.
  *
  */
#include <stddef.h>
#include "stm32_def.h"
#if defined (  __GNUC__  ) /* GCC CS3 */
  #include <sys/stat.h>
#endif
#include <errno.h>
#undef errno
extern int errno;

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

extern size_t stm32_uart_debug_write(uint8_t *data, uint32_t size);

// Helper macro to mark unused parameters and prevent compiler warnings.
// Appends _UNUSED to the variable name to prevent accidentally using them.
#ifdef UNUSED
  #undef UNUSED
#endif
#ifdef __GNUC__
  #define UNUSED(x) x ## _UNUSED __attribute__((__unused__))
#else
  #define UNUSED(x) x ## _UNUSED
#endif

__attribute__((weak))
caddr_t _sbrk(int incr)
{
  extern char _estack; /* Defined in the linker script */
  extern char _Min_Stack_Size; /* Defined in the linker script */
  extern char _end; /* Defined by the linker */
  static char *heap_end = &_end ;
  char *prev_heap_end = heap_end;

  if (heap_end + incr > (char *)__get_MSP()) {
    /* Heap and stack collision */
    errno = ENOMEM;
    return (caddr_t) -1;
  }
  /* Ensure to keep minimun stack size defined in the linker script */
  if (heap_end + incr >= (char *)(&_estack - &_Min_Stack_Size)) {
    errno = ENOMEM;
    return (caddr_t) -1;
  }

  heap_end += incr ;
  return (caddr_t) prev_heap_end ;
}

__attribute__((weak))
int _close(UNUSED(int file))
{
  return -1;
}

__attribute__((weak))
int _fstat(UNUSED(int file), struct stat *st)
{
  st->st_mode = S_IFCHR ;
  return 0;
}

__attribute__((weak))
int _isatty(UNUSED(int file))
{
  return 1;
}

__attribute__((weak))
int _lseek(UNUSED(int file), UNUSED(int ptr), UNUSED(int dir))
{
  return 0;
}

__attribute__((weak))
int _read (int file, char *ptr, int len)
{
	/* The I/O library uses an internal buffer */
	/* It asks for 1024 characters even if only getc() is used. */
	/* If we use a for(;;) loop on the number of characters requested, */
	/* the user is forced to enter the exact number requested, even if only one is needed. */
	/* So here we return only 1 character even if requested length is > 1 */
	*ptr = __io_getchar();

	return 1;
}

__attribute__((weak))
int _write(int file, char *ptr, int len)
{
	int DataIdx;

	for (DataIdx = 0; DataIdx < len; DataIdx++)
	{
		__io_putchar(*ptr++);
	}
	return len;
}

__attribute__((weak))
void _exit(UNUSED(int status))
{
  for (; ;) ;
}

__attribute__((weak))
int _kill(UNUSED(int pid), UNUSED(int sig))
{
  errno = EINVAL;
  return -1;
}

__attribute__((weak))
int _getpid(void)
{
  return 1;
}
