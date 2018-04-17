#include <string.h>
#include <masystd.h>

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
    inscribe( 0, hello, strlen( hello ), 0 );
    inscribe( 1, hello, strlen( hello ), 0 );
    return 0x42;
}

#ifdef __cplusplus
}
#endif

