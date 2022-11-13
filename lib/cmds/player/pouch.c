/*
** pouch.c -- material pouch command
**
*/

//: PLAYERCOMMAND
//$$ see: skills, hp, stats,score
// USAGE pouch
//
// Shows your material pouch and all the materials in it.

#include <config.h>

inherit CMD;

private
void main(string arg)
{
    mapping p_materials = this_body()->query_materials();
    string *mat_cats = CRAFTING_D->query_material_categories();
    int width = this_user()->query_screen_width() - 7;
    int per_row = width / 15;
    int total = 0;
    string divider = repeat_string("-", width) + "\n";
    if (!sizeof(mat_cats))
    {
        write("No materials defined, yet.\n");
        return;
    }

    foreach (int c in values(p_materials))
        total += c;

    printf(divider);
    printf("%%^CYAN%%^%s%%^RESET%%^ - %d materials\n", "MATERIAL POUCH", total);
    printf(divider);
    foreach (string category in mat_cats)
    {
        mapping mats = CRAFTING_D->query_materials(category);
        int line = 0;
        int count = 0;
        string mat_line = "";
        foreach (string mat in mats)
        {
            if (member_array(category + " " + mat, keys(p_materials)) != -1)
                count++;
            mat_line += sprintf("%8s: %-4d ", capitalize(mat), p_materials[category + " " + mat]);
            line++;

            if (line == per_row)
            {
                mat_line += "\n";
                line = 0;
            }
        }
        if (line != 0)
            mat_line += "\n";
        if (count)
        {
            // printf("%%^CYAN%%^%-7s%%^RESET%%^ [%d/%d]\n", capitalize(category), count, sizeof(mats));
            printf("%%^CYAN%%^%s%%^RESET%%^\n", capitalize(category) + " category");
            printf(mat_line + "\n");
        }
    }
    printf(divider);
}
