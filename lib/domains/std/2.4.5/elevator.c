// Do not remove the headers from this file! see /USAGE for more info.

inherit ELEVATOR;

void setup()
{
    // Call setup() in ELEVATOR
    ::setup();
    // Description of the elevator make sure to mention the buttons you can press.
    // Cmds like 'look at cryo button' also works.
    set_long("You are in the elevator.  There are four buttons, labeled 'church', 'attic', 'wiz hall' and 'lima'.$elevator_door");
    // Set destinations with filenames of where we can go and shorthands.
    set_destinations((["lima":"/domains/std/Wizroom",
                      "attic":"/domains/std/2.4.5/Attic",
                     "church":"/domains/std/2.4.5/Church",
                    "wizhall":"/domains/std/2.4.5/wiz_hall",
    ]));

    // Set start position for elevator
    move_to("church");

    // Set the distance from and to each destination in seconds
    set_distance("church", "wizhall", 26);
    set_distance("attic", "wizhall", 32);
    set_distance("lima", "church", 16);
    set_distance("lima", "attic", 10);
    set_distance("lima", "wizhall", 42);

    // Set an elevator door direction and default location
    set_objects((["/std/elevator_door":({"east", "/domains/std/2.4.5/Attic"})]));

    // Spawn the buttons on the wall automatically.
    // This *must* be done after the set_destinations() cal.
    setup_buttons();
}
