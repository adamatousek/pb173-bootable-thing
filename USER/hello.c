#include <string.h>
#include <masystd.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int main();

void _start() {
    int exitval = main();
    cease( exitval );
}

int main() {
    sysping( 0xC0FFEE );
    sysping( 0xADD1C7ED );
    const char *hello = "\\o/ Hello from the user side! \\o/\n";
    inscribe( 1, hello, strlen( hello ), 0 ); // VGA
    printf( "%s |I must have called 7 syscalls|\n", hello );
    puts( "/ \\    To tell you I'm stdio  / \\" );
    return 0x42;
}

#ifdef __cplusplus
}
#endif

