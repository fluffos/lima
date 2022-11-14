/* Skills command by Tsath */

//:PLAYERCOMMAND
//$$ see: hp, stats, score, spells
//USAGE: skills
//
// Prints out a list of your skills and skill ranks.
// - Skill ranks are shown by roman numerals (the higher the better from 0-15)
// - Training points are shown at the end of the skill bars (in yellow)
//
// Optional arguments are: combat, magic or misc to only show a subsection
// of the skill tree.
//
// The more you use your skills, the better you get. So things get better all
// the time! When you have a lot of training points, search for a trainer to
// train you. They both need to have the right skills and at the right skill
// rank to train you.

#include <config.h>
#include <classes.h>

inherit CMD;
inherit CLASS_SKILL;
inherit M_WIDGETS;

private
void main(string arg)
{
  mapping skills;
  object body;
  int width = this_user()->query_screen_width() - 7;
  int skill_bar = width - 39;
  int self_view = 1;
  string *guild_skills = GUILD_D->query_guild_skills(this_body()->guilds_belong());
  string *names;

  if (strlen(arg) > 0 && wizardp(this_user()))
  {
    body = present(arg, environment(this_body()));
    if (!body)
      body = find_body(arg);
    if (!body)
    {
      out("Cannot find '" + arg + "'.\n");
      return;
    }
    out("Skills for " + capitalize(arg) + ":\n");
    self_view = 0;
    arg = 0;
  }

  if (strlen(arg) && arg != "combat" && arg != "misc" && !body)
  {
    out("Valid arguments are: combat or misc.\n");
    return;
  }

  if (!body)
    body = this_body();

  skills = body->get_skills() || body->query_skills();

  if (sizeof(skills) == 0)
  {
    out("You have no skills yet.\n");
    return;
  }

  names = keys(skills);
  foreach (string name in keys(skills))
  {
    string *parts = explode(name, "/");
    for (int i = sizeof(parts); i--;)
    {
      string sname = implode(parts[0..i], "/");

      if (member_array(sname, names) == -1)
      {
        names += ({sname});
      }
    }
  }

  names = sort_array(names, 1);

  if (!i_simplify())
  {
    foreach (string name in names)
    {
      class skill skill = skills[name];
      string *parts = explode(name, "/");
      string name2 = repeat_string("   ", sizeof(parts) - 1) + parts[ < 1];
      int level = sizeof(parts);
      string pretty_name = body->is_body() ? SKILL_D->skill_rank_pretty(body, name) : SKILL_D->monster_skill_rank_pretty(body, name);
      int percentage = body->is_body() ? SKILL_D->percent_for_next_rank(body, name) : SKILL_D->monster_percent_for_next_rank(body, name);
      int green = skill_bar * percentage / 100;
      int red = skill_bar - green;
      if (strlen(arg) && strsrch(name, arg) != 0)
        continue;

      //If it's a special skill and not part of our guild_skills, skip it in the list.
      if (strsrch(name, "/special/") != -1 && member_array(name, guild_skills) == -1)
        continue;

      if (level == 1)
      {
        outf("\n" + upper_case(pretty_name) + "\n" + simple_divider());
      }
      else
        outf("%-25s %4s [%%^GREEN%%^%s%%^RESET%%^%s%%^RESET%%^] "+
             "%%^YELLOW%%^%-5s%%^RESET%%^\n",
             repeat_string("  ", level - 2) + pretty_name,
             percentage + "%",
             repeat_string("=", green), repeat_string(".", red),
             "" + skill->training_points,
        );
    }
    if (self_view)
      outf("%sUse skills to train them. Skill ranks are from [1] to [20].\nYour current max is [%d] as you are level %d.",
           simple_divider(),
           body->level_skill_cap(),
           body->query_level());
  }
  else
  {
    outf("%-30s %-12s\n", "Skill name", "Progress");
    foreach (string name in names)
    {
      class skill skill = skills[name];
      int percentage = SKILL_D->percent_for_next_rank(body, name);
      string *parts = explode(name, "/");
      string name2 = repeat_string("   ", sizeof(parts) - 1) + parts[ < 1];
      if (strlen(arg) && strsrch(name, arg) != 0)
        continue;

      if (sizeof(parts) == 1)
      {
        outf("%s:\n", SKILL_D->skill_rank_simple(body, name));
      }
      else
        outf("%-30s %-12s\n",
             SKILL_D->skill_rank_simple(body, name), percentage + "%");
    }
  }
}
