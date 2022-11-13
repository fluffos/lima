/* Do not remove the headers from this file! see /USAGE for more info. */

/*
** crafting_d.c -- Crafting, materials, salvage and repairs.
** This daemon manages materials, loot, salvage, upgrade and crafting.
**
** Tsath 2019-10-14
**
*/

inherit M_DAEMON_DATA;
inherit M_DICE;
#define PRIV_NEEDED "Mudlib:daemons"
#define CRAFTING_ITEMS "/domains/std/crafting/"

nosave int *repair_values = ({10, 100, 1000, 10000, 100000});
nosave string *cache_names = ({});
private
string currency_type = "gold";
private
nosave mapping special_mats = (["metal":([128:"steel", 64:"darksteel", 32:"silver", 16:"gold", 8:"platinum", 4:"titanium", 2:"adamantine", 1:"orichalcum", ]),
                                 "wood":([128:"fir", 64:"pine", 32:"oak", 16:"cedar", 8:"larch", 4:"hemlock", 2:"ebony", 1:"bloodwood", ]),
]);

// Should be moved to DAMAGE_D
private
nosave mapping special_attk = (["acid":([128:"sourness", 64:"bitterness", 32:"tartness", 4:"astringent bite", 1:"corrosive jaws", ]),
                                "cold":([128:"chill", 64:"frost", 32:"crystals", 4:"frigid freeze", 1:"arctic cold", ]),
                                "fire":([128:"glow", 64:"embers", 32:"flames", 4:"blaze", 1:"infernal fire", ]),
                               "force":([128:"pushing", 64:"thrusting", 32:"bursting", 4:"blasting", 1:"impetuous force", ]),
                           "lightning":([128:"light", 64:"sparks", 32:"volt", 4:"electricity", 1:"lightning storm", ]),
                            "necrotic":([128:"nausea", 64:"pestilence", 32:"rotting", 4:"decay", 1:"ungraceful death", ]),
                              "poison":([128:"toxicity", 64:"miasma", 32:"blight", 4:"contagion", 1:"pandemic plague", ]),
                             "psychic":([128:"spirit", 64:"obscura", 32:"mysticism", 4:"shadow", 1:"etheral duress", ]),
                             "radiant":([128:"sparkling", 64:"lucent", 32:"glowing", 4:"resplending", 1:"light of the stars", ]),
                             "thunder":([128:"thunderclap", 64:"thunderbolt", 32:"thundercrack", 4:"thunderous barrage", 1:"thunderous explosion", ])]);
private
nosave string *rn = ({"I", "II", "III", "IV", "V", "VI", "VII", "IIX", "IX", "X", "XI", "XII", "XIII", "XIV", "XV"});
private
nosave int *mat_sums = ({128, 192, 224, 240, 248, 252, 254, 255});
private
nosave int *attk_sums = ({128, 192, 224, 228, 229});
private
nosave mapping crafting_recipes = ([]);
private
nosave mapping recipe_file = ([]);

/* Saved variables */
mapping materials = (["skin":({
                                 "tuft",
                                 "piece",
                                 "patch",
                                 "square",
                                 "leather",
                             }),
                     "metal":({
                                 "grain",
                                 "splinter",
                                 "fragment",
                                 "part",
                                 "bar",
                             }),
                      "wood":({
                                 "grain",
                                 "splinter",
                                 "fragment",
                                 "part",
                                 "log",
                             }),
]);

mapping upgrade_mat_schemes = (["metal":(["adamantine":({
                                                           "advanced metal kit",
                                                           "metal weapon soul",
                                                           "metal weapon core",
                                                           "metal weapon heart",
                                                       }),
                                             "platinum":({
                                                            "precious metal kit",
                                                            "metal weapon core",
                                                        }),
                                                "steel":({
                                                            "simple metal kit",
                                                        }),
                                             "titanium":({
                                                            "advanced metal kit",
                                                            "metal weapon soul",
                                                        }),
                                               "silver":({
                                                            "rare metal kit",
                                                            "metal weapon heart",
                                                        }),
                                            "darksteel":({
                                                            "simple metal kit",
                                                            "metal weapon heart",
                                                        }),
                                                 "gold":({
                                                            "rare metal kit",
                                                            "metal weapon core",
                                                        }),
])]);

/* Functions */

