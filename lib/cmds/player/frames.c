/* Do not remove the headers from this file! see /USAGE for more info. */

//: PLAYERCOMMAND
//$$ see: color, colours, ansi, emoji
// USAGE frames
//      frames list
//      frames <type>
//
// Use 'frames list' to show which frame themes are supported.
// Use 'frames single' to set your frame theme to 'single'.

inherit CMD;
inherit M_WIDGETS;

private
void main(string arg)
{
    object frame = load_object(FRAME);
    string *themes = frame->query_themes();

    if (!arg || arg == "")
    {
        string simplestate;
        object shell = this_user()->query_shell_ob();
        if (shell)
            out("Frame style is: " + (this_user()->frames_theme() || "ascii") +
                ".\nUse 'frames list' to see other options.\n");

        return;
    }

    if (member_array(arg, themes) != -1)
    {
        this_user()->query_shell_ob()->set_variable("frames", arg);
        out("Frame style set to '" + arg + "'.\n");
    }
    else if (arg == "list")
    {
        printf("The following frame styles are supported:\n\n");
        printf("\t<bld>%-15.15s %s<res>", "Name","Example");

        foreach (string t in themes)
        {
            printf("\t%-15.15s %s\n", t, frame->demo_string(t));
        }
        printf("\n\n");
    }
    else
    {
        out("Usage: frames <style> or list.\n");
        return;
    }
}

void player_menu_entry(string str)
{
    bare_init();
    main(str);
    done_outputing();
}