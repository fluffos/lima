/* Do not remove the headers from this file! see /USAGE for more info. */

/*
 * The concept is a bit abstract, so the name isn't entirely correct.  This
 * is an abstraction of the concept of something that one attacks with;
 * either a weapon or one's self.  It controls the combat messages, and
 * modifies the result based on the target, etc.
 *
 * This means that when you fight with a weapon, the _weapon_ is really
 * attacking, not you.  Some of the names in this are a bit backwards compat,
 * though.
 *
 * April, 1998: Iizuka made significant changes and updated for adversary.
 */

inherit CLASS_EVENT_INFO;

/* Stats for damaging things */
private
nosave int weapon_class = 1;
private
nosave int to_hit_bonus;
private
nosave int disarm_bonus;
private
nosave int damage_bonus;
private
nosave int anti_disarm;
private
nosave string *damage_type = ({"blow"});
private
nosave mapping def_combat_messages = ([]);
private
nosave mapping combat_messages = ([]);
private
nosave string skill_used = "combat/unarmed";

/* Restrictions */
private
nosave string restricted_skill = "combat";
private
nosave string restricted_message = "You do not feel skilled with this weapon.";
private
nosave int restriction_level = 0;
private
nosave int restriction_rank = 0;
private
nosave int restricted = 0;
private
nosave mapping effects = ([]);
private
nosave int must_dual_wield = 0;
private
nosave int can_dual_wield = 0;

mixed adjust_my_result(mixed result) { return result; }

void add_effect(string key, object eff)
{
  if (effects[key])
    effects[key]->destroy_effect();

  effects[key] = eff;
}

void clear_effects()
{
  foreach (object effect in values(effects))
  {
    effect->destroy_effect();
  }
}

mapping query_effects()
{
  return effects;
}

int query_must_dual_wield() { return must_dual_wield; }
void set_must_dual_wield(int x) { must_dual_wield = x; }

int query_can_dual_wield() { return can_dual_wield; }
void set_can_dual_wield(int x) { can_dual_wield = x; }

int query_to_hit_bonus(object target) { return to_hit_bonus; }
void set_to_hit_bonus(int x) { to_hit_bonus = x; }

int query_disarm_bonus(object target) { return disarm_bonus; }
void set_disarm_bonus(int x) { disarm_bonus = x; }

int query_damage_bonus(object target) { return damage_bonus; }
void set_damage_bonus(int x) { damage_bonus = x; }

int query_anti_disarm(object target) { return anti_disarm; }
void set_anti_disarm(int x) { anti_disarm = x; }

string *query_damage_type() { return damage_type; }

void set_damage_type(string *str...)
{
  string *exclude = str - DAMAGE_D->query_damage_types();
  if (sizeof(exclude))
    error("Invalid damage type(s) : " + implode(exclude, ","));
  damage_type = str;
}

int is_weapon() { return 1; }

//: FUNCTION secondary_weapon_part()
// Override this to include it in messages when the weapon is used.
object secondary_weapon_part()
{
  return 0;
}

//: FUNCTION set_combat_messages
// Set the set of combat messages which are used by default
void set_combat_messages(string type)
{
  // TBUG("Combat messages set to "+type+" for "+this_object());
  if (!(def_combat_messages = MESSAGES_D->query_messages(type)))
    error("No messages of that type.\n");
}

//: FUNCTION set_combat_message
// Set a single combat message, removing the existing one(s)
void set_combat_message(string type, string msg)
{
  combat_messages[type] = msg;
}

//: FUNCTION query_combat_message
// Returns the combat message for a given type
mixed query_combat_message(string type)
{
  return combat_messages[type] || def_combat_messages[type];
}

private
mixed merge_lists(mixed list, mixed item, mixed def)
{
  if (arrayp(list))
    return list + ({item});
  else if (list)
    return ({list, item});
  else if (def)
    return merge_lists(def, item, 0);
  else
    return item;
}

//: FUNCTION add_combat_message
// Adds another combat message to the damage source.
//
// add_combat_message("miss","$N $vmiss completely, $n seems drunk.");
void add_combat_message(string type, string msg)
{
  combat_messages[type] = merge_lists(combat_messages[type], msg,
                                      def_combat_messages[type]);
}

//: FUNCTION replace_combat_message
// Replace any other messages with this message
//
// replace_combat_message("miss","$N $vmiss completely, $n seems drunk.");
void replace_combat_message(string type, string msg)
{
  combat_messages[type] = ({msg});
}

void set_weapon_class(int x)
{
  weapon_class = x;
  this_object()->set_max_durability(100 * x);
}

int query_weapon_class()
{
  if (this_object()->durability_percent() > 40)
    return weapon_class;
  else
  {
    int p = (this_object()->durability_percent() * 2.5);
    if (p < 20)
      p = 20;
    return (weapon_class * p / 100) || 1;
  }
}

string query_skill_used() { return skill_used; }

void set_restricted(int rest) { restricted = rest; }
void set_skill_used(string new_skill_used) { skill_used = new_skill_used; }
void set_skill_restriction_message(string new_restriction_msg) { restricted_message = new_restriction_msg; }

void set_skill_restriction(string new_restricted_skill, int rank)
{
  if (rank < 0)
    return;
  restricted_skill = new_restricted_skill;
  restriction_level = SKILL_D->ranks()[rank - 1];
  restriction_rank = rank;
}

string get_extra_long()
{
  string restricted_str = "";
  string durability_str;

  if (this_object()->query_wielded_by())
    restricted_str = restricted ? "This weapon requires skill rank " + SKILL_D->skill_req_pretty(restricted_skill, restriction_rank) + " before it can be trained and used proficiently."
                                : "You meet the rank of " + SKILL_D->skill_req_pretty(restricted_skill, restriction_rank) + " so you can train and use this weapon.";
  else
    restricted_str = "This weapon requires skill rank " + SKILL_D->skill_req_pretty(restricted_skill, restriction_rank) + ".";

  durability_str = this_object()->has_durability() ? " " + this_object()->durability_extra_long() : "";

  if (this_object()->is_living())
    return "";
  return (restriction_level ? restricted_str : "") + durability_str;
}

// Set when wield has found that the weapon was restricted. You need to unwield and wield to clear this.
int query_restricted()
{
  return restricted;
}

string query_restriction_message()
{
  return restricted_message;
}

string query_restricted_skill()
{
  return restricted_skill;
}

int query_restriction_level()
{
  return restriction_level;
}

class event_info source_modify_event(class event_info evt)
{
  object *effs = values(effects);
  effs -= ({0});

  // Optimize for normal case
  if (sizeof(effs) == 0)
    return evt;

  foreach (object eff in effs)
  {
    if (eff)
      eff->effect_modify_event(evt);
  }

  return evt;
}

void remove_effects()
{
  // Remove any lingering effects when damage source is removed.
  foreach (object eff in values(effects))
  {
    destruct(eff);
  }
}

mapping lpscript_attributes()
{
  return (["weapon_class":({LPSCRIPT_INT, "setup", "set_weapon_class"}),
             "skill_used":({LPSCRIPT_STRING, "setup", "set_skill_used"}),
        "combat_messages":({LPSCRIPT_STRING, "setup", "set_combat_messages"}),
  ]);
}