/* Do not remove the headers from this file! see /USAGE for more info. */

// This is a simple skills module, demonstrating one way to integrate the
// Lima skills system into the combat system
// It is designed to return a number in the range 0 to 100 for chance to hit,
// based on skills in the range 0 to MAX_SKILL_VALUE

#include <combat_modules.h>
#include <skills.h>
#include <classes.h>
#include <log.h>

inherit CLASS_SKILL;

int query_level();
int query_weapon_class();
int aggregate_skill(string skill);
int query_to_hit_bonus(object target);
int query_disarm_bonus(object target);
int query_damage_bonus(object target);
int query_anti_disarm(object target);
int cached_skill_rank(string s);

int chance_to_hit(object weapon, object target)
{
  string attack_skill = weapon ? weapon->query_skill_used() : "combat/unarmed";
  string defend_skill = target->query_defend_skill_used();
  int skill_contrib = strsrch(attack_skill, "combat/ranged") != -1 ? (this_object()->query_agi() / 5) : (this_object()->query_str() / 5);
  int attack_value;
  int defend_value;
  int attack_mod = 1;
  int defend_mod = 1;
  int res;
  int skill_test = 0;
  class skill skill;

  //Skill restrictions
  if (weapon->query_restricted())
  {
    attack_skill = weapon->query_restricted_skill();
    attack_mod = 10;
    defend_mod = 10;
  }

#ifndef USE_SKILLS
  write("*** CONFIG ERROR: "+mud_name()+" set to not define USE_SKILLS, but still defines FORMULA_SKILLS in combat_config.h. These two are incompatible. Please fix.\n");
  return 1;
#endif

  attack_value = to_int(aggregate_skill(attack_skill) / attack_mod);
  defend_value = target->aggregate_skill(defend_skill) * defend_mod;

  //TBUG("This: "+this_object()+" Weapon: "+weapon+" target: "+target+"\nAttack skill: "+attack_skill+" ("+attack_value+") Defend skill: "+defend_skill+" ("+defend_value+")");
  // Skill tests to potentially increase the skills

  //LOG_D->log(LOG_SKILLS, "attack," + attack_skill + "," + attack_value + ",defend," + defend_skill + "," + defend_value + "\n");
  skill_test = this_object()->test_skill(attack_skill, defend_value);

  res = attack_value - defend_value + MAX_SKILL_VALUE;
  res = res * 100 / (2 * MAX_SKILL_VALUE);

  if (skill_test)
  {
    // Calculate the value for successful skill_test
    return res + (weapon ? weapon->query_to_hit_bonus(target) : query_to_hit_bonus(target)) + skill_contrib;
  }
  else
  {
    // Calculate the value for fails.
    return res;
  }
}

int defend_chance(object weapon, object target)
{
  string attack_skill = weapon ? weapon->query_skill_used() : "combat/unarmed";
  string defend_skill = target->query_defend_skill_used();
  int attack_value;
  int defend_value;
  int attack_mod = 1;
  int defend_mod = 1;
  int res;
  class skill skill;

  //Skill restrictions
  if (weapon->query_restricted())
  {
    attack_skill = weapon->query_restricted_skill();
    attack_mod = 10;
    defend_mod = 10;
  }

  attack_value = to_int(aggregate_skill(attack_skill) / attack_mod);
  defend_value = target->aggregate_skill(defend_skill) * defend_mod;

  //LOG_D->log(LOG_SKILLS, "defend," + defend_skill + "," + defend_value + ",attack," + attack_skill + "," + attack_value + "\n");
  target->test_skill(defend_skill, attack_value);

  res = defend_value - attack_value + MAX_SKILL_VALUE;
  res = res * 100 / (2 * MAX_SKILL_VALUE);
  return res;
}

int disarm_chance(object target)
{
  int attack_mod = 1;
  int defend_mod = 1;
  int res;
  object weapon = this_object()->query_weapon();
  int chance_to_hit = chance_to_hit(weapon, target);
  int defend_disarm_value = target->aggregate_skill("combat/disarm") * defend_mod;
  //Calculate anti_disarm chances from the target + her weapon.
  int disarm_prevention =
      (target ? target->query_anti_disarm() : 0) + (target->query_weapon() ? target->query_weapon()->query_anti_disarm() : 0);

  //LOG_D->log(LOG_SKILLS, "disarm," + "combat/disarm" + "," + chance_to_hit + "\n");
  target->test_skill("combat/disarm", chance_to_hit);

  res = chance_to_hit - ((defend_disarm_value * 100) / MAX_SKILL_VALUE);

  //TBUG("Chance_to_hit: "+chance_to_hit+" Defend_disarm: "+(defend_disarm_value/100.0)+" Res: "+res+
  //" Disarm prev:"+disarm_prevention+" To hit bonus: "+(weapon ? weapon->query_to_hit_bonus(target) : query_to_hit_bonus(target))+" Disarm bonus "+query_disarm_bonus(target));
  //Subtract to_hit bonus and add disarm bonus.
  return to_int((res - (weapon ? weapon->query_to_hit_bonus(target) : query_to_hit_bonus(target)) + query_disarm_bonus(target) - disarm_prevention) / 10);
}

int calculate_damage(object weapon, object target)
{
  string attack_skill = weapon ? weapon->query_skill_used() : "combat/unarmed";
  int skill_contrib = strsrch(attack_skill, "combat/ranged") != -1 ? (this_object()->query_agi() / 5) : (this_object()->query_str() / 5);

  //If the weapon is set to restricted (happens when wielded in /std/adversary/wield/limbs)
  //we reduce the damage by 90% (you suck!).
  return cached_skill_rank(attack_skill) + random(query_level() + skill_contrib) +
         (weapon->query_restricted() ? to_int((random(weapon->query_weapon_class()) + 1) / 10) : random(weapon->query_weapon_class()) + 1 + query_damage_bonus(target));
}
