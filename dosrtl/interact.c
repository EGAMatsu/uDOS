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
    char ch;
    ch = RtlDoSvc(90, 0, 0, 0);

    for(size_t i = 0; i < 0xFFFFFFFF; i++) {};

    return (int)ch;
}

void RtlPromptInputSimple(
    char *str,
    size_t n)
{
    size_t i = 0;
    int ch = '\0';

    ch = RtlPromptInputChar();
    while(i != n) {
        while(ch == '\0') {
            ch = RtlPromptInputChar();
        }

        if(ch == '\n' || ch == '\r') {
            break;
        }

        str[i++] = ch;
        ch = RtlPromptInputChar();
    }
    str[i] = '\0';
    return;
}