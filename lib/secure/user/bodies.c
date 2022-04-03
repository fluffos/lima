/* Do not remove the headers from this file! see /USAGE for more info. */

/*
 * Handling of multiple bodies per user.
 *
 * Created 2022 by Tsath
 */

#define BODY_FNAME 0
#define BODY_LEVEL 1
#define BODY_RACE 2
#define BODY_GENDER 3

nomask void save_me();

private
mapping bodies = ([]);
private
string selected;

void set_body(string name, string fname, string race, int gender)
{
    if (!mapp(bodies))
        bodies = ([]);
    bodies[name] = ({fname, 1, race, gender});
    save_me();
}

string query_selected_body()
{
    return selected;
}

int query_num_bodies()
{
    return sizeof(keys(bodies));
}

int set_selected_body(string name)
{
    name = lower_case(name);
    if (member_array(name, keys(bodies)) != -1)
    {
        selected = name;
        return 1;
    }
    return 0;
}

mapping query_bodies()
{
    return bodies;
}

int update_body(object body)
{
    string name = body->query_name();
    if (arrayp(bodies[name]))
    {
        bodies[name][BODY_LEVEL] = body->query_level();
        bodies[name][BODY_RACE] = body->query_race();
        bodies[name][BODY_GENDER] = body->query_gender();
        save_me();
        return 1;
    }
    return 0;
}

protected
nomask void set_body_fname(string name, string new_body_fname)
{
    bodies[name][BODY_FNAME] = new_body_fname;
    save_me();
}

string query_fname(string name)
{
    if (!mapp(bodies))
        set_body(name, "/std/race/human.c", "human", 0);
    if (bodies[name])
        return bodies[name][BODY_FNAME];
    return "/std/race/human.c";
}

int query_gender(string name)
{
    if (!mapp(bodies))
        set_body(name, "/std/race/human.c", "human", 0);
    if (bodies[name])
        return bodies[name][BODY_GENDER];
    return 0;
}