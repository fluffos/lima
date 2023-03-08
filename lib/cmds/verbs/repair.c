/* Do not remove the headers from this file! see /USAGE for more info. */

inherit VERB_OB;
inherit M_INPUT;

private
nomask void repair_with_what(mixed what, string r)
{
   string result;
   object bench = filter_array(all_inventory(environment(this_body())), ( : $1->is_crafting_bench() :));

   if (!bench || !sizeof(bench))
   {
      write("You need a crafting bench to repair.\n");
      return;
   }

   r = lower_case(r);

   if (r == "m")
   {
      printf("Repairing using money.\n");
      if (CRAFTING_D->pay_for_repair_with_money(this_body(), what))
         printf("All repaired.\n");
      else
      {
         printf("You didn't have the money?\n");
      }
   }
   else if (r == "r")
   {
      printf("Using materials for repairs.\n");
      if (CRAFTING_D->pay_for_repair(this_body(), what))
         printf("All repaired.\n");
      else
      {
         printf("You were missing parts?\n");
      }
   }
   else
   {
      printf("Aborting.\n");
   }
}

void do_repair_obj(object ob)
{
   object bench = filter_array(all_inventory(environment(this_body())), ( : $1->is_crafting_bench() :));

   if (!bench || !sizeof(bench))
   {
      write("You need a crafting bench to repair.\n");
      return;
   }

   if (!ob->missing_durability())
   {
      printf("The %s is in mint condition.\n", ob->short());
      return;
   }
   else
   {
      mixed *estimate = CRAFTING_D->estimate_with_mats(this_body(), ob);
      printf(estimate[1]);
      if (estimate[0])
      {
         printf("\t%%^MENU_CHOICE%%^r%%^RESET%%^: Use materials to Repair %s?\n" +
                    "\t%%^MENU_CHOICE%%^m%%^RESET%%^: Use only Money, " + CRAFTING_D->repair_cost_string(ob) +
                    ", instead?\n" + "\t%%^MENU_CHOICE%%^Q%%^RESET%%^: Quit (default)",
                ob->the_short());
         modal_simple(( : repair_with_what, ob:));
      }
      else
      {
         printf("You do not have the funds to repair %s.\n", ob->the_short());
      }
   }

   CRAFTING_D->repair_obj(ob);
}

void do_repair_obs(object *obs)
{
   object *filtered_obs = ({});
   mixed *estimate;
   float only_money = 0.0;
   object *bench = filter_array(all_inventory(environment(this_body())), ( : $1->is_crafting_bench() :));

   if (!bench || !sizeof(bench))
   {
      write("You need a crafting bench to repair.\n");
      return;
   }

   foreach (object ob in obs)
   {
      if ((environment(ob) == this_body() && ob->missing_durability()))
      {
         filtered_obs += ({ob});
         only_money += CRAFTING_D->money_to_repair(ob);
      }
   }
   if (!sizeof(filtered_obs))
   {
      write("You find nothing in need of repair.\n");
      return;
   }
   estimate = CRAFTING_D->estimate_with_mats(this_body(), filtered_obs);
   printf(estimate[1]);
   if (estimate[0])
   {
      printf("\t%%^MENU_CHOICE%%^r%%^RESET%%^: Use materials to Repair?\n"
             "\t%%^MENU_CHOICE%%^m%%^RESET%%^: Use only Money, " +
             CRAFTING_D->repair_cost_string(only_money) +
             ", instead?\n"
             "\t%%^MENU_CHOICE%%^Q%%^RESET%%^: Quit (default)");
      modal_simple(( : repair_with_what, filtered_obs:));
   }
   else
   {
      printf("You do not have the funds to repair the items.\n");
   }
}

void do_repair()
{
   write("Repair what?\n");
}

void create()
{
   add_rules(({"", "OBJ", "OBS"}), ({"fix", "glue", "patch", "plug"}));
}
