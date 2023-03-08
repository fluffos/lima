/* Do not remove the headers from this file! see /USAGE for more info. */

inherit BODY;

#include <stats.h>

/*
 * Ok, quick explanation:
 *
 * FOO_BAR_FACTOR gives the percentage of FOO that is based on BAR.
 *
 * derived stats are based on the main stats, plus a portion which
 * depends on race, which is scaled by an amount which is the racial
 * bonus.
 *
 * Derive the coefficients of the transformation, to make things easier
 * later.
 */
/*
** Effects of the base stats on the derived stats
*/

int racial_con_bonus()
{
}

int racial_wis_bonus()
{
}

int racial_cha_bonus()
{
}

int racial_man_bonus()
{
}

nomask int *query_transformation_matrix()
{
   if (clonep())
      return base_name()->query_transformation_matrix();

   return ({
       map(({CON_STR_FACTOR, CON_AGI_FACTOR, 0, CON_WIL_FACTOR}), (
                                                                      : $1 * $(100 - racial_con_bonus())
                                                                      :)),
       map(({WIS_STR_FACTOR, WIS_AGI_FACTOR, WIS_INT_FACTOR, WIS_WIL_FACTOR}),
           (
               : $1 * $(100 - racial_wis_bonus() - WIS_SKILL_FACTOR)
               :)),
       map(({CHA_STR_FACTOR, CHA_AGI_FACTOR, CHA_INT_FACTOR, CHA_WIL_FACTOR}),
           (
               : $1 * $(100 - racial_cha_bonus() - CHA_SKILL_FACTOR)
               :)),
       map(({CHA_STR_FACTOR, 0, CHA_INT_FACTOR, CHA_WIL_FACTOR}),
           (
               : $1 * $(100 - racial_man_bonus() - MAN_SKILL_FACTOR)
               :)),
   });
}

nomask int *query_constant_vector()
{
   if (clonep())
      return base_name()->query_constant_vector();

   return ({racial_con_bonus(), racial_wis_bonus(), racial_cha_bonus(), racial_man_bonus()});
}

// A default query_race, just in case it is forgotten
string query_race()
{
   error("No valid query_race() defined...\n");
}
