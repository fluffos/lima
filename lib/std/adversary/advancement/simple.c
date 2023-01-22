/* Do not remove the headers from this file! see /USAGE for more info. */

#define XP_FACTOR 250

void save_me();

private
int level = 1;
private
int experience = 0;
private
int xp_modifier;
private
nosave int guild_xp_buff;

void set_level(int x)
{
   level = x;
}

int query_level()
{
   return level;
}

void raise_level()
{
   set_level(query_level() + 1);
   write("Congratulations! You gained a level!\n");
}

int query_experience()
{
   return experience;
}

void set_guild_xp_buff(int buff)
{
   if (buff > guild_xp_buff)
   {
      guild_xp_buff = buff;
      tell(this_object(), "%^COMBAT_CONDITION%^You received an XP buff of " + buff + "%%^RESET%^.\n");
   }
}

void clear_guild_xp_buff(int buff)
{
   guild_xp_buff = 0;
   tell(this_object(), "%^COMBAT_CONDITION%^Your XP buff fades.%^RESET%^\n");
}

int query_guild_xp_buff()
{
   return guild_xp_buff;
}

void set_experience(int x)
{
   experience = x;
   if (this_body()->query_link())
      this_body()->save_me();
}

void remove_experience(int x)
{
   int old_level, min_xp, old_xp;
   old_level = query_level();
   min_xp = 0;
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

   tell(this_object(), "%^YELLOW%^You gained " + x + " xp.%^RESET%^\n");

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
   int xp;
   float multFactor = (((0.0 + monster_level) / callerLevel));
   if (multFactor > 1.0)
      multFactor = ((multFactor - 1) * 2) + 1;
   xp = to_int(floor((1.0 + (monster_level / 1.0)) * ((XP_FACTOR / 15)) * (multFactor > 2.0 ? 2.0 : multFactor)));
   //TBUG("***: "+multFactor);
   //handling global xp modifier
   //xp = to_int(xp * GAME_D->query_global_xp_mult());
   return xp > 0 ? xp : 1;
}

//:FUNCTION xp_value
// int xp_value()
// Calculate XP value for monsters (and players). The function reduces XP for monsters if the player
// is too high level compared to the monster. It rewards players up to 20% if the monster is higher than
// their level. The function always returns 1 point as a minimum (sad).
varargs int xp_value(object xp_for)
{
   int xp;
   if (!xp_for)
      xp_for = previous_object();
   xp = calc_xp_for(xp_for->query_could_be_level(), this_object()->query_could_be_level());
   xp = to_int(xp * (1.0 + (xp_for->query_guild_xp_buff() / 100.0)));

   //Only needed for debug
   //int callerLevel = previous_object()->query_could_be_level() > 0 ? previous_object()->query_could_be_level() : 1;
   //TBUG("Caller level:" + callerLevel + " This level: " + this_object()->query_level());
   return xp;
}

int query_xp_modifier() { return xp_modifier; }

void set_xp_modifier(int n) { xp_modifier = n; }

int query_xp_for_level(int lev)
{
   int level = query_level() - 1;
   if (lev)
      level = lev;
   return XP_FACTOR * pow(level, 2) - XP_FACTOR * level;
}

int query_next_xp()
{
   return query_xp_for_level(query_level() + 1);
}

int query_level_for_xp(int xp)
{
   if (!xp)
      xp = experience;
   return to_int(XP_FACTOR + sqrt(XP_FACTOR * XP_FACTOR - 4 * XP_FACTOR * (-xp))) / (2 * XP_FACTOR);
}

int query_could_be_level()
{
   return (this_object()->is_body() ? query_level_for_xp(experience) : query_level());
}