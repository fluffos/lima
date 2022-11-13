/* Do not remove the headers from this file! see /USAGE for more info. */

/* XP, levels and skills for PLEXUS <Tsath>
**
** Level    XP
**   1      0
**   2      500
**   3      1500
     
*/

void save_me();

private
nosave int xp_factor = 250;
private
int experience = 0;
private
int level = 0;
private
#ifdef USE_KARMA
int karma = 100; //Karma goes from -1000 to 1000.
#endif

string query_body_style();
void update_body_style(string);
void do_game_command(string str);

int xp_for_level(int lev)
{
   return xp_factor * pow(lev, 2) - xp_factor * lev;
}

void set_level(int x)
{
   experience = xp_for_level(x);
   level = x;
   //Refresh the body with new abilities after changing level.
   update_body_style(query_body_style());
}

private
calc_level()
{
   //   TBUG("Calc_level(): "+level);
   level = to_int(xp_factor + sqrt(xp_factor * xp_factor - 4 * xp_factor * (-experience))) / (2 * xp_factor);
}

int query_level()
{
   if (level == 0)
      calc_level();
   return level == 0 ? 1 : level;
}

int query_next_xp()
{
   return xp_for_level(query_level() + 1);
}

int query_experience()
{
   return experience;
}

void remove_experience(int x)
{
   int old_level, min_xp, old_xp;
   old_level = query_level();
   min_xp = xp_for_level(old_level);
   old_xp = experience;

   experience -= x;
   if (experience < min_xp)
      experience = min_xp;

   if ((old_xp - experience) > 0)
      tell(this_object(), "%^RED%^You lost " + (old_xp - experience) + " xp.%^RESET%^\n");
   if (this_object()->query_link())
      save_me();
}

void add_experience(int x)
{
   int old_level, new_level;
   old_level = query_level();
   //   TBUG("Adding XP to" + this_object() + ": " + x + " / Link: " + this_object()->query_link());
   experience += x;
   calc_level();
   new_level = query_level();

   tell(this_object(), "%^YELLOW%^You gained " + x + " xp.%^RESET%^\n");

   if (new_level != old_level)
   {
      tell(this_object(), "\n>>>>> %^B_GREEN%^LEVEL UP!%^RESET%^ You're now level " + new_level + "! <<<<<\n");
      do_game_command("flex");
   }

   if (this_object()->query_link())
      save_me();
}

int skill_xp_plus()
{
   int total;
   foreach (int skill in values(this_object()->query_skills()))
      total += skill;
   return total;
}

//:FUNCTION calc_xp_for
// int calc_xp_for(int player_level, int monster_level)
// Decoupled function to allow external XP calculation.
int calc_xp_for(int player_level, int monster_level)
{
   int callerLevel = player_level > 0 ? player_level : 1;
   float multFactor = to_float(monster_level) / callerLevel;
   int xp = to_int(floor((1.0 + (monster_level / 10)) * ((xp_factor / 10)) * (multFactor > 1.2 ? 1.2 : multFactor)));
   return xp > 0 ? xp : 1;
}

//:FUNCTION xp_value
// int xp_value()
// Calculate XP value for monsters (and players). The function reduces XP for monsters if the player
// is too high level compared to the monster. It rewards players up to 20% if the monster is higher than
// their level. The function always returns 1 point as a minimum (sad).
int xp_value()
{
   int xp = calc_xp_for(previous_object()->query_level(), this_object()->query_level());

   //Only needed for debug
   int callerLevel = previous_object()->query_level() > 0 ? previous_object()->query_level() : 1;
   //TBUG("Caller level:" + callerLevel + " This level: " + this_object()->query_level());
   return xp;
}

#ifdef USE_KARMA
//:FUNCTION query_karma
// int query_karma()
// Returns the raw karma between -1000 and 1000.
// Only if USE_KARMA defined in combat_modules.h
int query_karma()
{
   return CLAMP(karma, -1000, 1000);
}

//:FUNCTION set_karma
//Set karma to specific number rather than modifyind it.
void set_karma(int k)
{
   karma = k;
}

//:FUNCTION modify_karma
// int modify_karma(int modifier)
// Modifies karma with a modifer from -5 to 5.
// The closer to 0 the more impact the karma modification will have.
// This also means that the more good or evil you are, the harder
// it will become to change it.
// Returns the new karma. Karma is between -1000 and 1000.
int modify_karma(int modifier)
{
   float karma_mod = 0;
   modifier = CLAMP(modifier, -5, 5);
   karma_mod = modifier * ((1100 / (200.0 + abs(karma))));
   karma = karma + karma_mod;

   karma = CLAMP(karma, -1000, 1000);
   return karma;
}
#endif
