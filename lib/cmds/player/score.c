/* Do not remove the headers from this file! see /USAGE for more info. */

/*
** score.c -- print out player stats
**
** Tsath, created.
*/

//: PLAYERCOMMAND
//$$ see: stats
// USAGE score
//
// Shows you various details about yourself.

#include <config.h>
#include <hooks.h>

inherit CMD;
inherit M_WIDGETS;
inherit M_ANSI;

// Always returns a strlen 6.
string pretty_bonus(int b)
{
    string out = "      ";
    if (b > 0)
        out = "(<002>+" + b + "<res>)";
    else if (b < 0)
        out = "(<001>" + b + "<res>)";

    if (colour_strlen(out) < 6)
        out += repeat_string(" ", 6 - colour_strlen(out));

    return out;
}

private
void main(string arg)
{
    string name;
    object body;
    string e_info = 0;
    string *guilds;
    string g_info = 0;
    string r_info = "";
    string x_info = "";
#ifdef USE_KARMA
    string k_info = "";
#endif
    string l_info = "";
    string o_info;
    string FC = "<bld>%-10.10s<res>"; // First column all the way down
    string *curr;
    int xp = 1;
    mapping accounts;
    int i;
    int width = default_user_width() - 11;
    body = this_body();

    if (strlen(arg) > 0 && wizardp(this_user()))
    {
        body = find_body(arg);
        if (!body)
        {
            out("Cannot find '" + arg + "'.\n");
            return;
        }
        write("Score for " + capitalize(arg) + ":\n");
    }

    xp = body->query_experience();
    write(simple_divider());

#ifdef USE_TITLES
    name = body->query_title();
#else
    name = body->query_name();
#endif
    write(sprintf("%s  (%s) - Level %d\n", name, wizardp(body->query_link()) ? "Wizard" : "Mortal", body->query_level()));

    write(simple_divider());

    e_info = sprintf(FC, "Exp");
    write(e_info + green_bar(xp - body->xp_for_level(body->query_level()), body->query_next_xp() - body->query_xp_for_level(body->query_level()), width) + "\n");
    if (body->query_next_xp() - body->query_experience() > 0)
        write(sprintf(FC + "%d XP - Level %d in %d more points.\n", "",
                      body->query_experience(),
                      (body->query_level() + 1),
                      (body->query_next_xp() - body->query_experience())));
    else
        write(sprintf(FC + "%d XP - You could be level %d.\n", "",
                      body->query_experience(),
                      body->query_could_be_level()));

    write(simple_divider());

    o_info = "";
    curr = body->query_currencies();
    if (!sizeof(curr))
    {
        o_info += "You are carrying no money.";
    }
    else
    {
        for (i = 0; i < sizeof(curr); i++)
        {
            mapping money = MONEY_D->calculate_denominations(body->query_amt_money(curr[i]), curr[i]);
            string *money_str = ({});
            foreach (string denom, int count in money)
                money_str += ({count + " " + denom + (count == 1 ? "" : "s")});

            o_info += sprintf(FC + "%s",
                              i == 0 ? "Money" : "",
                              //(curr[i] == "dollar" ? "$" : capitalize(curr[i])),
                              format_list(money_str));
        }
    }
    write(o_info + "\n");
    o_info = "";
    accounts = ACCOUNT_D->query_accounts(body);
    if (!sizeof(accounts))
    {
        o_info += "";
    }
    else
    {
        i = 0;
        foreach (string bank, float val in accounts)
        {
            o_info += sprintf(FC + "$ %d (in %s Bank)",
                              "",
                              to_int(val),
                              capitalize(bank));
            i++;
        }
    }
    if (strlen(o_info))
        write(o_info + "\n");
    write(simple_divider());

#ifdef USE_RACES
    r_info = "  Race: " + capitalize(body->query_race());
#endif

    x_info = "  Exp: " + body->query_experience();

    l_info = "  Level: " + body->query_level();

    write(sprintf("" +
                      FC + "Strength     %-4d%s  Agility   %-4d%s  Intelligence %-4d%s\n" +
                      FC + "Constitution %-4d%s  Charisma  %-4d%s\n",
                  "Stats",
                  body->query_str(), (pretty_bonus(body->query_str() - body->query_str_pure())),
                  body->query_agi(), (pretty_bonus(body->query_agi() - body->query_agi_pure())),
                  body->query_int(), (pretty_bonus(body->query_int() - body->query_int_pure())), "",
                  body->query_con(), (pretty_bonus(body->query_con() - body->query_con_pure())),
                  body->query_cha(), (pretty_bonus(body->query_cha() - body->query_cha_pure()))));

    write(sprintf(FC + "%d of %d used %s\n", "Points",
                  body->query_stat_points(),
                  body->query_stat_points_max(),
                  (body->query_stat_points() < body->query_stat_points_max() ? "- visit a trainer!" : "")));
    write(simple_divider());
    write(sprintf(FC + "XP Buff: %s\n",
                  "Other",
                  (body->query_guild_xp_buff() ? "Guild buff " + body->query_guild_xp_buff() + "%" : "None")));
    write(simple_divider());

#ifdef USE_KARMA
    if (!i_simplify())
    {
        int karma = this_body()->query_karma();
        int marker = 65 * ((karma + 1000) / 2000.0);
        int marker2 = 0;
        k_info = repeat_string("-", marker) + "X" + repeat_string("-", 65 - marker);
        k_info = k_info[0..31] + "<002>" + k_info[32..];
        if (marker < 31)
            k_info = replace_string(k_info, "X", "<bld>X<res><001>");
        else
            k_info = replace_string(k_info, "X", "<bld>X<res><002");

        out("Karma    " + slider_red_green(karma, 2000, width) + "\n");
    }

    if (1)
    {
        int karma = this_body()->query_karma();
        string status = "";
        switch (karma)
        {
        case 1000:
            status = "Good incarnate.";
            break;
        case 901..999:
            status = "Fantastic karma. <3 <3";
            break;
        case 501..900:
            status = "Great karma. Everybody loves you.";
            break;
        case 201..500:
            status = "Good karma. Flowers sprout where you walk.";
            break;
        case 0..200:
            status = "Slightly good karma.";
            break;
        case -200.. - 1:
            status = "Slightly bad karma.";
            break;
        case -500.. - 201:
            status = "Quite Bad karma. You will soon provoke attacks from good people.";
            break;
        case -900.. - 501:
            status = "Very Bad karma. You will likely provoke attacks.";
            break;
        case -999.. - 901:
            status = "Extremely bad karma. You will be attacked on sight.";
            break;
        case -1000:
            status = "Evil incarnate.";
            break;
        }

        if (wizardp(this_body()))
            outf("         %s (%d score)\n", status, karma);
        else
            outf("         %s\n", status);
    }
#endif


    if (1)
    {
        int capa = this_body()->query_capacity();
        int enc_capa = this_body()->query_encumbered_capacity();
        int enc_heavy_capa = this_body()->query_heavy_capacity();
        int no_move = this_body()->query_no_move_capacity();
        int max = this_body()->query_max_capacity();
        string capa_string;

        if (capa < enc_capa)
            capa_string = "Unencumbered";
        else if (capa < enc_heavy_capa)
            capa_string = "Encumbered";
        else if (capa < max)
            capa_string = "Heavily encumbered";
        else
            capa_string = "Unable to move";

        write("Weight    " + slider_colours_sum(capa, ([enc_capa:"<002>",
                                                          enc_heavy_capa:"<003>", no_move:"<001>", max:"<004>"]),
                                                width) +
              "\n");
        write("          Carrying " + weight_to_string(capa, get_user_variable("metric") != 1) + " / " +
              weight_to_string(enc_capa, get_user_variable("metric") != 1) +
              " - " + capa_string + ".\n");
        write(simple_divider());
    }
}
