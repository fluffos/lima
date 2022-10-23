// /daemons/XTERM256_D.c
// This daemon handles the 256 Colours of XTERM codes
//
// Created:     2022/04/08: Gesslar
// Last Change: 2022/04/08: Gesslar
//
// 2022/04/08: Gesslar - Created

#include <xterm256.h>

inherit M_DAEMON_DATA ;

private nosave string *fg_codes = ({ }) ;
private nosave string *bg_codes = ({ }) ;
private nosave mapping alt_codes = ([ ]) ;
private nosave string *fallback_codes = ({ }) ;
private nosave mapping x256_to_16_fg = ([ ]) ;
private nosave mapping x256_to_16_bg = ([ ]) ;

private void load_all_colours() ;
public string xterm256_wrap(string str, int wrap_at, int indent_at) ;

protected void create()
{
    ::create() ;
    load_all_colours() ;
}

private void load_all_colours()
{
    int i ;
    string *lines ;

    // First let's load all the 256 colours into the colour codes
    i = 256 ;
    fg_codes = allocate(i) ;
    bg_codes = allocate(i) ;
    while(i--)
    {
        fg_codes[i] = sprintf("%c[38;5;%dm", 27, i) ;
        bg_codes[i] = sprintf("%c[48;5;%dm", 27, i) ;
    }

    alt_codes = ([
        "res" : sprintf("%c[0m",  27), // reset
        "rev" : sprintf("%c[7m",  27), // reverse video
        "it0" : sprintf("%c[23m", 27), // italics off
        "it1" : sprintf("%c[3m",  27), // italics on
        "ul0" : sprintf("%c[24m", 27), // underline off
        "ul1" : sprintf("%c[4m",  27), // underline on
        "fl0" : sprintf("%c[25m", 27), // flash off
        "fl1" : sprintf("%c[5m",  27), // flash on
    ]) ;

    lines = explode(read_file("/etc/xterm256/256_to_16_fallback.txt"), "\n") ;
    i = 256 ;
    fallback_codes = allocate(i) ;
    while(i--)
    {
        string fallback ;

        sscanf(lines[i], "%*s,%s", fallback) ;

        fallback_codes[i] = fallback ;
    }

    lines = explode(read_file("/etc/xterm256/xterm_ansi_16_fg.txt"), "\n") ;
    i = 16 ;
    while(i--)
    {
        string xterm ;
        string fallback ;
        
        sscanf(lines[i], "%s,%s", xterm, fallback) ;
        x256_to_16_fg[xterm] = sprintf("%c[%sm", 27, fallback) ;
    }

    lines = explode(read_file("/etc/xterm256/xterm_ansi_16_bg.txt"), "\n") ;
    i = 16 ;
    while(i--)
    {
        string xterm ;
        string fallback ;
        
        sscanf(lines[i], "%s,%s", xterm, fallback) ;
        x256_to_16_bg[xterm] = sprintf("%c[%sm", 27, fallback) ;
    
    }
}

//:FUNCTION substitute_colour
// substitute_colour takes a string with tokenized xterm256 colour
// codes and a mode, parses the tokens and substitutes with 
// xterm colour codes suitable for printing.
// available modes are: 
// plain - strip all colour and style codes
// vt100 - strip only colour codes
// xterm - replace all tokens with xterm256 colour codes
// ansi  - fall back to ansi colour codes
public varargs string substitute_colour(string text, string mode)
{
    mixed *assoc ;
    string *parts, sub ;
    int *matched ;
    int sz, num ;
    
    if(nullp(text)) return "" ;

    assoc = pcre_assoc(text, ({ XTERM256_COLOURS }), ({ 1 })) ;
    parts = assoc[0] ;
    matched = assoc[1] ;
    sz = sizeof(parts) ;

    switch(mode)
    {
        case "plain" :
            while(sz--)
            {
                // Skip non matches
                if(matched[sz] == 0) continue ;

                parts[sz] = "" ;
            }
            break ;
        case "vt100" :
            while(sz--)
            {
                // Skip non matches
                if(matched[sz] == 0) continue ;

                // First, are we an alt code
                if(sscanf(parts[sz], "<%s>", sub) == 1)
                {
                    if(!nullp(sub = alt_codes[sub]))
                    {
                        parts[sz] = sub ;
                        continue ;
                    }
                }
                
                parts[sz] = "" ;
            }
            break ;
        case "xterm" :
            while(sz--)
            {
                // Skip non matches
                if(matched[sz] == 0) continue ;

                // First, are we an alt code
                if(sscanf(parts[sz], "<%s>", sub) == 1)
                {
                    if(!nullp(sub = alt_codes[sub]))
                    {
                        parts[sz] = sub ;
                        continue ;
                    }
                }

                // Now, we have to be one of the colour codes!
                if(sscanf(parts[sz], "<%d>", num) == 1) parts[sz] = fg_codes[num] ;
                else if(sscanf(parts[sz], "#%d#", num) == 1) parts[sz] = bg_codes[num] ;

            }
            break ;
        case "ansi" :
            while(sz--)
            {
                // Skip non matches
                if(matched[sz] == 0) continue ;

                // First, are we an alt code
                if(sscanf(parts[sz], "<%s>", sub) == 1)
                {
                    if(!nullp(sub = alt_codes[sub]))
                    {
                        parts[sz] = sub ;
                        continue ;
                    }
                }

                // Now, we have to be one of the colour codes!
                if(sscanf(parts[sz], "<%d>", num) == 1) parts[sz] = x256_to_16_fg[fallback_codes[num]] ;
                else if(sscanf(parts[sz], "#%d#", num) == 1) parts[sz] = x256_to_16_bg[fallback_codes[num]] ;
                else parts[sz] = "" ;
            }
            break ;
        default :
            return substitute_colour(text, "plain") ;
    }

    return implode(parts, "") ;
}

public string xterm256_wrap(string str, int wrap_at, int indent_at)
{
    string *parts = explode(str, " ");
    mapping running = ([ "length" : 0 ]);

    // this routine strips out the first space, put it back into the 
    // array if the original string had a leading space
    if(str[0..0] == " ") parts = ({ "", parts... });

    parts = map(parts, function(string part, mapping running, int max, int indent)
    {
        string plain ;
        int len ;

        // new lines
        if(part[0..0] == "\n")
        {
            running["length"] = 0;
            return part ;
        }

        // test for colour codes
        plain = substitute_colour(part, "plain") ;
        // printf("%O\n", plain) ;

        len = sizeof(plain);

        running["length"] += (len + 1);

        if(running["length"] >= max)
        {
            running["length"] = sizeof(plain) + indent;
            return "\n" + repeat_string(" ", indent) + part;
        }

        return part ;

    }, running, wrap_at, indent_at);

    return implode(map(explode(implode(parts, " "), "\n"), (: rtrim :)), "\n") ;
}

int colorp(string text)
{
    return pcre_match(text, XTERM256_COLOURS) ;
}
