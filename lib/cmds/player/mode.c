/* Do not remove the headers from this file! see /USAGE for more info. */

//  Yaynu@Lima Bean

//: PLAYERCOMMAND
//$$see: color, colours, simplify
// USAGE mode
//      mode plain
//      mode vt100
//      mode xterm
//      mode ansi
//
// Typing "mode" will show if your character is set to receive ansi codes
// from the mud(used to display colours).

inherit CMD;
inherit M_ANSI;

private
void main(string arg)
{
  if (!arg)
  {
    string ansistate;
    if (terminal_mode())
      ansistate = terminal_mode();
    else
      ansistate = "plain";

    out("Terminal mode is '" + ansistate + "'.\n");
    return;
  }

  if (arg == "vt100" || arg == "ansi" || arg == "xterm")
  {
    this_user()->query_shell_ob()->set_variable("mode", arg);
    out("Setting mode to '" + arg + "'.\n");
  }
  else if (arg == "off" || arg == "plain")
  {
    this_user()->query_shell_ob()->unset_variable("mode");
    out("Setting mode to 'plain'.\n");
  }
  else
  {
    out("Usage: mode <plain|vt100|ansi|xterm>.\n");
    return;
  }
  this_user()->update_translations();
}

void player_menu_entry(string str)
{
  bare_init();
  main(str);
  done_outputing();
}