/* Do not remove the headers from this file! see /USAGE for more info. */

/* cleaned up and adjusted to denominations 10-Feb-98 by MonicaS
   Tsath: Rewritten to fit with floats.
   */

inherit OBJ;
inherit M_GETTABLE;
inherit M_MESSAGES;

private
mapping coins = ([]);

string long_descr()
{
   string *descr = ({});
   foreach (string currency in keys(coins))
   {
      mapping denom = MONEY_D->calculate_denominations(coins[currency], currency);
      foreach (string denom_name, int value in denom)
      {
         descr += ({MONEY_D->denomination_to_string(value, denom_name)});
      }
   }

   return "A pile of coins consisting of " + format_list(descr) + ".";
}

private
float query_amt_coins(string type)
{
   return coins[type];
}

string *query_coins()
{
   return keys(coins);
}

void split_coins(float amount, string type)
{
   if (coins[type] >= amount)
   {
      this_body()->add_money(type, amount);
      coins[type] -= amount;
      this_body()->simple_action("$N $vtake some coins.");
      if (coins[type] <= 0)
      {
         map_delete(coins, type);
         if (!sizeof(query_coins()))
            remove();
      }
   }
   else
      write("There aren't that many coins of that type here.\n");
}

mixed get(string amount_str, string type)
{
   string denomination;
   int amount;
   string *types;

   if (!amount_str)
   {
      foreach (denomination, amount in coins)
      {
         this_body()->add_money(denomination, amount);
      }
      this_body()->simple_action("$N $vtake the pile of coins.");
      remove();
      return "";
   }
   else
   {
      if (type == "coins" || type == "coin")
      {
         types = query_coins();
         if (sizeof(types) == 1)
         {
            amount = to_int(amount_str);
            split_coins(amount, types[0]);
         }
         else
            write("Which type of coins do you want to get?\n");
      }
      else if (!coins[type])
         write("There are no coins of that type here.\n");
      else
      {
         if (amount_str == "all")
            split_coins(coins[type], type);
         else
            split_coins(to_int(amount_str), type);
      }
   }
}

void merge_coins(float amount, string type)
{
   this_object()->add_id(type);
   coins[type] = coins[type] + amount;
}

void setup(float amount, string type)
{
   coins[type] = amount;
   set_id("coins", type, "money", "coin", "pile", "pile of coins");
   add_adj(type);
   set_in_room_desc("A pile of coins");
   set_proper_name("coins");
   set_long(( : long_descr:));
   set_gettable(1);
   set_plural(1);
}
