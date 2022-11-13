// Tsath

//:PLAYERCOMMAND
//USAGE: 	stats
//$$ see: skills, hp, score, spells
//
//Shows your scores in the base statistics:
// 	Strength:	physical stature, power, melee attack damage
// 	Agility:	body control, speed, flexibility, ranged attack damage
// 	Intelligence:	inherent capability for learning, spell damage
// 	Willpower:	drive, stamina, defense against spells, concentration regain
//
//Magic items also contribute to the base stats. They can be seen in the
//"Bonus" row if applicable.
//
//As well as your three derived stats:
//	Constitution:	stamina, carrying capacity, health regain
//	Wisdom:		collected knowledge
// 	Charisma:	natural attraction, leadership, persuasion
//
//Constitution is:  20% STR, 20% AGI and 60% WIL.
//Wisdom is: 7% STR, 7% AGI, 31% INT, 25% WIL and 30% "misc/knowledge"
//Charisma is: 15% STR, 10% AGI, 25% INT, 25% WIL and 25% "misc/converse"
//Mana is: 10% STR, 50% INT, 20% WIL, 20% "spell" skill.
//
//Your derived stats automatically level up as your skills and base
//stats do. You can see estimated contribution in the stats command,
//but notice that rounding occurs so they may not match 100%.
//
//When you level up you gain points to increase your base stats
//permanently. To do this, check 'stats' to see your available points,
//and spend them by doing:
//     stats raise str
//     stats raise agi
//     stats raise int
//     stats raise wil
//
//These increase are permanent and cannot be undone, so choose with care.

#include <hooks.h>
#include <stats.h>

inherit CMD;
inherit M_WIDGETS;

/*
string decorate_bonus(int bonus)
{
     if (!bonus)
          return "";
     if (bonus > 0)
          return "+%^GREEN%^" + bonus + "%^RESET%^";
     else
          return "-%^RED%^" + bonus + "%^RESET%^";
}

string decorate_mod(int bonus)
{
     if (!bonus)
          return "";
     if (bonus > 0)
          return "+%^CYAN%^" + bonus + "%^RESET%^";
     else
          return "-%^RED%^" + bonus + "%^RESET%^";
}
*/

string decorate_bonus(int bonus)
{
     if (!bonus)
          return "";
     if (bonus > 0)
          return "+" + bonus;
     else
          return "-" + bonus;
}

string decorate_mod(int bonus)
{
     if (!bonus)
          return "";
     if (bonus > 0)
          return "+" + bonus;
     else
          return "-" + bonus;
}

void usage()
{
     write("Usage:\n stats raise str\n stats raise agi\n stats raise int\n stats raise wil\n");
}