void load_crafting_recipes()
{
    string *files = get_dir(CRAFTING_ITEMS "*.c");
    string contents;
    string *ar_contents;
    string err;

    foreach (string item in files)
    {
        string crafting_str;
        object obj;
        if (err = catch (obj = load_object(CRAFTING_ITEMS + item)))
        {
            write("Error while loading crafting item:\n" + err + "\n");
            continue;
        }

        contents = read_file(CRAFTING_ITEMS + item);
        if (strlen(contents))
        {
            int found = 0;
            ar_contents = explode(contents, "\n");
            foreach (string line in ar_contents)
            {
                if (line[0..10] == "//CRAFTING:")
                {
                    crafting_str = trim(line[11..]);
                }
            }
        }

        if (!crafting_str)
        {
            write("Missing //CRAFTING: string for '" + item + "'.\n");
            continue;
        }

        recipe_file[crafting_str] = item;
        crafting_recipes[crafting_str] = obj->query_recipe();
    }
    save_me();
}

varargs mapping query_crafting_recipes(string filter)
{
    mapping new_map = ([]);
    if (!filter)
        return crafting_recipes;

    foreach (string key in filter_array(keys(crafting_recipes), (
                                                                    : strsrch($1, $(filter)) != -1
                                                                    :)))
    {
        new_map[key] = crafting_recipes[key];
    }
    return new_map;
}

mapping query_crafting_recipe(string name)
{
    return crafting_recipes[name];
}

int valid_crafting_recipe(string name)
{
    return crafting_recipes[name] != 0;
}

mixed *query_special_mats(string category)
{
    return ({special_mats[category], mat_sums});
}

mixed *query_all_special_mats()
{
    return ({special_mats, mat_sums});
}

mixed *query_special_attk()
{
    return ({special_attk, attk_sums});
}

string *query_roman_numerals()
{
    return rn;
}

mapping query_upgrade_schemes()
{
    return upgrade_mat_schemes;
}

string *query_upgrade_scheme(string category, string material)
{
    if (upgrade_mat_schemes[category] && upgrade_mat_schemes[category][material])
        return upgrade_mat_schemes[category][material];
    return 0;
}

private
void refresh_cache()
{
    cache_names = ({});
    foreach (string category in keys(materials))
    {
        foreach (string m in materials[category])
            cache_names += ({category + " " + m});
    }
}

void clean_up()
{
    destruct();
}

int valid_category(string c)
{
    return member_array(c, keys(materials)) != -1;
}

int valid_material(string m)
{
    return member_array(m, cache_names) != -1;
}

int add_category(string category, string *m)
{
    if (valid_category(category))
        return 0;

    if (!materials[category])
        materials[category] = m;
    foreach (string mat in m)
        cache_names += ({category + " " + mat});
    save_me();
    return 1;
}

string *query_salvage_categories()
{
    return keys(materials);
}

int remove_category(string c)
{
    if (!valid_category(c))
        return 0;

    map_delete(materials, c);
    refresh_cache();
    save_me();
    return 1;
}

string *query_material_categories()
{
    return keys(materials);
}

string *query_materials(string category)
{
    return materials[category];
}

object create_material(string m)
{
    object mat;

    if (recipe_file[m])
    {
        mat = new ("/domains/std/crafting/" + recipe_file[m]);
        return mat;
    }
    else if (!valid_material(m))
        return 0;

    if (!mat)
        mat = new (MATERIAL, m);
    return mat;
}

string material_below(string category, string material)
{
    string *mats = query_materials(category);
    int pos;
    if (!sizeof(mats))
        return 0;
    pos = member_array(material, mats);
    pos--;

    if (pos >= 0)
    {
        return category + " " + mats[pos];
    }
    return 0;
}

string material_above(string category, string material)
{
    string *mats = query_materials(category);
    int pos;
    if (!sizeof(mats))
        return 0;
    pos = member_array(material, mats);
    pos++;

    if (pos < (sizeof(mats)))
        return category + " " + mats[pos];
    return 0;
}

object create_material_for_me(string m)
{
    create_material(m)->move(this_body());
}

string salvage_description(object *s)
{
    string *parts = ({});

    if (!sizeof(s))
        return "nothing";

    foreach (object ob in s)
    {
        parts += ({ob->short()});
    }
    return tidy_list(parts);
}

void save()
{
    save_me();
    write("Saved.\n");
}

