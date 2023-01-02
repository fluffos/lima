/* Do not remove the headers from this file! see /USAGE for more info. */

// Rust 2-6-94

//: PLAYERCOMMAND
// USAGE who [ -w -p ]
//
// The who command will give you a list of all user's presently logged
// onto the MUD, and whether or not they are idle.
//
// Used with the given arguments you can alter the way who works:
//
// who -w
// Will show wizards only.
//
// who -p
// Will show players only.

#include <playerflags.h>

inherit CMD;

#define USER_DESC "(PLAYERS ONLY)"
#define WHO_FORMAT "%s:  (Local Time is: %s) %s"

string get_who_string(string arg)
{
  object *u;
  int i;
  string name, extra, retval;
  object frame = new (FRAME);

  extra = retval = "\n";
  if (this_user())
  {
    switch (arg)
    {
    case "-p":
    case "-z":
      u = filter_array(bodies(), (
                                     : !wizardp($1)
                                     :));
      extra = USER_DESC + "\n";
      break;
    case "-w":
    case "-i":
      u = filter_array(bodies(), (
                                     : wizardp:));
      extra = "(IMPLEMENTORS ONLY)\n";
      break;
    case "-m":
      if (wizardp(this_user()))
      {
        u = filter_array(children(USER_OB),
                         (
                             : !interactive($1)
                             :))
                ->query_body();
        extra = "(NON-INTERACTIVES)\n";
        break;
      }
      /* FALLTHRU */
    default:
      if (arg)
        write("Who: Unknown flag '" + arg + "'.\n");
      u = bodies();
      break;
    }
    if (!wizardp(this_user()))
      u = filter_array(u, (
                              : $1->is_visible()
                              :));
  }
  else
    u = filter_array(bodies(), (
                                   : $1->is_visible()
                                   :));

  u -= ({0});
  i = sizeof(u);
  frame->set_title(sprintf("Who - %d user%s", i, (i != 1 ? "s" : "")));
  if (u[0]->query_level())
  {
    frame->set_header_content("Level  Name");
    frame->set_footer_content(sprintf(WHO_FORMAT, mud_name(), ctime(time()), extra,
                                      i));
  }
  else
    frame->set_header_content(sprintf(WHO_FORMAT, mud_name(), ctime(time()), extra,
                                      i));

  if (!i)
    retval += "Sorry, no one fits that bill.";
  foreach (object body in u)
  {
    name = body->query_formatted_desc(78);
    if (!name)
      name = capitalize(body->query_userid());

    if (!body->is_visible())
      name = "(" + name + ")";
    if (body->test_flag(F_IN_EDIT))
      name = "*" + name;
    if (body->query_level())
      retval += sprintf(" %-6.6s %-68s\n", "" + body->query_level(), name);
    else
      retval += sprintf("%-68s\n", name);
  }
  frame->set_content(retval);
  return frame->render();
}

private
void main(string arg)
{
  if (arg == "")
    arg = 0;

  out(get_who_string(arg));
}

void player_menu_entry()
{
  bare_init();
  main(0);
  done_outputing();
}