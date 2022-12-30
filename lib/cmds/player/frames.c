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

string frames_help()
{
    return "'frames list' to see other options.\n" +
           "'frames themes' to list the colour themes.\n" +
           "'frames theme <theme>' to select a colour theme.\n" +
           "\n";
}

private
void main(string arg)
{
    object frame = load_object(FRAME);
    string *themes = frame->query_themes();
    string *colours = frame->query_colour_themes();
    string col;
    string usertheme = this_user()->frames_theme();
    string usercoltheme = this_user()->frames_colour();

    if (!arg || arg == "")
    {
        string simplestate;
        out("Frame style is: " + (usertheme || "ascii") + ".\n" +
            "Frame theme is: " + (usercoltheme || "none") + ".\n\n" +
            frames_help());

        return;
    }

    if (member_array(arg, themes) != -1)
    {
        this_user()->query_shell_ob()->set_variable("frames", arg);
        out("Frame style set to '" + arg + "'.\n");
    }
    else if (sscanf(arg, "theme %s", col) == 1)
    {
        if (member_array(col, colours) == -1)
        {
            printf("Unknown theme colours.\n");
            return;
        }
        this_user()->query_shell_ob()->set_variable("frame_colour", col);
        out("Frame colours set to '" + col + "'.\n");
    }
    else if (arg == "themes")
    {
        printf("The following frame themes are supported:\n\n");
        printf("\t<bld>%-15.15s %s<res>", "Name", "Example");

        foreach (string c in colours)
        {
            printf("\t%-15.15s %s\n", c, c != "none" ? frame->colour_demo(usertheme, c) : "");
        }
        printf("\n\n");
    }
    else if (arg == "list")
    {
        printf("The following frame styles are supported:\n\n");
        printf("\t<bld>%-15.15s %s<res>", "Name", "Example");

        foreach (string t in themes)
        {
            printf("\t%-15.15s %s\n", t, frame->demo_string(t));
        }
        printf("\n\n");
    }
    else
    {
        out("Usage:\n" + frames_help() + ".\n");
        return;
    }
}

void player_menu_entry(string str)
{
    bare_init();
    main(str);
    done_outputing();
}