/* Do not remove the headers from this file! see /USAGE for more info. */

// limbs.c - Adversary module to control limb-based health. Body styles
//           are stored in /daemons/body_d.c.

#include <limbs.h>

inherit CLASS_LIMB;
inherit __DIR__ "diagnose_msg";

void die();
varargs void simple_action(string, string);
void update_health();
protected
void restore_concentration(int x);
void save_me();
int query_level();
void calc_level();
int do_unwield(string);
int query_asleep();
int query_stunned();
int xp_value();
int karma_impact();
int query_wil();
int query_con();
varargs int query_capacity(string relation);
varargs int query_max_capacity(string relation);
int query_no_move_capacity();
void gmcp_send_vitals();

private
nosave string body_style = "humanoid";
private
mapping health = BODY_D->get_body("humanoid");
private
nosave int health_time = time();
private
int concentration = 0;
private
int heal_rate = 15;
private
int concentration_rate = 30;
private
int dead = 0;

//:FUNCTION check_body_style
// int check_body_style()
// Checks if this body has a body defined already (returns 1), otherwise
// returns 0 and adds a humanoid body. This can be replaced with
// update_body_style(string body_style) should that be needed.
int check_body_style()
{
   if (mapp(health))
      return 1;
   health = BODY_D->get_body("humanoid");
   return 0;
}

string query_body_style()
{
   return body_style;
}

//:FUNCTION hp_adjustment
// int hp_adjustment(int hp,int level)
// Returns the adjustment HP for a limb for an adversary.
int hp_adjustment(int hp, int level)
{
   float hpModifier = 1 + (level / 10.0);
   return to_int(hp * hpModifier);
}

//:FUNCTION update_body_style
// int update_body_style(string body_style);
// Queries BODY_D for the number and type of limbs that will be used.
// e.g. update_body_style("humanoid") will give the body a torso, head,
// two arms, and two legs.
// Returns 0 if the body style doesn't exist or if it doesn't contain
// at least one vital or system limb.
int update_body_style(string bstyle)
{
   mapping new_body = BODY_D->get_body(bstyle);
   body_style = bstyle;
   calc_level();
   //TBUG("Setting body style "+body_style+" for "+this_object()+ " Level "+this_object()->query_level());

   if (!new_body)
      return 0;

   // Make sure we have at least one vital or system limb..

   if (!filter(keys(new_body), (
                                   : ((class limb)$(new_body)[$1])->flags &
                                         (LIMB_VITAL | LIMB_SYSTEM)
                                   :)))
      return 0;

   health = new_body;

   foreach (string name, class limb l in health)
   {
      l->max_health = hp_adjustment(l->max_health, this_object()->query_level());
      l->health = hp_adjustment(l->max_health, this_object()->query_level());
   }

   update_health();
   return 1;
}

void update_body()
{
}

int is_vital_limb(string limb)
{
   return ((class limb)health[limb])->flags & LIMB_VITAL;
}

int is_system_limb(string limb)
{
   return ((class limb)health[limb])->flags & LIMB_SYSTEM;
}

int is_wielding_limb(string limb)
{
   return ((class limb)health[limb])->flags & LIMB_WIELDING;
}

int is_mobile_limb(string limb)
{
   return ((class limb)health[limb])->flags & LIMB_MOBILE;
}

int is_attacking_limb(string limb)
{
   return ((class limb)health[limb])->flags & LIMB_ATTACKING;
}

//:FUNCTION query_limbs
// string *query_limbs();
// Returns a string *containing all limbs that health is applied to.
string *query_limbs()
{
   return keys(health);
}

//:FUNCTION query_wielding_limbs
// string *query_wielding_limbs();
// Returns a string *containing all the limbs that can wield weapons.
string *query_wielding_limbs()
{
   return sort_array(filter(keys(health), (
                                              : ((class limb)health[$1])->flags & LIMB_WIELDING:)),
                     -1);
}

//:FUNCTION query_attacking_limbs
// string *query_attacking_limbs();
// Returns a string *containing all the limba that can attack.
string *query_attacking_limbs()
{
   return filter(keys(health), (
                                   : ((class limb)health[$1])->flags &
                                         LIMB_ATTACKING:));
}

//:FUNCTION query_vital_limbs
// string *query_vital_limbs();
// Returns a string *containing all the limbs that are considered
// vital for survival. If any one of these limbs is disabled, the
// adversary dies.
string *query_vital_limbs()
{
   return filter(keys(health), (
                                   : ((class limb)health[$1])->flags & LIMB_VITAL:));
}

