/* Do not remove the headers from this file! see /USAGE for more info. */

// Base functionality for behavior_tree

#include <commands.h>
#include <behaviour.h>

inherit NODE_CLASS;

#ifdef CLUSTER_NAVIGATION
void init_navigation_cluster();
#endif

void add_hook(string, function);
void remove_hook(string, function);
object query_target();
object query_link();
varargs int evaluate_node();

void action_arrival(object);
void action_departure(object);

mapping desires = ([]);
mapping blackboard = ([]);
mapping node_list, parents;

private
nosave function arrival_fn = (
    : action_arrival:);
private
nosave function departure_fn = (
    : action_departure:);

private
function my_hook;

private
object env;

private
int delay_time = 5; // Default delay time for leaves.
private
object debugee = 0;   // The body of the person debugging us.
string *queue = ({}); // Queue of nodes that must be processed.

private
int room_has_changed = 1;

int has_room_changed()
{
    return room_has_changed;
}

void room_checked()
{
    room_has_changed = 0;
}

mixed need_hooks() { return 1; }

object query_body()
{
    return this_object();
}

object query_shell_ob()
{
    return this_object();
}

string status(int s)
{
    switch (s)
    {
    case -1:
        return "NONE";
    case 0:
        return "FAILURE";
    case 1:
        return "SUCCESS";
    case 2:
        return "RUNNING";
    }
}

void backpush(mixed node)
{
    if (arrayp(node))
        queue += node;
    else
        queue += ({node});
}

void frontpush(string node)
{
    queue = ({node}) + queue;
}

string print_queue()
{
    string out = "";
    int i = 1;
    return format_list(queue);
}

mapping query_nodes()
{
    return node_list;
}

mapping query_parents()
{
    return parents;
}

void reset_tree()
{
    foreach (string name, class node node in node_list)
    {
        node->status = EVAL_NONE;
        node->node_num = 0;
    }
}

class node front()
{
    return sizeof(queue) ? node_list[queue[0]] : 0;
}

class node parent(class node node)
{
    return node && parents[node->name] ? node_list[parents[node->name]] : 0;
}

class node pop()
{
    class node n = front();
    queue = sizeof(queue) ? queue[1..] : ({});
    return n;
}

string discover_parent(string node)
{
    string parent;
    string *names = keys(node_list);
    int i = 0;
    while (!parent && i < sizeof(names))
    {
        if (member_array(node, node_list[names[i]]->children) != -1)
            parent = names[i];
        i++;
    }
    return parent;
}

// Accepts offstring as a single string or as an array of strings.
// If offspring is 0, the array is set to ({}).
varargs void create_node(int type, string name, mixed offspring)
{
    node_list[name] = new (class node, type
                           : type, name
                           : name, children
                           : arrayp(offspring) ? offspring : offspring ? ({offspring})
                                                                       : ({}),
                             delay
                           : type == NODE_LEAF ? 3 : 0);
    parents[name] = discover_parent(name);
}

void call_command(string str)
{
    object save_tu;
    mixed *winner;
    string verb, argument;

    save_tu = this_user();
    set_this_user(this_object());

    verb = str;
    sscanf(verb, "%s %s", verb, argument);

    winner = CMD_D->find_cmd_in_path(verb, ({CMD_DIR_PLAYER "/"}));
    if (arrayp(winner))
        winner[0]->call_main("", "", "", "", "", "", argument);
    else if (environment())
    {
        mixed result = parse_sentence(str);

        if (stringp(result))
            write(result);
    }

    set_this_user(save_tu);
}

// Return the number of listeners (player in room)
int query_observers()
{
    object *inv;
    if (!env)
        return 0;
    if (inv = all_inventory(env))
        return sizeof(filter(inv, (
                                      : $1->query_link()
                                      :)));
    return 0;
}

// The periodic action call_out
void behaviour_call()
{
    if (query_observers())
        call_out("behaviour_call", delay_time);
    evaluate_node();
}

// If last player leaves, remove call_out
void action_departure(object who)
{
    room_has_changed = 1;
    if (who->query_link())
    {
        if (!query_observers())
            remove_call_out("behaviour_call");
    }
}

// If first player arrives, add call_out
void action_arrival(object who)
{
    room_has_changed = 1;
    if (who->query_link())
    {
        if (query_observers() == 1)
            call_out("behaviour_call", delay_time);
    }
}

// It moves the "object_arrived/left" hooks (which are associated with a room)
// when the action monster moves
void action_movement()
{
    if (env)
    {
        env->remove_hook("object_arrived", arrival_fn);
        env->remove_hook("object_left", departure_fn);
    }

    env = environment();
    env->add_hook("object_arrived", arrival_fn);
    env->add_hook("object_left", departure_fn);
    if (find_call_out("behaviour_call") < 0)
        if (query_observers())
            call_out("behaviour_call", delay_time);
}

void debug(mixed s)
{
    if (debugee && debugee->is_body())
        tell(debugee, sprintf("%s: %O\n", __FILE__, (s)));
}

void debug_me(int i)
{
    debugee = i ? this_body() : 0;
}

void set_blackboard(string key, mixed value)
{
    blackboard[key] = value;
}

mixed blackboard(string key)
{
    return blackboard[key];
}

mapping query_blackboard()
{
    return blackboard;
}

mapping query_desires()
{
    return desires;
}

void mod_desire(string desire, int mod)
{
    if (!desires[desire])
        desires[desire] = 50;
    desires[desire] = CLAMP(desires[desire] + mod, 0, 100);
}

void add_child(string node, string child)
{
    if (!child)
        error("Cannot add empty child.");
    if (!arrayp(node_list[node]->children))
        node_list[node]->children = ({child});
    else
        node_list[node]->children += ({child});
    parents[child] = node;
}

private
void init_tree()
{
    node_list = ([]);
    parents = ([]);
    create_node(NODE_ROOT, "root", "root_sequence");
    create_node(NODE_SEQUENCE, "root_sequence");
#ifdef CLUSTER_NAVIGATION
    init_navigation_cluster();
#endif
}

void start_behaviour()
{
    if (!node_list)
        init_tree();
    add_hook("move", (
                         : action_movement:));
    if (env = environment(this_object()))
    {
        env->add_hook("object_arrived", arrival_fn);
        env->add_hook("object_left", departure_fn);
    }

    if (query_observers())
        call_out("behaviour_call", delay_time);
}

void stop_behaviour_call()
{
    remove_call_out("behaviour_call");
}
