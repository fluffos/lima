// Do not remove the headers from this file! see /USAGE for more info.
inherit OBJ;

string dest;

mixed do_press();
mixed direct_press_obj();
string long()
{
  return "The button is marked '" + dest + "'\n";
}

string query_destination()
{
  return dest;
}

void setup(string d)
{
  set_attached(1);
  set_adj("elevator");
  set_id("button");
  dest = d;
  add_adj(d);
}

mixed do_press()
{
  environment(this_object())->handle_press(dest);
  return 1;
}

mixed direct_press_obj()
{
  return 1;
}
