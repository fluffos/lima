/* Do not remove the headers from this file! see /USAGE for more info. */

/*
 * Handle the ability of 'monsters' to 'target' other objects.
 *
 * One can rewrite this to, say, pick a random target instead of concentrating
 * on one person, but be careful that query_target() must continually return
 * the same object between get_target()'s
 */

void stop_attacking();

private
nosave object target;
private
nosave object *other_targets = ({});
#ifdef TARGETTING_IS_RANDOM
private
nosave int explicit; // Attack the person we just switch_to()'ed
#endif

object query_target()
{
   return target;
}

object *query_targets()
{
   return ({target}) + other_targets;
}

/* Find someone to attack.  Return zero if we're dead or asleep or
 * have no one to attack.
 */
//: FUNCTION get_target
// Get someone to attack from our attackers list
object get_target()
{
   int x, n = sizeof(other_targets);

#ifdef TARGETTING_IS_RANDOM
   if (!explicit)
   {
      object tmp;
      x = random(n + 1);
      if (x < n) // target == n means keep the current target
      {
         tmp = other_targets[x];
         other_targets[x] = target;
         target = tmp;
      }
   }
   else
      explicit = 0;
#endif
   // Make sure they are alive and in the same room as us.  If not, find
   // someone else.
   while (!target || target->query_ghost() ||
          (member_array(target, deep_useful_inv(parser_root_environment(environment(this_object())))) == -1))
   {
      if (!n)
         return (target = 0);
      x = random(n);
      target = other_targets[x];
      other_targets[x..x] = ({});
      n--;
   }
   return target;
}

//: FUNCTION switch_to
// Make the specified target the primary target
void switch_to(object who)
{
#ifdef TARGETTING_IS_RANDOM
   explicit = 1;
#endif
   if (who == target)
      return;
   if (target)
      other_targets += ({target});
   other_targets -= ({who});
   target = who;
}

//: FUNCTION stop_hitting_me
// remove the previous object from the target list
void stop_hitting_me()
{
   if (previous_object() == target)
      target = 0;
   other_targets -= ({previous_object()});
}

/* This stops attacking "who".  It also takes care of stoping them
 * from attacking us.  stop_fight() or stop_fight(0) stops all
 * combat.
 */
//: FUNCTION stop_fight
// stop fighting with object ob (handles making sure they stop fighting
// with us too).  stop_fight(0) stops all fights.
varargs void stop_fight(object who)
{
   if (!who || undefinedp(who))
   {
      if (target)
         target->stop_hitting_me();
      other_targets->stop_hitting_me();
      target = 0;
      other_targets = ({});
      stop_attacking();
      return;
   }

   if (who == target)
      target = 0;
   else
      other_targets -= ({who});
   who->stop_hitting_me();
   if (query_targets() == ({0}))
   {                    // If there are no targets left,
      stop_attacking(); // remove the heart_beat().
   }
}
