/* Do not remove the headers from this file! see /USAGE for more info. */

inherit WEAPON;
inherit M_THROWABLE;
inherit M_VALUABLE;

void setup()
{
  set_adj("simple");
  set_id("club");
  set_weapon_class(4);
  set_damage_type("bludgeoning");
  set_long("A rough club carved from a single piece of wood.");
  set_in_room_desc("A simple club is lying on the ground.\n");
  set_weight(1);
  set_value(0.1);
  set_skill_used("combat/club");
  set_combat_messages("combat-bludgeon");
  set_salvageable((["wood":95,"metal":5 ]));
}
