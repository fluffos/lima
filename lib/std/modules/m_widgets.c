/* Do not remove the headers from this file! see /USAGE for more info. */

/*
** M_WIDGETS (Tsath 2019-2020)
**
** This module returns nicely formatted ANSI type widgets that can be reused
** all over the MUD. The string return are not sprintf() compatible, so you
** need to write() or out() them.
**
*/

//: FUNCTION i_simplify
// Returns TRUE if the current user (not the object receiving the message!)
// has simplify turned on.
nomask int i_simplify()
{
	return get_user_variable("simplify") != 0;
}

//: FUNCTION i_emoji
// Returns TRUE if the current user (not the object receiving the message!)
// has emojis turned on.
nomask int i_emoji()
{
	return get_user_variable("emoji") != 0 && !i_simplify();
}

int default_user_width()
{
	return this_user()->query_screen_width() - 2;
}

//: FUNCTION on_off_widget
// [On ] (green) or [Off] (red) depending on int on.
// "On " or "Off" for simplified view.
string on_off_widget(int on)
{
	if (i_simplify())
		return on ? "On " : "Off ";
	if (on)
		return sprintf("[<002>On<res> ]");
	else
		return sprintf("[<001>Off<res>]>");
}

varargs string fancy_divider(int width)
{
	if (!width)
		width = default_user_width();
	if (i_simplify())
		return "";
	return repeat_string("-=", ((width - 1) / 2)) + "\n";
}

varargs string simple_divider(int width)
{
	if (!width)
		width = default_user_width();
	if (i_simplify())
		return "";
	return repeat_string("-", width) + "\n";
}

//: FUNCTION green_bar
// A bar that is green with white dots when spent
string green_bar(int value, int max, int width)
{
	int green, white;
	if (i_simplify())
		return value + "/" + max;
	if (value > max)
		value = max;
	green = (value * 1.00 / max) * (width)-1;
	white = width - 2 - green;

	return sprintf("[" + (white == 0 ? "<006>" : "<002>") + "%s<res><007>%s<res>]",
				   repeat_string("=", green),
				   repeat_string(".", white));
}

//: FUNCTION critical_bar
// A bar that change colour the lower it gets
string critical_bar(int value, int max, int width)
{
	int green, white;
	float p;
	string *simple_themes = ({"ascii", "lines", "none"});
	int unicodetheme = member_array(this_user()->frames_theme(), simple_themes) == -1;
	string barchar = unicodetheme ? "▅" : "=";
	string nobarchar = unicodetheme ? "▅" : ".";

	string bar_colour = "<040>";
	if (i_simplify())
		return "";
	p = value / (max * 1.0);

	if (p < 0.10)
		bar_colour = "<129>";
	else if (p < 0.20)
		bar_colour = "<196>";
	else if (p < 0.50)
		bar_colour = "<226>";

	if (value > max)
		value = max;
	green = (value * 1.00 / max) * (width)-1;
	if (green < 0)
		green = 0;
	white = width - 1 - green;
	//	TBUG("Green: "+green+" White: "+white+" value: "+value+" max: "+max+" width: "+width);

	return sprintf("[" + bar_colour + "%s<res><"+(unicodetheme ? "238" : "007")+">%s<res>]",
				   repeat_string(barchar, green),
				   repeat_string(nobarchar, white));
}

//: FUNCTION critical_bar
// A bar that change colour the lower it gets
string reverse_critical_bar(int value, int max, int width)
{
	int green, white;
	float p;
	string bar_colour = "<005>";
	if (i_simplify())
		return "";
	if (!max)
		return "";
	p = value / (max * 1.0);

	if (p < 0.30)
		bar_colour = "<002>";
	else if (p < 0.60)
		bar_colour = "<003>";
	else if (p < 0.80)
		bar_colour = "<001>";

	if (value > max)
		value = max;
	green = (value * 1.00 / max) * (width)-1;
	if (green < 0)
		green = 0;
	white = width - 1 - green;

	return sprintf("[" + bar_colour + "%s<res><011>%s<res>]",
				   repeat_string("=", green),
				   repeat_string(".", white));
}

//: FUNCTION slider_red_green
// A slider which is red below the middle and green above, and marks the
// value with a X. 0 is the middle and max is minus on red axis and + on green axis.
string slider_red_green(int value, int max, int width)
{
	string return_string;
	int marker;
	if (i_simplify())
		return "";
	width = width - 3; // [, X and ]
	marker = width * ((value + (max / 2.0)) / (max * 1.0));

	return_string = repeat_string("-", marker) + "X" + repeat_string("-", width - marker);
	return_string = return_string[0..(width / 2)] + "<002>" + return_string[(width / 2)..];
	if (marker < (width / 2))
		return_string = replace_string(return_string, "X", "<bld>X<res><001>");
	else
		return_string = replace_string(return_string, "X", "<bld>X<res><002>");
	return "[<001>" + return_string + "<res>]";
}

//: FUNCTION slider_colours_sum
// A slider with multiple colours and cumulative ranges and a marker.
// The colours mapping should be on the format:
//   ([20:"red",50:"yellow",100:"green"])
// where each number is bigger and strings are ANSI colours.
string slider_colours_sum(int value, mapping colours, int width)
{
	int marker;
	int max = sort_array(keys(colours), -1)[0];
	string return_string;
	int pfish_add = 0;
	int next_pos = 0;
	if (i_simplify())
		return "";
	width = width - 3; // [ and ]
	marker = width * (1.0 * value / max);
	if (marker == 0)
		return_string = "X" + repeat_string("-", width - 1);
	else if (marker == (width - 1))
		return_string = repeat_string("-", width) + "X";
	else
		return_string = repeat_string("-", marker) + "X" + repeat_string("-", width - marker);

	foreach (int val in sort_array(keys(colours), 1))
	{
		string col = colours[val];
		if (!next_pos)
			return_string = upper_case(col) + return_string;
		if (next_pos)
			return_string = return_string[0..(next_pos + pfish_add)] + upper_case(col) + return_string[((next_pos + pfish_add) + 1)..];
		pfish_add += strlen(col) + 4;
		next_pos = width * (1.0 * val / max);
	}

	return "[" + return_string + "<res>]";
}