//:FUNCTION query_mobile_limbs
// string *query_mobile_limbs();
// Lima doesn't do anything with mobile limbs, but they're provided for
// those who want health of mobile limbs to affect movement and such.
string *query_mobile_limbs()
{
   return filter(keys(health), (
                                   : ((class limb)health[$1])->flags & LIMB_MOBILE:));
}

//:FUNCTION query_system_limbs
// string *query_system_limbs();
// Returns a string *of 'system' limbs. When ALL system limbs are
// disabled, the adversary dies.
string *query_system_limbs()
{
   return filter(keys(health), (
                                   : ((class limb)health[$1])->flags & LIMB_SYSTEM:));
}

//:FUNCTION query_non_limbs
// string *query_non_limbs();
// Returns a list of body parts that are not worth tracking health for.
// Such body parts are defined by having a max_health of -1.
string *query_non_limbs()
{
   return filter(keys(health), (
                                   : ((class limb)health[$1])->max_health == -1
                                   :));
}

void set_heal_rate(int x)
{
   if (x < 0)
      x = 0; /* a number of things depend on this */
   heal_rate = x;
}

int query_heal_rate()
{
   return heal_rate;
}

//:FUNCTION set_max_limb_health
// void set_max_limb_health(string limb, int x);
// Sets the maximum health for a given limb.
void set_max_limb_health(string limb, int x)
{
   class limb tmp = health[limb];

   if (!tmp)
      error("Bad limb.\n");
   if (tmp->max_health == -1 || x < 0)
      return;

   update_health();

   tmp->max_health = x;
   tmp->health = x;
}

//:FUNCTION set_max_health
// void set_max_health(int x);
// Set the maximum number of hit points of a monster, and also set it's
// hit points to the new max
void set_max_health(int x)
{
   int max = 0;

   foreach (string l, class limb d in health)
      if (d->max_health > max)
         max = d->max_health;

   foreach (string l, class limb d in health)
   {
      if (d->max_health != -1)
      {
         d->max_health = (d->max_health * x / max < 1 ? 1 : d->max_health * x / max);
         d->health = d->max_health;
      }
   }
}

void slain_by(object slayer)
{
   //Override me.
}

//:FUNCTION kill_us
// void kill_us();
// This functions handles quite a bit:
// * Adds XP to the slayer
// * calls slain_by(player) in this object.
// * Updates the slayers BESTIARY
// * Updates opponents Karma
// * and finally kills us. (die()).
//
// Yeah, sorry, we had to.
void kill_us()
{
   dead = 1;
   if (previous_object())
   {
      string team = PARTY_D->locate_user(previous_object()->short());
      object *viable = filter_array(all_inventory(environment(this_object())), (
                                                                                   : $1->is_body()
                                                                                   :));
      this_object()->slain_by(previous_object());
      if (team && sizeof(viable) > 1)
      {
         //Player is in a team
         PARTY_D->award_experience(previous_object(), team, viable, query_level());
         PARTY_D->modify_karma(team, viable, karma_impact());
         previous_object()->query_bestiary()->add_slain(this_object());
      }
      else
      {
         previous_object()->modify_karma(karma_impact());
         previous_object()->add_experience(xp_value());
         if (previous_object()->query_bestiary())
            previous_object()->query_bestiary()->add_slain(this_object());
      }
   }

   //If this is a player, his worn/wielded equipment loses random durability.
   if (this_object()->is_body())
   {
      object *obs = all_inventory(this_object());
      object *filtered_obs = ({});

      foreach (object ob in obs)
      {
         if ((ob->is_armor() && ob->ob_state()) || (ob->is_weapon() && ob->query_wielded_by() == this_object()))
            filtered_obs += ({ob});
         if (ob->is_spell_effect())
            ob->remove();
      }

      foreach (object o in filtered_obs)
      {
         int reduce_dura = o->max_durability() / 10;
         o->decrease_durability(random(reduce_dura));
         o->set_worn(0);
         o->do_remove();
      }
   }

   dead = 1;
   die();
}

string query_random_limb()
{
   string *limbs = filter(keys(health),
                          (
                              : ((class limb)health[$1])->health > 0
                              :));
   return sizeof(limbs) ? choice(limbs) : 0;
}

