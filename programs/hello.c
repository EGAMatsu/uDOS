/* hello.c
 * 
 * Hello world program example
 */

#include <rtl.h>

int test(int a, int b) {
    return a + b;
}

int PgMain(
    ExecParams *exec)
{
    RtlDebugPrint("SAMPLE.000 - Hello World!\r\n");
    RtlDebugPrint("Hello World! :)\r\n");
    return 0;
}
