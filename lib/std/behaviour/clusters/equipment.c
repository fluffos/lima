
/* Do not remove the headers from this file! see /USAGE for more info. */

// CLUSTER_EQUIPMENT

#include <behaviour.h>

void add_child(string node, string child);
varargs void create_node(int type, string name, mixed offspring);
object *worst_threats_present();
object *query_targets();
string badly_wounded();
string very_wounded();
void do_game_command(string str);

int no_heals = 0;

void init_equipment_cluster()
{
    // If any of these ones return true we stop here, and navigate somewhere else
    create_node(NODE_SELECTOR, "equipment_seq", ({"hurt", "find_heal"}));
    add_child("root_sequence", "equipment_seq");
    create_node(NODE_SEQUENCE, "hurt", ({"safe_to_heal", "use_heal"}));
    create_node(NODE_SEQUENCE, "find_heal", ({"take_from_room", "use_heal"}));
    create_node(NODE_LEAF, "safe_to_heal");
    create_node(NODE_LEAF, "use_heal");
    create_node(NODE_LEAF, "take_from_room");
}

int use_heal()
{
    object *consumables = filter_array(all_inventory(this_object()), (
                                                                         : $1->is_healing()
                                                                         :));
    object pick;
    string limb = very_wounded() || badly_wounded();

    if (sizeof(consumables))
        pick = choice(consumables);

    if (!pick)
    {
        no_heals = 1;
        return;
    }

    // Bit defensive code here since things are very interactive and can disappear
    // quickly.
    if (pick->is_bandage() && limb)
    {
        this_object()->do_game_command("apply bandage to " + limb);
        return EVAL_SUCCESS;
    }
    else if (pick->is_food())
    {
        pick->do_eat(this_object());
        return EVAL_SUCCESS;
    }
    else if (pick->is_drink())
    {
        pick->drink_it(this_object());
        return EVAL_SUCCESS;
    }
    return EVAL_FAILURE;
}

int safe_to_heal()
{
    string wounded = this_object()->badly_wounded() || this_object()->very_wounded();
    int targets = sizeof(query_targets() - ({0}));

    // We're not fighting
    return targets ? EVAL_FAILURE : EVAL_SUCCESS;
}

int take_from_room()
{
    object *consumables;
    object pick;

    if (!environment())
        return EVAL_FAILURE;
    consumables = filter_array(all_inventory(environment()), (
                                                                 : $1->is_healing()
                                                                 :));

    if (sizeof(consumables))
    {
        pick = choice(consumables);
    }
    if (!pick)
    {
        return EVAL_FAILURE;
    }

    // Grab it!
    do_game_command("get " + pick->short());

    // We got it!
    if (environment(pick) == this_object())
    {
        no_heals = 0;
        return EVAL_SUCCESS;
    }
    return EVAL_FAILURE;
}