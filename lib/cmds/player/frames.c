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
    return "Valid arguments:\n\n'styles' to see other style options.\n" +
           "'style <style>' to see other options.\n" +
           "'themes' to list the colour themes.\n" +
           "'theme <theme>' to select a colour theme.\n" +
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
        object demoframe = new (FRAME);
        demoframe->set_title("Frame settings");
        demoframe->set_header_content("<bld>Frame style is:<res> " + (usertheme || "ascii") + ".\n" +
                                      "<bld>Frame theme is:<res> " + (usercoltheme || "none") + ".\n"+
                                      (i_simplify() ? "Frames are off, since you have simplify on.\n": ""));

        demoframe->set_content(frames_help());
        demoframe->set_footer_content("<bld>These settings will change most UI on "+mud_name()+" to this style.");
        out(demoframe->render());
        return;
    }

    else if (sscanf(arg, "style %s", col) == 1)
    {
        if (member_array(col, themes) != -1)
        {
            this_user()->query_shell_ob()->set_variable("frames", col);
            out("Frame style set to '" + col + "'.\n");
        }
        else
            out("Unknown frame style.\n");
    }
    else if (sscanf(arg, "theme %s", col) == 1)
    {
        if (member_array(col, colours) != -1)
        {
            this_user()->query_shell_ob()->set_variable("frame_colour", col);
            out("Frame colours set to '" + col + "'.\n");
            return;
        }
        else
            out("Unknown theme colours.\n");
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
    else if (arg == "styles")
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