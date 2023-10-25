inherit TAMING_CHIP;

//CRAFTING:advanced combat taming chip

void setup()
{
    set_adj("advanced");
    set_armour_class(2);
    set_taming_level(15);
    set_combat_chip();
}

mapping query_recipe()
{
    return (["soft pvc":5, "circuit":5,"copper scrap":5]);
}

string query_station()
{
  return "electronics";
}