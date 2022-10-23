#include <daemons.h>

//:FUNCTION colourp
// colourp returns 0 or 1 if it contains any xterm colour codes
int colourp(string text)
{
    return XTERM256_D->colorp(text) ;
}

//:FUNCTION colorp
// colorp returns 0 or 1 if it contains any xterm colour codes
int colorp(string text)
{
    return colourp(text) ;
}
