/* Do not remove the headers from this file! see /USAGE for more info. */

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
  mapping skills = this_body()->get_skills();
  int width = this_user()->query_screen_width() - 7;
  int skill_bar = width - 39;
  string *names;
  int first = 1;

  if (strlen(arg) && arg != "combat" && arg != "magic" && arg != "misc")
  {
    out("Valid arguments are: combat, magic or misc.\n");
    return;
  }

  if (sizeof(skills) == 0)
  {
#ifdef USE_SKILLS
    out("You have no skills yet.\n");
#else
    out(mud_name()+" does not use skills.\n");
#endif
    return;
  }
  names = sort_array(keys(skills), 1);

  if (!i_simplify())
  {
    foreach (string name in names)
    {
      class skill skill = skills[name];
      string *parts = explode(name, "/");
      string name2 = repeat_string("   ", sizeof(parts) - 1) + parts[ < 1];
      int level = sizeof(parts);
      string pretty_name = SKILL_D->skill_rank_pretty(this_body(), name);
      int percentage = SKILL_D->percent_for_next_rank(this_body(), name);
      int green = skill_bar * percentage / 100;
      int red = skill_bar - green;
      if (strlen(arg) && strsrch(name, arg) != 0)
        continue;

      if (level == 1)
      {
        //      outf(" %%^BOLD%%^%"+width+"s%%^RESET%%^\n"," "+repeat_string("_",sizeof(pretty_name)+2)+" ");
        if (first)
        {
          outf(" %%^BOLD%%^%'_'|" + width + "s%%^RESET%%^\n", "| " + pretty_name + " |");
          first = 0;
        }
        else
          outf("|%%^BOLD%%^%'_'|" + width + "s%%^RESET%%^|\n", "| " + pretty_name + " |");

        outf("|%%^BOLD%%^%' '" + width + "s%%^RESET%%^|\n", "");
      }
      else
        outf("|%-25s %4s [%%^GREEN%%^%s%%^RESET%%^%s%%^RESET%%^] %%^YELLOW%%^%-5s%%^RESET%%^|\n",
             repeat_string("  ", level - 2) + pretty_name,
             percentage + "%",
             repeat_string("=", green), repeat_string(".", red),
             "" + skill->training_points, );
    }

    outf("|%%^BOLD%%^%'_'|" + width + "s%%^RESET%%^|\n", "");
  }
  else
  {
    outf("%-30s %-12s  %-8s\n", "Skill name", "Progress", "Training points");
    foreach (string name in names)
    {
      class skill skill = skills[name];
      int percentage = SKILL_D->percent_for_next_rank(this_body(), name);
      string *parts = explode(name, "/");
      string name2 = repeat_string("   ", sizeof(parts) - 1) + parts[ < 1];
      if (strlen(arg) && strsrch(name, arg) != 0)
        continue;

      if (sizeof(parts) == 1)
      {
        outf("%s:\n", SKILL_D->skill_rank_simple(this_body(), name));
      }
      else
        outf("%-30s %-12s  %-8s\n",
             SKILL_D->skill_rank_simple(this_body(), name), percentage + "%", sizeof(parts) > 1 ? skill->training_points + "" : "-");
    }
  }
}
