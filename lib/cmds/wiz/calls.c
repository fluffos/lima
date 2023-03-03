/* Do not remove the headers from this file! see /USAGE for more info. */

// call_out info display by Rust.
// Improved by Tsath 2020 to show time better, plus better documentation.

//:COMMAND
//USAGE:  calls
//
//This command shows you the number of call_outs that are active.
//
//Produces display like :
//     calls
//
//object                                 Function            Delay
//-----------------------------------------------------------------------------
///secure/master                         <function>          17m 13s
///domains/std/harry#474                 <function>          19h 23m 53s
///domains/std/harry                     <function>          3s
//
//There are 3 call_outs active.

inherit CMD;

#define CHOMP(x) replace_string(x, "/domains/", "^")

private
void main()
{
    mixed *call_out_stuff;
    mixed *data;
    int i;

    call_out_stuff = filter_array(call_out_info(), (
                                                       : sizeof:));

    //We do not have any ANSI here, but also don't want emojis.
    set_output_flags(NO_ANSI);
    
    outf("%-45s%-25s%-10s\n", "Object", "Function", "Delay");
    outf("%82'-'s\n", "");

    foreach (data in call_out_stuff)
    {
        if (functionp(data[1]))
            outf("%-70O%-10s\n", CHOMP(data[1]), time_to_string(data[2], 1));
        else if (data[0])
            outf("%-45s%-25s%-10s\n", CHOMP(file_name(data[0])),
                 data[1], time_to_string(data[2], 1));
        else
            outf("%-45s%-25s%-10s\n", "<destructed>", data[1], time_to_string(data[2], 1));
    }

    outf("\nThere are %d call_outs active.\n", sizeof(call_out_stuff));
}
