/* Do not remove the headers from this file! see /USAGE for more info. */

/* $Id: account_d.c,v 1.1 1998/01/27 23:20:44 monica Exp $
**
**  Expanded by Tsath 2020, 2021.
**  Now supports transaction logs and bank statements.
*/

inherit M_DAEMON_DATA;
inherit M_WIDGETS;

#define MAX_TRANSACTION_LOG 30
#define LOG_BANK 0
#define LOG_TEXT 1
#define LOG_MONEY 2
#define LOG_SALDO 3
#define LOG_TIME 4

// ({ “Deposit in Abacus Bank, southside”,500,1850, timestamp })
private
mapping accounts = ([]);
mapping transactions = ([]);

float query_account(string bank, mixed player)
{
  string name;
  if (objectp(player))
    name = player->query_link()->query_userid();
  else
    name = player;

  if (accounts[bank])
    return accounts[bank][name];
  else
    return 0;
}

private
void transaction(mixed player, string bank, string what, float money)
{
  string name;
  float saldo;
  if (!to_int(money))
    return;
  if (objectp(player))
    name = player->query_link()->query_userid();
  else
    name = player;
  saldo = query_account(bank, name);
  if (!transactions[name])
    transactions[name] = ({});
  if (sizeof(transactions[name]) > MAX_TRANSACTION_LOG)
    transactions[name] = transactions[name][5..];

  transactions[name] += ({({bank, what, money, saldo, time()})});
}

string fake_account_id(string bank, object body)
{
  string name = lower_case(body->query_name());
  return upper_case(bank[0..2]) + "-" + (bank[0] + name[0]) + "" + (bank[0] + name[1]) + "" + (bank[0] + name[2]);
}

string bank_statement(string bank, object body)
{
  string name = lower_case(body->query_name());
  string *out = ({"%^BLUE%^" + upper_case(bank + " bank") + sprintf("%60s", "Account #: " + fake_account_id(bank, body) + "%^RESET%^"),
                  "",
                  sprintf("%-35.35s  %-15s  %-15s  %-10s", "Description", "Withdrawals", "Deposits", "Balance"),
                  simple_divider()[0.. < 2]});
  if (!transactions[name])
    return "You have no account with " + bank + " Bank.\n";
  foreach (mixed *ar in transactions[name])
  {
    if (ar[LOG_BANK] == bank && ar[LOG_MONEY])
      out += ({sprintf("%-35.35s  %-15s  %-15s  %-10s",
                       ar[LOG_TEXT],
                       ar[LOG_MONEY] < 0 ? "" + (to_int(ar[LOG_MONEY]) * -1) : "",
                       ar[LOG_MONEY] >= 0 ? to_int(ar[LOG_MONEY]) + "" : "",
                       to_int(ar[LOG_SALDO]) + "", )});
  }
  out += ({simple_divider()[0.. < 2]});
  return implode(out, "\n");
}

mapping query_transactions()
{
  return transactions;
}

mapping query_account_data()
{
  return accounts;
}

void remove_bank(string bank)
{
  map_delete(accounts, bank);
  save_me();
}

mapping query_accounts(object player)
{
  mapping acm = ([]);
  foreach (string bank in keys(accounts))
  {
    float balance = query_account(bank, player);
    if (balance > 0.00)
      acm[bank] = balance;
  }
  return acm;
}

varargs void deposit(string bank, mixed player, float amount, string what)
{
  string name;
  if (objectp(player))
    name = player->query_link()->query_userid();
  else
    name = player;

  if (!accounts[bank])
    accounts[bank] = ([]);
  accounts[bank][name] =
      to_float(accounts[bank][name]) + amount;
  transaction(player, bank, what ? what : (amount >= 0 ? "Deposit into" : "Withdrawal from") + " account", amount);
  save_me();
}

varargs void withdraw(string bank, mixed player, float amount, string what)
{
  string name;
  if (objectp(player))
    name = player->query_link()->query_userid();
  else
    name = player;
  deposit(bank, player, -amount, what);
}

void set_account(string bank, object player, float amount)
{
  if (!accounts[bank])
    accounts[bank] = ([]);
  accounts[bank][player->query_userid()] = amount;
  save_me();
}

void clean_up()
{
  destruct(this_object());
}
