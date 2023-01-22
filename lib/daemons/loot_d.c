/*
** loot_d.c -- Controls loot drops mud-wide
**
** Tsath 2020-06-10 Doles out loot for monster level 1-100.
**
*/

string currency_type = "gold";

//:FUNCTION roll_dice
// Roll x number of dice with y sides.
// Not all gaming systems use just the total of the dice, so each result is
// returned in an array.
int *roll_dice(int num_dice, int num_sides)
{
    int i;
    int *results = ({});

    for (i = 0; i < num_dice; i++)
    {
        results += ({random(num_sides) + 1});
    }
    return results;
}

//:FUNCTION dice
// int dice(int d)
// Roll a single die and return the result
int dice(int d)
{
    return random(d) + 1;
}

//:FUNCTION x_dice
// int x_dice(int n, int d)
// Return the sum of n dice with d sides .
int x_dice(int n, int d)
{
    int total = 0;

    for (int i = 0; i < n; i++)
        total += dice(d);
    return total;
}

object *money_treasure(int level)
{
    object *money = ({});
    int p = random(100) + 1;
    level += 5; //Up the drops a bit.

    switch (level)
    {
    case 1..16:
        switch (p)
        {
        case 1..30:
            money += ({new (MONEY, x_dice(1, 6) * (level / 16.0), currency_type)});
            break;
        case 31..60:
            money += ({new (MONEY, x_dice(2, 6) * (level / 16.0), currency_type)});
            break;
        case 61..70:
            money += ({new (MONEY, x_dice(3, 6) * (level / 16.0), currency_type)});
            break;
        case 71..95:
            money += ({new (MONEY, x_dice(4, 6) * (level / 16.0), currency_type)});
            break;
        case 96..100:
            money += ({new (MONEY, x_dice(6, 6) * (level / 16.0), currency_type)});
            break;
        }
        break;
    case 17..40:
        switch (p)
        {
        case 1..30:
            money += ({new (MONEY, x_dice(5, 6) * ((level - 16) / 23.0), currency_type)});
            break;
        case 31..60:
            money += ({new (MONEY, x_dice(6, 6) * ((level - 16) / 23.0), currency_type)});
            break;
        case 61..70:
            money += ({new (MONEY, x_dice(7, 6) * ((level - 16) / 23.0), currency_type)});
            break;
        case 71..95:
            money += ({new (MONEY, x_dice(8, 6) * ((level - 16) / 23.0), currency_type)});
            break;
        case 96..100:
            money += ({new (MONEY, x_dice(10, 6) * ((level - 16) / 23.0), currency_type)});
            break;
        }
        break;
    case 41..64:
        switch (p)
        {
        case 1..20:
            money += ({new (MONEY, x_dice(4, 6) * 10 * ((level - 40) / 23.0), currency_type)});
            break;
        case 21..35:
            money += ({new (MONEY, x_dice(8, 6) * 20 * ((level - 40) / 23.0), currency_type)});
            break;
        case 36..75:
            money += ({new (MONEY, x_dice(12, 6) * 100 * ((level - 40) / 23.0), currency_type)});
            break;
        case 76..100:
            money += ({new (MONEY, x_dice(16, 6) * 10 * ((level - 40) / 23.0), currency_type)});
            break;
        }
        break;
    case 65..80:
        switch (p)
        {
        case 1..15:
            money += ({new (MONEY, x_dice(2, 6) * 200 * ((level - 40) / 23.0), currency_type)});
            break;
        case 16..55:
            money += ({new (MONEY, x_dice(3, 6) * 1000 * ((level - 40) / 23.0), currency_type)});
            break;
        case 56..100:
            money += ({new (MONEY, x_dice(4, 6) * 1000 * ((level - 40) / 23.0), currency_type)});
            break;
        }
        break;
    case 81..100:
        switch (p)
        {
        case 1..15:
            money += ({new (MONEY, x_dice(2, 6) * 400 * ((level - 40) / 23.0), currency_type)});
            break;
        case 16..55:
            money += ({new (MONEY, x_dice(3, 6) * 2000 * ((level - 40) / 23.0), currency_type)});
            break;
        case 56..100:
            money += ({new (MONEY, x_dice(4, 6) * 4000 * ((level - 40) / 23.0), currency_type)});
            break;
        }
        break;
    }

    if (!money[0]->query_amt_money("dollars"))
        return 0;

    return money;
}

void drop_corpse(object adversary)
{
    string corpse_filename = CORPSE;
    string corpse_long = adversary->query_corpse_long();
    string in_room_singular = adversary->in_room_singular();
    string in_room_plural = adversary->in_room_plural();
    //Handling of players dropping corpses
    if (adversary->is_body())
    {
        object corpse = new (corpse_filename, adversary->query_name(), corpse_long);

        corpse->override_room_desc(in_room_singular, in_room_plural);
        corpse->move(environment(adversary));
    }
    else //everybody else
    {
        object corpse = new (corpse_filename, adversary->query_name(), corpse_long);
        object death_location = environment(adversary);
        string meaty = adversary->query_drops_meat();
        adversary->move(HEAVEN);
        corpse->override_room_desc(in_room_singular, in_room_plural);
        if (sizeof(all_inventory(adversary)))
        {
            object *moneys = money_treasure(adversary->query_level());
            all_inventory(adversary)->set_worn(0);
            all_inventory(adversary)->move(corpse);
            if (moneys)
                moneys->move(corpse);
        }
        if (meaty)
        {
            object meat = new (MEAT, meaty, adversary->query_level());
            meat->move(corpse);
        }

        corpse->move(death_location);
    }
}