/* interact.c
 *
 * Obtain input or display output to the user in an interactive manner (one
 * which will require user input)
 */

#include <interact.h>
#include <krnl32.h>
#include <stdint.h>

int RtlPromptInputChar(
    void)
{
    unsigned char ch;
    RtlDoSvc(90, &ch, 1, 0);
    return (int)ch;
}

void RtlPromptInputString(
    char *str,
    size_t n)
{
    RtlDoSvc(90, str, n, 0);
    return;
}
