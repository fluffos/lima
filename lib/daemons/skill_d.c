/* Do not remove the headers from this file! see /USAGE for more info. */

#include <security.h>
#include <classes.h>
#include <skills.h>

inherit M_DAEMON_DATA;
inherit CLASS_SKILL;

/*
** Keep the list of the available skills.
**
** NOTE: The values in this mapping are unused (set to 1)
*/
private
mapping skills = ([]);
private
mixed SKILL_RANKS = ({100, 250, 500, 1000, 1500, 2000, 2500, 3000, 4000, 5000, 6000, 7000, 8000, 9000, 10000});
private
mixed SKILL_TITLES = ({"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X", "XI", "XII", "XIII", "XIV", "XV"});

#define PRIV_REQUIRED "Mudlib:daemons"

string *register_skill(string skill)
{
    string *parts;
    string *result;

    if (!check_privilege(PRIV_REQUIRED))
        error("security: illegal attempt to add a skill\n");

    if (skills[skill])
        return 0;

    /* create the skill and all parent skills */
    parts = explode(skill, "/");
    result = allocate(sizeof(parts));
    for (int i = sizeof(parts); i--;)
    {
        string name = implode(parts[0..i], "/");

        if (!skills[name])
        {
            result[i] = name;
            skills[result[i]] = 1;
        }
    }

    save_me();

    return filter(result, (
                              : $1:));
}

string *remove_skill(string skill)
{
    string *result = ({skill});

    if (!check_privilege(PRIV_REQUIRED))
        error("security: illegal attempt to remove a skill\n");

    if (!skills[skill])
        return 0;

    /* delete the skill */
    map_delete(skills, skill);

    /* delete all subskills: skills that start with <skill> + "/" */
    skill = skill + "/";
    foreach (string name in keys(skills))
        if (strsrch(name, skill) == 0)
        {
            map_delete(skills, name);
            result += ({name});
        }

    save_me();

    return result;
}

string *query_skills()
{
    return sort_array(keys(skills), 1);
}

int skill_title_from_pts(int skill_pts)
{
    int rank = 0;

    while (skill_pts > SKILL_RANKS[rank])
    {
        rank++;
    }
    return SKILL_TITLES[rank];
}

int rank_name_from_pts(int skill_pts)
{
    int rank = 0;

    while (skill_pts > SKILL_RANKS[rank])
    {
        rank++;
    }
    return rank;
}

int skill_rank(object player, string skill_name)
{
    class skill skill;
    int rank = 0;

    skill = player->get_skill(skill_name);
    while (skill->skill_points > SKILL_RANKS[rank])
    {
        rank++;
    }
    return rank;
}

mixed titles()
{
    return SKILL_TITLES;
}

mixed ranks()
{
    return SKILL_RANKS;
}

string skill_req_pretty(string skill_name, int rank)
{
    string name = explode(skill_name, "/")[ < 1];
    return capitalize(name) + (rank > 0 ? " [" + SKILL_TITLES[rank] + "]" : "");
}

string skill_rank_pretty(object player, string skill_name)
{
    int level = sizeof(explode(skill_name, "/"));
    string name = explode(skill_name, "/")[ < 1];
    int rank = skill_rank(player, skill_name);
    return capitalize(name) + (rank > 0 ? " [" + SKILL_TITLES[rank] + "]" : "");
}

string skill_rank_simple(object player, string skill_name)
{
    int rank = skill_rank(player, skill_name);
    string name = strsrch(skill_name, "/") != -1 ? implode(explode(skill_name, "/")[1..], "/") + " rank " + rank : upper_case(skill_name);
    return name;
}

int percent_for_next_rank(object player, string skill_name)
{
    class skill skill;
    int rank = skill_rank(player, skill_name);
    int next_rank = SKILL_RANKS[rank] - (rank == 0 ? 0 : SKILL_RANKS[rank - 1]);
    skill = player->get_skill(skill_name);

    return (skill->skill_points - (rank == 0 ? 0 : SKILL_RANKS[rank - 1])) * 100 / next_rank;
}

void stat_me()
{
    write("SKILLS_D stats:\n");

    foreach (string name in sort_array(keys(skills),1))
    {
        write("\t"+name+"\n");
    }
}