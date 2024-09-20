/* Do not remove the headers from this file! see /USAGE for more info. */

//: MODULE
// m_bodystats_dnd.c -- body statistics (characteristics)
//
// .. TAGS: RST

#include <classes.h>
#include <config/stats.h>
#include <hooks.h>
#include <stats.h>


//Define this to use "Roll 4d6 drop lowest". If not defined, we use old school 3d6.
#define USE_4D6DL

private
inherit CLASS_STATMODS;

mixed call_hooks(string, int);
void refresh_stats();
int skill_stat_sum(string stat);

private
mapping stats = ([]);

nomask int query_stat_pure(string stat)
{
   return stats["stat_" + stat];
}

nomask int query_stat(string stat)
{
   return stats["cur_" + stat];
}

void set_mod_stat(string stat, int s)
{
   stats["mod_" + stat] = s;
}

int query_mod_stat(string stat)
{
   return stats["mod_" + stat];
}

void inc_mod_stat(string stat)
{
   stats["mod_" + stat]++;
}

int spare_points()
{
   int total_pts = (this_object()->query_level() / 4) * 2;
   total_pts -=
       stats["mod_str"] + stats["mod_dex"] + stats["mod_con"] + stats["mod_int"] + stats["mod_wis"] + stats["mod_cha"];
   return total_pts;
}

/*
** Derived statistics
*/
//: FUNCTION refresh_stats
// refresh_stats() recalculates all the stats and requery's all the bonuses.
// Combat calls this once a round.  If you are using stats in a non-combat
// setting, you might want to call this first.
void refresh_stats()
{
   int adj_str, adj_dex, adj_con, adj_int, adj_wis, adj_cha;

   stats["cur_str"] = stats["stat_str"] + stats["mod_str"] + (adj_str = call_hooks("str_bonus", HOOK_SUM));
   stats["cur_dex"] = stats["stat_dex"] + stats["mod_dex"] + (adj_dex = call_hooks("dex_bonus", HOOK_SUM));
   stats["cur_int"] = stats["stat_int"] + stats["mod_int"] + (adj_int = call_hooks("int_bonus", HOOK_SUM));
   stats["cur_wis"] = stats["stat_wis"] + stats["mod_wis"] + (adj_wis = call_hooks("wil_bonus", HOOK_SUM));
   stats["cur_con"] = stats["stat_con"] + stats["mod_con"] + (adj_con = call_hooks("wil_bonus", HOOK_SUM));
   stats["cur_cha"] = stats["stat_cha"] + stats["mod_cha"] + (adj_cha = call_hooks("wil_bonus", HOOK_SUM));
}

nomask void set_stat(string stat, int c)
{
   stats["stat_" + stat] = c;
}

/*
** Character creation stuff
**
** query_roll_mods() may be overriden by subclasses to provide for changes
**   in the statistics generation.
**
** init_stats() may only be called once (admins may call whenever)
*/
class stat_roll_mods query_roll_mods()
{
   // should be overloaded
   return new (class stat_roll_mods);
}

private
int roll_dice()
{
   int *total = ({});
   int min;

   for (int i = 0; i < 4; i++)
      total += ({random(6) + 1});
   min = min(total);
   return ((int)sum(total...)) - min;
}

private
nomask int roll_stat()
{
#ifdef USE_4D6DL
   return roll_dice();
#else
   return random(6) + random(6) + random(6) + 3;
#endif
}

int max_stat()
{
   return max(({stats["stat_str"], stats["stat_dex"], stats["stat_int"], stats["stat_wis"], stats["stat_con"],
                stats["stat_cha"]}));
}

void reset_stat_increases()
{
   stats["mod_str"] = 0;
   stats["mod_dex"] = 0;
   stats["mod_con"] = 0;
   stats["mod_int"] = 0;
   stats["mod_wis"] = 0;
   stats["mod_cha"] = 0;
   refresh_stats();
}

//: FUNCTION init_stats
// Rolls the stats for the first time, based on the proper racial adjustments.
// Admins can call this to reinitialize a player's stats (for example, in the
// case of abysmally horrific (near minimum) rolls.
nomask void init_stats()
{
   class stat_roll_mods mods;

   /*
   if ( stat_str && !check_previous_privilege(1) )
   error("cannot reinitialize statistics\n");
*/

   stats["stat_str"] = roll_stat();
   stats["stat_dex"] = roll_stat();
   stats["stat_con"] = roll_stat();
   stats["stat_int"] = roll_stat();
   stats["stat_wis"] = roll_stat();
   stats["stat_cha"] = roll_stat();

   reset_stat_increases();

   refresh_stats();
}

/*
** Must be defined for the mudlib inteface. Put in the stat you want.
*/
int query_carrying_stat()
{
   return query_stat("str");
}

/*
** Must be defined for the mudlib inteface. Put in the stat you want.
*/
int query_health_stat()
{
   return query_stat("con");
}

/*
** Must be defined for the mudlib inteface. Put in the stat you want.
*/
int query_physical_dmg_stat()
{
   return query_stat("str");
}

/*
** Must be defined for the mudlib inteface. Put in the stat you want.
*/
int query_agility_stat()
{
   return query_stat("dex");
}

/*
** Must be defined for the mudlib inteface. Put in the stat you want.
*/
int query_social_stat()
{
   return query_stat("cha");
}

/*
** Must be defined for the mudlib inteface. Put in the stat you want.
*/
int query_mental_stat()
{
   return query_stat("int");
}

/*
** Must be defined for the mudlib inteface. Put in the stat you want.
*/
int query_reflex_stat()
{
   return query_stat("int");
}

mapping stat_abrev()
{
   return (["strength":"str",
           "dexterity":"dex", "intelligence":"int", "wisdom":"wis", "constitution":"con", "charisma":"cha", ]);
}

int colour_strlen(string str)
{
   return strlen(XTERM256_D->substitute_colour(str, "plain"));
}

// Always returns a strlen 6.
string pretty_bonus(int b)
{
   string out = "      ";
   if (b > 0)
      out = "(<002>+" + b + "<res>)";
   else if (b < 0)
      out = "(<001>" + b + "<res>)";

   if (colour_strlen(out) < 6)
      out += repeat_string(" ", 6 - colour_strlen(out));

   return out;
}

string dnd_stat_mod(string stat)
{
   int mod = stats["cur_" + stat] / 2 - 5;
   return mod > -1 ? "+" + mod : "" + mod;
}

string show_stats()
{
   return sprintf(
       "    <bld>Strength<res> %2d (%2s)   <bld>Dexterity<res> %2d (%2s)   <bld>Constitution<res> %2d (%2s)\n" +      //
       "<bld>Intelligence<res> %2d (%2s)      <bld>Wisdom<res> %2d (%2s)       <bld>Charisma<res> %2d (%2s)\n\n", //
       query_stat("str"), dnd_stat_mod("str"),                                                                    //
       query_stat("dex"), dnd_stat_mod("dex"),                                                                    //
       query_stat("con"), dnd_stat_mod("con"),                                                                    //
       query_stat("int"), dnd_stat_mod("int"),                                                                    //
       query_stat("wis"), dnd_stat_mod("wis"),                                                                    //
       query_stat("cha"), dnd_stat_mod("cha"), );
}

/*
** Object inheriting this module should be sure to call this.
*/
void internal_setup(mixed args...)
{
}
