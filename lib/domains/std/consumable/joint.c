inherit SLOWHEAL;

void setup()
{
    set_id("joint");
    add_adj("small");
    set_in_room_desc("A tight little home-rolled joined.");
    set_long("You can try to light it.");
    set_fuel(15);
    set_heal_value(5);
    set_consume_message("$N $vlight $p joint.");
    set_heal_message("You smoke your joint and feel more relaxed. Ah, yeah.");
    set_expire_message("A last wheeeeze on $p joint and $n $vflick it away.\n");
}

int direct_light_obj()
{
    return 1;
}

int direct_smoke_obj()
{
    return 1;
}

void do_light()
{
    do_consume();
}

void do_smoke()
{
    do_consume();
}

mixed direct_consume_obj() { return "#Don't eat joints, light them!"; }
