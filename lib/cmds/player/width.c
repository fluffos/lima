/* Do not remove the headers from this file! see /USAGE for more info. */

//: PLAYERCOMMAND
// USAGE width
//       width <number>
//       width auto
//
// Allows you to see (or set) what the mud will use as your screen width when
// formatting information. Some clients send the width of the terminal window,
// and if yours do that, you can use 'width auto' to set your width automatically.
// It's probably a good idea to set it to the same as your client' setting,
// so they don't get confused when talking to each other.

inherit CMD;

void main(mixed width)
{
  int wi = to_int(width);
  TBUG(width != "auto");
  TBUG(!intp(wi));
  if (!intp(wi) || (strlen(width) > 0 && width != "auto"))
  {
    out("Must set screen width to an integer or 'auto'.\n");
    return;
  }
  if (!wi && width != "auto")
  {
    out("Your current screen width is " + this_user()->query_screen_width() +
        (this_user()->is_width_auto() ? " (auto)" : "") + ".\n");
    return;
  }
  if (wi < 10 && width != "auto")
  {
    out("Screen width must be 10 or greater.\n");
    return;
  }
  this_user()->set_screen_width(width == "auto" ? 0 : wi);
  out("Screen width set to " + (width == "auto" ? "auto" : wi) + ".\n");
}
