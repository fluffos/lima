/* Do not remove the headers from this file! see /USAGE for more info. */

/* %^RESET%^ gets used early in the login sequence by tell() */
private
nosave mapping translations = (["RESET":""]);
private
mapping colours;

void save_me();
object query_shell_ob();

int screen_width;

void set_screen_width(int width)
{
    screen_width = width;
    this_user()->save_me();
}

int query_screen_width()
{
    return screen_width ? screen_width : 79;
}

mapping query_translations()
{
    return translations;
}

string terminal_mode()
{
    string mode = query_shell_ob()->get_variable("mode");
    if (!mode)
        return "plain";
    return mode;
}

void update_translations()
{
    /* handle new people, old save files, or save files for which this
     * option was disabled */
    if (!colours)
        colours = ([]);
    /* defaults; we take advantage of the fact that mapping addition
     * chooses the second when there is an overlap.
     */
    colours = ANSI_D->defaults() + colours;
    if (terminal_mode() != "plain")
        translations = ANSI_D->query_translations()[0];
    else
        translations = ANSI_D->query_translations()[1];
    translations = copy(translations);
    foreach (string code, string value in colours)
    {
        string *parts = map(explode(value, ","), (
                                                     : upper_case:));
        string val = "";

        foreach (string item in parts)
        {
            if (translations[item])
                val += translations[item];
        }
        translations[code] = val;
    }
}

void set_colour(string which, string what)
{
    colours[upper_case(which)] = what;
    update_translations();
    save_me();
}

void query_colour(string which)
{
    return colours[which];
}

mixed *query_colours()
{
    return keys(colours);
}

void remove_colour(string which)
{
    map_delete(colours, upper_case(which));
    /* just in case */
    map_delete(colours, lower_case(which));
    map_delete(colours, which);
    update_translations();
    save_me();
}

void do_receive(string msg, int msg_type)
{
    int xterm = colourp(msg);

    if (msg_type & NO_ANSI)
    {
        if (msg_type & NO_WRAP)
        {
            receive(msg);
        }
        else
        {
            receive(wrap(msg, query_screen_width()));
        }
    }
    else
    {
        int indent = (msg_type & MSG_INDENT) ? 4 : 0;
        int wrap = (msg_type & NO_WRAP) ? 0 : query_screen_width();
        string wrapped = msg;

        // Only do XTERM parsing if needed.
        if (xterm)
        {
            wrapped = XTERM256_D->xterm256_wrap(msg + "<res>", wrap, indent);
            wrapped = XTERM256_D->substitute_colour(wrapped, terminal_mode());
        }
        else
            wrapped = terminal_colour(wrapped + "%^RESET%^", translations, wrap, indent);

        receive(wrapped);
    }
}

void receive_inside_msg(string msg, object *exclude, int message_type,
                        mixed other)
{
    do_receive(msg, message_type);
}

void receive_outside_msg(string msg, object *exclude, int message_type,
                         mixed other)
{
    do_receive(msg, message_type);
}

void receive_remote_msg(string msg, object *exclude, int message_type,
                        mixed other)
{
    do_receive(msg, message_type);
}

void receive_private_msg(string msg, int message_type, mixed other)
{
    do_receive(msg, message_type);
}