//:FUNCTION disable_limb
// void disable_limb(string limb);
// Disables a limb. For effects on vital and system limbs, see
// query_vital_limbs() and query_system_limbs().
void disable_limb(string limb)
{
   int dont_kill_me_now = 0;

   if (health[limb]->health == -1)
      return;

   ((class limb)health[limb])->health = 0;
   if (((class limb)health[limb])->flags & LIMB_VITAL)
   {
      kill_us();
   }
   else if (((class limb)health[limb])->flags & LIMB_SYSTEM)
   {
      foreach (string l in query_system_limbs())
      {
         if (((class limb)health[l])->health != 0)
            dont_kill_me_now = 1;
      }
      if (dont_kill_me_now == 1)
         simple_action("$N cannot use $p $o anymore.", limb);
      else
         kill_us();
   }
   else if (((class limb)health[limb])->flags & LIMB_WIELDING)
   {
      simple_action("$N cannot use $p $o anymore.", limb);
      do_unwield(limb);
   }
   else
      simple_action("$N cannot use $p $o anymore.", limb);
}

//:FUNCTION enable_limb
// void enable_limb(string limb);
// Re-enables a disabled limb.
void enable_limb(string limb)
{
   class limb tmp = health[limb];

   if (tmp->health == 0)
      tmp->health = 1;
   health[limb] = tmp;
}

varargs void set_health(string limb, int x)
{
   class limb tmp = health[limb];

   if (!tmp)
      error("Bad limb.\n");
   update_health();

   if (x > tmp->max_health)
      error("Attempt to set health > max_health.\n");
   if (tmp->health < 1 || dead)
      return;

   tmp->health = x;
   if (tmp->health <= 0)
      disable_limb(limb);
}

//:FUNCTION hurt_us
// varargs int hurt_us(int x, string limb);
// Hurt us a specified amount.
varargs int hurt_us(int x, string limb)
{
   class limb tmp;
   //TBUG("hurt_us: "+x+" limb: "+limb+" this:"+this_object()->short());

   if (!limb)
      limb = query_random_limb();
   tmp = health[limb];
   if (!tmp)
      error("Bad limb.\n");
      
   update_health();

   if (tmp->health < 1 || dead)
      return 0;

   tmp->health -= x;
   if (tmp->health <= 0)
   {
      x += tmp->health; /* negative */
      tmp->health = 0;
      disable_limb(limb);
   }

   update_health();
   gmcp_send_vitals();

   return x;
}

//:FUNCTION heal_limb
// protected void heal_limb(string limb, int x);
// Heal us a specified amount, truncating at max_health.
protected
void heal_limb(string limb, int x)
{
   class limb tmp = health[limb];

   if (!tmp)
      error("Bad limb.\n");

   if (tmp->health == -1)
      return;

   if (tmp->health == 0 || dead)
      x = fuzzy_divide(x, 10);

   tmp->health += x;
   if (tmp->health > tmp->max_health)
      tmp->health = tmp->max_health;
}

//:FUNCTION is_limb
// int is_limb(string s);
// Returns 1 if 's' is a valid limb.
int is_limb(string s)
{
   return !undefinedp(health[s]);
}

//:FUNCTION query_max_health
// varargs int query_max_health(string limb);
// Tells us the maximum health of a given limb.
varargs int query_max_health(string limb)
{
   int x = 1;

   if (limb)
      return is_limb(limb) ? ((class limb)health[limb])->max_health : 0;

   foreach (string l in keys(health))
      if (health[l]->max_health > x)
         x = health[l]->max_health;
   return x;
}

//:FUNCTION heal_us
// varargs void heal_us(int x, string limb);
// Heals all limbs by 'x' amount.
varargs void heal_us(int x, string limb)
{
   if (!limb || undefinedp(limb))
   {
      foreach (limb in keys(health))
         heal_limb(limb, x);
      return;
   }
   else
      heal_limb(limb, x);
}

//:FUNCTION reincarnate
// void reincarnate();
// Makes us alive again!
void reincarnate()
{
   if (dead)
   {
      foreach (string limb, class limb l in health)
         enable_limb(limb);
      dead = 0;
      health_time = time();
   }
   this_object()->gmcp_send_vitals();
}

void update_health()
{
   if (dead)
      return;

   if (time() != health_time)
   {
      foreach (string limb in keys(health))
         heal_limb(limb, fuzzy_divide((time() - health_time) * (heal_rate + this_object()->query_con()), 2000));
      restore_concentration(fuzzy_divide((time() - health_time) * (concentration_rate + this_object()->query_wil()), 2000));
      health_time = time();
   }
}

//:FUNCTION query_health
// int query_health(string limb);
// Find the current number of hitpoints of a monster
int query_health(string limb)
{
   update_health();
   return ((class limb)health[limb])->health;
}

