/* Do not remove the headers from this file! see /USAGE for more info. */

inherit ADVERSARY;
inherit BEHAVIOUR_TREE;

void setup()
{
    set_name("George");
    set_id("goblin", "george", "geo");
    set_gender(1);
    set_proper_name("George the Goblin");
    set_in_room_desc("A small ugly goblin called George.");
    set_long("George is a small goblin, but with a strange glint in his eye.");
    set_max_health(30);
    set_wielding("/domains/std/weapon/sword");
    set_level(10);
    set_wander_area("wiz_area");
    start_behaviour();
    //set_objects((["^std/oak_door_key":1]));
}