void salvage_obj(object ob)
{
    mapping salvage = ob->query_salvageable();
    object *salvaged = ({});
    string picked_material;
    int s_roll = random(100) + 1;
    int total = 0;
    foreach (string key in keys(salvage))
    {
        if (picked_material)
            continue;
        total += salvage[key];
        if (s_roll <= total)
        {
            picked_material = key;
        }
    }

    for (int i = 0; i < ob->query_salvage_level(); i++)
    {
        int index = -1;
        s_roll = random(100) + 1 + ob->query_salvage_level();
        if (s_roll > 100)
            s_roll = 100;

        switch (s_roll)
        {
        case 1..30:
            index = -1;
            break;
        case 31..65:
            index = 0;
            break;
        case 66..82:
            index = 1;
            break;
        case 83..94:
            index = 2;
            break;
        case 95..98:
            index = 3;
            break;
        case 99..100:
            index = 4;
            break;
        }
        if (index >= 0)
            salvaged += ({create_material(picked_material + " " + materials[picked_material][index])});
    }

    // TBUG("Salvaged: "+sprintf("%O",salvaged));

    printf("You salvage %s from the %s.\n",
           salvage_description(salvaged), ob->short());

    foreach (object mat in salvaged)
    {
        this_body()->add_material(mat);
    }
    ob->remove();
}

float money_to_repair(object ob)
{
    return ob->missing_durability() / 100.0;
}

string repair_cost_string(object ob)
{
    return MONEY_D->currency_to_string(money_to_repair(ob), "gold");
}

mapping estimate_objects(object player, object *obs)
{
    mapping pouch = copy(player->query_materials());
    mapping picked_mats = ([]);
    int remainder = 0;
    if (!arrayp(obs))
        obs = ({obs});

    foreach (object ob in obs)
    {
        mapping salvage = ob->query_salvageable();
        int missing_dura = ob->missing_durability();
        int rep_size = sizeof(repair_values) - 1;
        if (!salvage)
            continue;

        foreach (string mat in keys(salvage))
        {
            int needed = salvage[mat] * missing_dura / 100;
            if (!arrayp(picked_mats[mat]))
                picked_mats[mat] = ({0, 0, 0, 0, 0});
            for (int i = rep_size; i >= 0; i--)
            {
                if (repair_values[i] < needed)
                {
                    int picked;
                    picked = CLAMP(needed / repair_values[i], 0, pouch[mat + " " + materials[mat][i]]);
                    picked_mats[mat][i] += picked;
                    needed -= picked * repair_values[i];
                    TBUG("Now picked " + picked + " of " + materials[mat][i]);
                    pouch[mat + " " + materials[mat][i]] -= picked;
                    if (i == 0 && needed != 0 && picked_mats[mat][i] < pouch[mat + " " + materials[mat][i]])
                    {
                        TBUG("Added one extra " + mat + " " + materials[mat][i] + " to cover the remainder (" + needed + ").");
                        picked_mats[mat][i]++;
                        needed = 0;
                    }
                }
            }
            remainder += needed;
        }
    }
    picked_mats["money"] = remainder / 100.0;
    return picked_mats;
}

void set_materials(mapping m)
{
    materials = m;
    save_me();
}

void set_upgrade_schemes(mapping m)
{
    upgrade_mat_schemes=m;
    save_me();
}

int pay_for_repair_with_money(object player, mixed obs)
{
    float total_cost = 0;
    int all_good = 1;
    mapping money;

    if (objectp(obs))
        obs = ({obs});

    foreach (object ob in obs)
    {
        total_cost += ob->missing_durability() / 100.0;
    }

    if (total_cost && player->query_amt_currency(currency_type) > total_cost)
    {

        money = MONEY_D->handle_subtract_money(player, total_cost, currency_type);
        player->my_action("$N $vbuy repairs for " + MONEY_D->currency_to_string(total_cost, currency_type) +
                          ". You give " + MONEY_D->currency_to_string(money[0], currency_type) + ", " +
                          (sizeof(money[1]) ? " and get " + MONEY_D->currency_to_string(money[1], currency_type) + " as change" : "") + ".\n");
        all_good = 1;
    }

    if (all_good)
    {
        obs->durability_after_repair();
        obs->remove_adj("tattered");
    }
    return all_good;
}

int pay_for_repair(object player, mixed obs)
{
    mapping estimate = estimate_objects(player, obs);
    mapping pouch = player->query_materials();
    int all_good = 1;
    mapping money;
    int rep_size = sizeof(repair_values) - 1;

    foreach (string mat in keys(estimate))
    {
        if (mat == "money" || !all_good)
            continue;
        for (int i = rep_size; i >= 0; i--)
        {
            string material = mat + " " + materials[mat][i];
            if (!player->remove_material(material, estimate[mat][i]))
            {
                TBUG("Could not find " + material + " / " + estimate[mat][i]);
                all_good = 0;
            }
        }
    }
    if (estimate["money"] && player->query_amt_currency(currency_type) > (estimate["money"] / 100.0))
    {
        money = MONEY_D->handle_subtract_money(player, estimate["money"], currency_type);
        player->my_action("$N $vbuy repairs for " + MONEY_D->currency_to_string(estimate["money"], currency_type) +
                          ". You hand over " + MONEY_D->currency_to_string(money[0], currency_type) + "" +
                          (sizeof(money[1]) ? " and get " + MONEY_D->currency_to_string(money[1], currency_type) + " as change" : "") + ".\n");
        all_good = 1;
    }

    if (all_good)
    {
        obs->durability_after_repair();
        obs->remove_adj("tattered");
    }
    return all_good;
}

