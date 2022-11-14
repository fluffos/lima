// Do not remove the headers from this file! see /USAGE for more info.
#include <hooks.h>

inherit MOVING_ROOM;

/* The destinations with their lamps lit */
string *queue = ({});
string door_direction = "nowhere";

void hook_elevator_door_closed();

void setup_buttons()
{
    foreach (string dest in keys(dests))
    {
        new ("/std/elevator_button", dest)->move(this_object());
    }
}

void setup()
{
    set_brief("Elevator");

    set_state_description("elevator_door_on", "\nThere is an open door to the " + door_direction + ".");
    set_state_description("elevator_door_off", "\nThere is a closed door to the " + door_direction + ".");

    set_in_progress((
                        : tell_from_inside, this_object(), "The elevator continues ...\n"
                        :),
                    5);
    add_hook("elevator_door_closed", (
                                         : hook_elevator_door_closed:));
}

void set_door_direction(string d)
{
    door_direction = d;
    set_state_description("elevator_door_on", "\nThere is an open door to the " + door_direction + ".");
    set_state_description("elevator_door_off", "\nThere is a closed door to the " + door_direction + ".");
}

void move_to(string dest)
{
    ::move_to(dest);
}

void handle_queue()
{
    if (query_in_motion() || !sizeof(queue))
        return;

    call_hooks("elevator_will_move", HOOK_IGNORE, 0);
}

void hook_elevator_door_closed()
{
    if (!sizeof(queue))
        return;

    move_to(queue[0]);
}

string *queue()
{
    return queue;
}

/* add a destination to the queue. */
void add_to_queue(string where)
{
    if (member_array(where, queue) != -1)
        return;
    queue += ({where});
    handle_queue();
}

void handle_press(string dest)
{
    if (!dest)
    {
        return;
    }
    this_body()->simple_action("$N $vpress the '" + dest + "' button.\n");

    if (query_where() == dest)
    {
        call_hooks("elevator_arrived", HOOK_IGNORE, 0, query_location());
    }
    else
    {
        add_to_queue(dest);
    }
}

void arrive()
{
    tell_from_inside(this_object(), "The elevator slows down and stops\n");
    ::arrive();
    queue -= ({query_where()});
    call_hooks("elevator_arrived", HOOK_IGNORE, 0, query_location());
}

int call_elevator(string where)
{
    if (query_where() == where)
    {
        call_hooks("elevator_arrived", HOOK_IGNORE, 0, query_location());
        return 2;
    }

    if (member_array(where, queue) != -1)
    {
        write("Nothing happens.\n");
        return 0;
    }

    add_to_queue(where);
    return 1;
}
