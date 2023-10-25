inherit TAMING_CHIP;

//CRAFTING:simple draft taming chip

void setup()
{
    set_adj("simple");
    set_armour_class(2);
    set_taming_level(5);
    set_draft_chip();
}

mapping query_recipe()
{
    return (["soft pvc":1, "circuit":1,"copper scrap":1]);
}

string query_station()
{
  return "electronics";
}