// Do not remove the headers from this file! see /USAGE for more info.
inherit OBJ;

string where;
string elevator;

mixed direct_press_obj();

void do_press()
{
    int state = 0;
    this_body()->simple_action("$N $vcall the elevator by pressing the $o.", this_object());
    state = load_object(evaluate_path(elevator))->call_elevator(where);

    switch (state)
    {
    case 1:
        tell_from_inside(environment(this_object()), "The lamp on the elevator button lights up.\n");
        environment(this_object())->set_room_state("lamp");
        break;
    case 2:
        tell_from_inside(environment(this_object()), "The lamp briefly blinks.\n");
        break;
    case 0:
        tell_from_inside(environment(this_object()), "The flashes red.\n");
        break;
    }
}

void setup(string w, string e)
{
    set_attached(1);
    set_adj("elevator");
    set_id("button");
    where = w;
    elevator = e;
}

mixed direct_press_obj()
{
    return 1;
}
