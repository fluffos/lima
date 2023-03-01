/* Do not remove the headers from this file! see /USAGE for more info. */

inherit CMD;

private
void main(string str)
{

    object *smarts = filter(objects(), (
                                           : $1->is_smart() && clonep($1)
                                           :));

    printf("<bld>%-25.25s %-20.20s %s<res>", "Name", "Env", "Mood");
    foreach (object s in smarts)
    {
        string name = s->short();
        string env = environment(s) ? environment(s)->short() : "Nowhere";
        string mood = s->emotion_string();
        printf("%-25.25s %-20.20s %s", name, env, mood);
    }
}
