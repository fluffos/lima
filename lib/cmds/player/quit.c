/* Do not remove the headers from this file! see /USAGE for more info. */

// Adapted to new user menu system, Tsath 2022

//:PLAYERCOMMAND
//Quit to the user menu.

inherit CMD;

private void main()
{
  this_body()->quit();
  new(USER_MENU)->start_menu();
}