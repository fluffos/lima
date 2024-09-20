/* Do not remove the headers from this file! see /USAGE for more info. */

inherit SWORD;

void setup()
{
   set_adj("dull");
   set_weapon_class(5);
   set_weight(1.1);
   set_value(15);
   add_combat_message("miss", "$N $vtake a clumsy swipe at $t, but only $vsucceed in making a fool of $r.");
}