//:FUNCTION get_health
// mapping get_health();
// Return the health mapping for adversary.
mapping get_health()
{
   if (!mapp(health))
      health=BODY_D->get_body("humanoid");
   update_health();
   return health;
}

int query_ghost()
{
   return dead;
}

// : FUNCTION banner_wounded
// void banner_wounded(string limb, int hp);
// Tell this_object() that they're badly wounded.
void banner_wounded(string limb, int hp)
{
   tell(this_object(), "\n[%^RED%^WOUNDED!%^RESET%^] Your " + limb + " has " + hp + " hp!\n");
}

//:FUNCTION badly_wounded
// int badly_wounded();
// Returns 1 if we're near death.
int badly_wounded()
{
   int wimpy_at = 20;

   if (this_object()->is_body())
   {
      object link = this_object()->query_link();
      if (link)
      {
         wimpy_at = this_user()->query_shell_ob()->get_variable("wimpy_percent");
         if (!wimpy_at)
            wimpy_at = 20;
      }
   }

   foreach (string limb in keys(health))
   {
      class limb lb = health[limb];

      if ((lb->flags & LIMB_VITAL) && (lb->health < ((lb->max_health * wimpy_at) / 100)))
      {
         banner_wounded(limb, lb->health);
         return 1;
      }
   }
}

int query_no_move_capacity()
{
   return query_max_capacity();
}

//:FUNCTION can_move
// int can_move();
// Returns 1 if we can move, 0 if not.
int can_move()
{
   int can_move = 0;

   //Are we over max capacity?
   if (query_capacity() > query_no_move_capacity())
      return 0;

   //Do we have limbs that can move us?
   foreach (string l, class limb lb in health)
   {
      if ((lb->flags & LIMB_MOBILE) && (lb->health > 0))
         can_move = 1;
   }

   return can_move;
}

//:FUNCTION query_concentration
// int query_concentration()
// Returns the amount of concentration currently had by the adversary.
int query_concentration()
{
   return concentration;
}

//:FUNCTION max_concentration
// int max_concentration()
// Returns the max concentration based on the mana stat and a bonus for level of the
// adversary.
int max_concentration()
{
   return to_int((this_object()->query_man() || 1) * (1 + (query_level() / 10.0)));
}

//:FUNCTION set_concentration
// void set_concentration(int mp)
// Set the concentration to an integer, but never higher than max_concentration().
void set_concentration(int mp)
{
   if (mp > max_concentration())
      mp = max_concentration();
   concentration = mp;
}

//:FUNCTION spend_concentration
// void spend_concentration(int m)
// Spends concentration nomatter whether there is enough or too little. concentration is left at 0 no matter
// what. Returns 1 if we had enough, 0 if we didn't.
int spend_concentration(int m)
{
   concentration -= m;
   if (concentration < 0)
   {
      concentration = 0;
      return 0;
   }
   return 1;
}

//:FUNCTION use_concentration
// int use_concentration(int m)
// Uses concentration from the concentration pool only if it's available and returns 1. If there is not enough
// nothing is used, and 0 is returned.
int use_concentration(int m)
{
   TBUG("M:" + m);
   if (concentration - m > 0)
   {
      concentration -= m;
      return 1;
   }
   return 0;
}

//:FUNCTION restore_concentration
// protected void restore_concentration(int x);
// Restore us a specified amount, truncating at max_concentration().
protected
void restore_concentration(int x)
{
   //Dont get concentration while dead, sorry.
   if (dead)
      return;

   concentration += x;
   if (concentration > max_concentration())
      concentration = max_concentration();
}

//:FUNCTION heal_all
// void heal_all();
// Heal us entirely.
void heal_all()
{
   foreach (string l in keys(health))
      if (!health[l]->health)
         enable_limb(l);
   heal_us(query_max_health());
   restore_concentration(max_concentration());
}

string diagnose()
{
   string ret;
   string *damaged_limbs;

   if (query_ghost())
      return "$N $vare dead. Other than that, things are going pretty well for $n.\n";

   if (query_asleep())
      ret = "$N $vare asleep.\n";
   else if (query_stunned())
      ret = "$N $vare stunned.\n";
   else
      ret = "";

   damaged_limbs = filter(query_limbs(),
                          (
                              : query_health($1) < health[$1]->max_health:));
   foreach (string limb in damaged_limbs)
      ret += diagnose_limb_msg(health[limb]->health * 100 / health[limb]->max_health, limb);

   if (ret == "")
      ret = "You are in excellent health.\n";

   return ret;
}
