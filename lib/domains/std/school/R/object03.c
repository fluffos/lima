#include "../school.h";
inherit INDOOR_ROOM;

void setup(){
    set_brief("Optional extras for a simple object");
    set_long(@LORIEL
Adding some weight:
within the setup function, set its size:

    set_size(1);

Adding a value:
first inherit the module 
then (within setup) set the value



    set_value(1);
LORIEL
    );
    set_exits( ([
	"west" : R + "object02",
	"corridor" : R + "corridor05",
      ]) );
    set_objects( ([
      ]) );
}
