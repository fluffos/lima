/*
** M_HEALING
**
** Module that handles healing limbs of living things.
**
** Tsath 2019-11-16
*/


private int heal_value=0;

//:FUNCTION set_heal_value
//Sets the number of HP returned by one hit from this
//thing.
void set_heal_value(int val)
{
    heal_value = val;
}

//:FUNCTION query_heal_value
//Query the number of HP returned by one hit from this
//thing.
int query_heal_value()
{
    return heal_value;
}

void heal_limb(string limb)
{
    if (this_body())
        this_body()->heal_us(heal_value,limb);
}

void heal_from_food()
{
    if (this_body())
        this_body()->heal_us(heal_value);
}

void heal_from_slowheal()
{
    if (this_body())
        this_body()->heal_us(heal_value);
}

void heal_from_drink()
{
    if (this_body())
        this_body()->heal_us(heal_value);
}

void heal_from_bandage(string limb)
{
    if (this_body())
        this_body()->heal_us(heal_value,limb);
}