varargs string *buy_durability(mixed m, int rest)
{
    if (intp(m) || floatp(m))
        return MONEY_D->money_to_array((m - rest) / 100.0);
    if (objectp(m))
        return MONEY_D->money_to_array((m->missing_durability() - rest) / 100.0);
    if (arrayp(m))
    {
        float total_dura = 0.0;
        foreach (object ob in m)
            total_dura += ob->missing_durability();
        total_dura -= rest;
        return MONEY_D->currency_to_string(total_dura / 100.0, currency_type);
    }
}

mixed estimate_with_mats(object player, mixed obs)
{
    string returnStr = "";
    int possible = 1;
    int mats_used = 0;
    mapping picked_mats = estimate_objects(player, obs);
    mapping pouch = player->query_materials();
    int rep_size = sizeof(repair_values) - 1;
    if (objectp(obs))
        obs = ({obs});

    returnStr += "Repair estimate for: ";
    foreach (object ob in obs)
    {
        returnStr += capitalize(ob->short()) + ", ";
    }
    returnStr = returnStr[0.. < 3] + "\n\n";

    foreach (string mat in keys(picked_mats))
    {
        string mat_str = "";
        if (mat == "money")
            continue;
        for (int i = rep_size; i >= 0; i--)
        {
            if (picked_mats[mat][i])
            {
                mat_str += "[" + picked_mats[mat][i] + "/" + pouch[mat + " " + materials[mat][i]] + "] " + materials[mat][i] +
                           (picked_mats[mat][i] != 1 ? "s" : "") +
                           ", ";
                mats_used = 1;
            }
        }

        if (strlen(mat_str))
        {
            returnStr += sprintf("%-5s: ", capitalize(mat)) + mat_str;
            returnStr = returnStr[0.. < 3] + "\n";
        }
    }

    if (picked_mats["money"] && player->query_amt_currency(currency_type) > (picked_mats["money"] / 100.0))
        returnStr += "Money: " + MONEY_D->currency_to_string(picked_mats["money"], currency_type) + ".\n";
    if (picked_mats["money"] && player->query_amt_currency(currency_type) < (picked_mats["money"] / 100.0))
    {
        returnStr += "Money: " + format_list(buy_durability(picked_mats["money"])) + " %%^RED%%^(You lack money!)%%^RESET%%^\n";
        possible = 0;
    }

    returnStr += (mats_used ? "(You save " + format_list(buy_durability(obs, picked_mats["money"])) + " by using materials)" : "(Salvage weapons and armors to get materials for cheaper repairs)") + "\n\n";

    return ({possible, returnStr});
}

int add_upgrade_scheme(string category, string special_material, string *recipes)
{
    int halt = 0;

    if (!valid_category(category))
    {
        write("Invalid salvage category '" + category + "'.\n");
        return 0;
    }

    if (member_array(special_material, values(special_mats[category])) == -1)
    {
        write("Invalid special material '" + special_material + "'.\n");
        return 0;
    }

    if (!sizeof(recipes))
    {
        write("Invalid empty recipe list.\n");
        return 0;
    }

    foreach (string recipe in recipes)
    {
        if (!query_crafting_recipe(recipe))
        {
            write("Unknown recipe '" + recipe + "'.\n");
            halt = 1;
        }
    }
    if (halt)
        return;

    if (!upgrade_mat_schemes[category])
        upgrade_mat_schemes[category] = ([]);
    upgrade_mat_schemes[category][special_material] = recipes;

    save_me();
    return 1;
}

void stat_me()
{
    foreach (string category in keys(materials))
    {
        write("Category: " + category + "\n");
        write(" " + format_list(materials[category]));
        write("\n");
    }
    write("\n\nCrafting recipes: \n");
    write("  " + implode(sort_array(keys(crafting_recipes), 1), "\n  "));
    write("\n");

    write("\nUpgrade schemes: \n");
    foreach (string name, mapping m in upgrade_mat_schemes)
    {
        write("CATEGORY: " + name + "\n");
        foreach (string name2, string * sar in m)
        {
            write(" [" + name2 + "]\n\t" + implode(sar, ",") + "\n");
        }
        write("\n");
    }
}

void create()
{
    ::create();
    refresh_cache();
    load_crafting_recipes();
}