private
void main(string arg)
{
     string *args;
     int pts, total;
     string rank = this_body()->refresh_stats(); //This doesn't return rank, but we need to call it first. *heh*
     int str_mod = this_body()->query_mod_str();
     int agi_mod = this_body()->query_mod_agi();
     int int_mod = this_body()->query_mod_int();
     int wil_mod = this_body()->query_mod_wil();

     int str_bonus = this_body()->call_hooks("str_bonus", HOOK_SUM);
     int agi_bonus = this_body()->call_hooks("agi_bonus", HOOK_SUM);
     int int_bonus = this_body()->call_hooks("int_bonus", HOOK_SUM);
     int wil_bonus = this_body()->call_hooks("wil_bonus", HOOK_SUM);

     if (strlen(arg))
          args = explode(arg, " ");

     if (sizeof(args))
     {
          if (args[0] != "raise" || sizeof(args) != 2)
               return usage();

          if (!this_body()->spare_points())
          {
               write("You have no more points. Level up for more.\n");
               return;
          }

          switch (lower_case(args[1]))
          {
          case "str":
          case "strength":
               this_body()->inc_mod_str();
               write("Strength increased to " + this_body()->query_str() + ".\n");
               break;
          case "agi":
          case "agility":
               this_body()->inc_mod_agi();
               write("Agility increased to " + this_body()->query_agi() + ".\n");
               break;
          case "int":
          case "intelligence":
               this_body()->inc_mod_int();
               write("Intelligence increased to " + this_body()->query_int() + ".\n");
               break;
          case "wil":
          case "willpower":
               this_body()->inc_mod_wil();
               write("Willpower increased to " + this_body()->query_wil() + ".\n");
               break;
          }
          return;
     }

     if (!i_simplify())
     {
          outf("%%^YELLOW%%^BASE STATS:%%^RESET%%^\n%%^BOLD%%^%15|s%15|s%15|s%14|s%%^RESET%%^\n"
               "     .-\"\"-.      "
               "   .-\"\"-.      "
               "   .-\"\"-.      "
               "   .-\"\"-.  \n"
               "    /      \\     "
               "  /      \\     "
               "  /      \\     "
               "  /      \\  \n"
               "   ;  %%^GREEN%%^%4|s%%^RESET%%^  ;    "
               " ;  %%^GREEN%%^%4|s%%^RESET%%^  ;    "
               " ;  %%^GREEN%%^%4|s%%^RESET%%^  ;    "
               " ;  %%^GREEN%%^%4|s%%^RESET%%^  ;  \n"
               "    \\      /     "
               "  \\      /     "
               "  \\      /     "
               "  \\      /  \n"
               "     '-..-'      "
               "   '-..-'      "
               "   '-..-'      "
               "   '-..-'    \n"
               "%s%4|s      "
               "     %4|s      "
               "     %4|s      "
               "     %4|s    %s"
               "%s%4|s      "
               "     %4|s      "
               "     %4|s      "
               "     %4|s    %s",
               "STRENGTH", "AGILITY", "INTELLIGENCE", "WILLPOWER",
               this_body()->query_str() + "",
               this_body()->query_agi() + "",
               this_body()->query_int() + "",
               this_body()->query_wil() + "",
               (str_mod != 0 || agi_mod != 0 || int_mod != 0 || wil_mod != 0 ? "Pts.: " : "       "),
               decorate_mod(str_mod),
               decorate_mod(agi_mod),
               decorate_mod(int_mod),
               decorate_mod(wil_mod),
               (str_mod != 0 || agi_mod != 0 || int_mod != 0 || wil_mod != 0 ? "\n" : ""),
               (str_bonus != 0 || agi_bonus != 0 || int_bonus != 0 || wil_bonus != 0 ? "Bonus:" : "      "),
               decorate_bonus(str_bonus),
               decorate_bonus(agi_bonus),
               decorate_bonus(int_bonus),
               decorate_bonus(wil_bonus),
               (str_bonus != 0 || agi_bonus != 0 || int_bonus != 0 || wil_bonus != 0 ? "\n" : ""), );

          outf("\n");
          outf("%%^YELLOW%%^DERIVED STATS:%%^RESET%%^             %%^BOLD%%^%5-s %5-s %5-s %5-s %5-s%%^RESET%%^\n%%^BOLD%%^CONSTITUTION%%^RESET%%^"
               "     %%^CYAN%%^%-10s%%^RESET%%^ %5-s %5-s %5-s %5-s  %5-s\n",
               "STR", "AGI", "INT", "WIL", "Skill",
               "" + this_body()->query_con(),
               "" + this_body()->stat_component(CON, STR),
               "" + this_body()->stat_component(CON, AGI),
               "" + this_body()->stat_component(CON, INT),
               "" + this_body()->stat_component(CON, WIL),
               "-", );
          outf("%%^BOLD%%^WISDOM%%^RESET%%^           %%^CYAN%%^%-10s%%^RESET%%^ %5-s %5-s %5-s %5-s  %5-s(Knowledge)\n",
               "" + this_body()->query_wis(),
               "" + this_body()->stat_component(WIS, STR),
               "" + this_body()->stat_component(WIS, AGI),
               "" + this_body()->stat_component(WIS, INT),
               "" + this_body()->stat_component(WIS, WIL),
               "" + (this_body()->skill_wis_sum()), );

          outf("%%^BOLD%%^CHARISMA%%^RESET%%^         %%^CYAN%%^%-10s%%^RESET%%^ %5-s %5-s %5-s %5-s  %5-s(Converse)\n",
               "" + this_body()->query_cha(),
               "" + this_body()->stat_component(CHA, STR),
               "" + this_body()->stat_component(CHA, AGI),
               "" + this_body()->stat_component(CHA, INT),
               "" + this_body()->stat_component(CHA, WIL),
               "" + this_body()->skill_cha_sum(), );
          outf("%%^BOLD%%^MANA%%^RESET%%^             %%^CYAN%%^%-10s%%^RESET%%^ %5-s %5-s %5-s %5-s  %5-s(Magic)\n",
               "" + this_body()->query_man(),
               "" + this_body()->stat_component(MAN, STR),
               "" + this_body()->stat_component(MAN, AGI),
               "" + this_body()->stat_component(MAN, INT),
               "" + this_body()->stat_component(MAN, WIL),
               "" + this_body()->skill_man_sum(), );
          if (this_body()->spare_points())
          {
               outf("You have %d points to spend on base stats.\n", this_body()->spare_points());
               out("Hint: 'help stats' or 'help levels' for more info\n");
               /*
               */
          }
     }
     else
     {
          outf("Base stats:\n");
          outf("%s %d\n", "Strength", this_body()->query_str());
          outf("%s %d\n", "Agility", this_body()->query_agi());
          outf("%s %d\n", "Intellligence", this_body()->query_int());
          outf("%s %d\n\n", "Willpower", this_body()->query_wil());

          outf("Derived stats:\n");
          outf("%s %d\n", "Constitution", this_body()->query_con());
          outf("%s %d\n", "Wisdom", this_body()->query_wis());
          outf("%s %d\n", "Charisma", this_body()->query_cha());
          outf("%s %d\n", "Mana", this_body()->query_man());
     }
}
