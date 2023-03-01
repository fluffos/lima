/* Do not remove the headers from this file! see /USAGE for more info. */

// CLUSTER_NAVIGATION

#include <behaviour.h>

inherit M_SMARTMOVE;

void add_child(string node, string child);
varargs void create_node(int type, string name, mixed offspring);
void set_blackboard(string key, mixed value);
mixed blackboard(string key);
void call_command(string str);
int has_room_changed();
void room_checked();
void do_wander();
int player_did_arrive(string dir);
void moving();

private
string *wander_area = ({});

void init_navigation_cluster()
{
    // Add nodes
    // create_node(NODE_LEAF,"do_wander");
}


//: FUNCTION set_wander_area
// Set the area(s) that an NPC can wander in.  If this is not set
// it is assumed that the NPC can wander anywhere without area
// restrictions.
void set_wander_area(mixed areas)
{
    if (arrayp(areas))
    {
        wander_area = clean_array(areas);
    }

    if (stringp(areas))
    {
        wander_area = clean_array(({areas}));
    }
}

//: FUNCTION add_wander_area
// Add area(s) which an NPC can wander in.  See set_wander_area()
void add_wander_area(mixed areas)
{
    if (arrayp(areas))
    {
        wander_area = clean_array(wander_area + areas);
    }

    if (stringp(areas))
    {
        wander_area = clean_array(wander_area + ({areas}));
    }
}

//: FUNCTION remove_wander_area
// Remove area(s) which an NPC can wander in.  See set_wander_area()
void remove_wander_area(string *area...)
{
    if (member_array(area, wander_area))
        wander_area -= ({area});
}

//: FUNCTION clear_wander_area
// Clear the area(s) in which an NPC can wander in.  Effectively
// this allows the NPC to wander anywhere.  See set_wander_area()
void clear_wander_area()
{
    wander_area = ({});
}

//: FUNCTION query_wander_area
// Returns an array of areas in which may wander.
// See set_wander_area()
string *query_wander_area()
{
    return wander_area;
}

void move_me(string direction)
{
    call_command(sprintf("go %s", direction));
}

int do_wander()
{
    string *directions;
    int i;
    string chosen_dir;
    string file;
    object dest;

    if (environment(this_object()))
        directions = environment(this_object())->query_exit_directions();

    /* Stop if there are no directions (this takes care of blue prints*/
    if (!directions || !i = sizeof(directions))
        return;

    chosen_dir = directions[random(i)];

    file = environment(this_object())->query_exit_destination(chosen_dir);
    /* If the destination is not loaded, do so */
    dest = find_object(file);
 
    if (!dest)
        dest = load_object(file);

    /* Check if the npc has wander restrictions */
    if (sizeof(wander_area))
    {
        if (arrayp(wander_area) && arrayp(dest->query_area()))
        {
            if (sizeof(wander_area & dest->query_area()))
            {
                move_me(chosen_dir);
            }
        }
    }
    else
    {
        move_me(chosen_dir);
    }
    return EVAL_SUCCESS;
}
