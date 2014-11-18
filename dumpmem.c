
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>

#include <stdlib.h>

void
hexdump( char *address, char *buffer, long length ) {
    while ( length > 0 ) {
        printf( "%p  %08x %08x %08x %08x  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n",
	        address,
		*( (long*)buffer ),
		*( (long*)(buffer+4) ),
		*( (long*)(buffer+8) ),
		*( (long*)(buffer+12) ),
		isprint( *(buffer +  0) ) ? *(buffer +  0) : '.',
		isprint( *(buffer +  1) ) ? *(buffer +  1) : '.',
		isprint( *(buffer +  2) ) ? *(buffer +  2) : '.',
		isprint( *(buffer +  3) ) ? *(buffer +  3) : '.',
		isprint( *(buffer +  4) ) ? *(buffer +  4) : '.',
		isprint( *(buffer +  5) ) ? *(buffer +  5) : '.',
		isprint( *(buffer +  6) ) ? *(buffer +  6) : '.',
		isprint( *(buffer +  7) ) ? *(buffer +  7) : '.',
		isprint( *(buffer +  8) ) ? *(buffer +  8) : '.',
		isprint( *(buffer +  9) ) ? *(buffer +  9) : '.',
		isprint( *(buffer + 10) ) ? *(buffer + 10) : '.',
		isprint( *(buffer + 11) ) ? *(buffer + 11) : '.',
		isprint( *(buffer + 12) ) ? *(buffer + 12) : '.',
		isprint( *(buffer + 13) ) ? *(buffer + 13) : '.',
		isprint( *(buffer + 14) ) ? *(buffer + 14) : '.',
		isprint( *(buffer + 15) ) ? *(buffer + 15) : '.'  );
        address += 16;
        buffer += 16;
	length -= 16;
    }
}

int main( int argc, char **argv ) {
    pid_t pid;
    char *start;
    long length;
    int fd;
    ssize_t size_read;
    off_t seeked_to;
    int wait_status;

    char filename[80];
    char *buffer;

    pid_t wait_pid;

    if ( argc < 3 ) {
        fprintf( stderr, "Usage: dumpmem <pid> <start> <length>\n" );
	_exit( 1 );
    }

    sscanf( argv[1], "%d", &pid );
    sscanf( argv[2], "%p", &start );
    sscanf( argv[3], "%x", &length );
    printf( "Dumping pid %d [%p(%d)]\n", pid, start, length );

    buffer = malloc( length );
    if ( buffer == NULL ) {
        fprintf( stderr, "memory allocation error\n" );
	_exit( ENOMEM );
    }

    ptrace( PTRACE_ATTACH, pid, 0, 0 );

    wait_pid = wait(&wait_status);
    if ( wait_pid == -1 ) {
        perror("waitpid");
	_exit( 0 );
    }
    if ( wait_pid != pid ) {
        fprintf( stderr, "didn't wait for %d\n", pid );
    }

    sprintf( filename, "/proc/%d/mem", pid );
    fd = open( filename, O_RDONLY );
    if ( fd < 0 ) {
        perror("open");
        ptrace( PTRACE_DETACH, pid, 0, 0 );
	_exit( 0 );
    }

    seeked_to = lseek( fd, (off_t)start, SEEK_SET );
    if ( seeked_to == (off_t)-1 ) {
        perror("lseek");
	goto finish;
    }

    size_read = read( fd, buffer, length );
    if ( size_read == -1 ) {
        perror("read");
	goto finish;
    }
    if ( size_read != length ) { 
        fprintf( stderr, "expected to read %d bytes, but read %d bytes\n", length, size_read );
	_exit( 1 );
    }

    hexdump( start, buffer, length );

finish:
    close( fd );
    ptrace( PTRACE_DETACH, pid, 0, 0 );

    return 0;
}

/*
 * vim:autoindent
 */
