/* Do not remove the headers from this file! see /USAGE for more info. */

inherit M_ACCESS;

private mapping inheritable = ([
  "accountant" : ACCOUNTANT,
  "aggressive" : AGGRESSIVE_MONSTER,
  "following monster" : FOLLOWING_MONSTER,
  "monster" : ADVERSARY,
  "living" : LIVING,
  "wandering monster" : WANDERING_MONSTER,

  "armor" : ARMOR,
  "book" : BOOK,
  "container" : CONTAINER,
  "key" : KEY,
  "moving room" : "/std/moving_room",
  "object" : OBJ,
  "outdoor" : OUTDOOR_ROOM,
  "portal" : PORTAL,
  "room" : ROOM,
  "door" : DOOR,
  "hidden door" : HIDDEN_DOOR,
  "secret door" : SECRET_DOOR,
  "torch" : TORCH,
  "weapon" : WEAPON,

  "actor": M_ACTIONS,
  "block" : M_BLOCKEXITS,
  "drinkable" : M_DRINKABLE,
  "gettable" : M_GETTABLE,
  "lockable" : M_LOCKABLE,
  "openable" : M_OPENABLE,
  "readable" : M_READABLE,
  "regex" : M_REGEX,
  "valuable" : M_VALUABLE,
  "vendor" : M_VENDOR,
]);

int cur, intrigger;
int unique;

string handle_oneof(string arg, mixed *block);
string handle_expression(string arg);
mixed handle_subexpression(string arg);
string handle_block(string *lines);
string handle_check(string arg, mixed *block);
string handle_delay(string arg);
string handle_action(string arg);

void add_error(int, string);

private mapping keywords = ([
  "oneof" : (: handle_oneof :),
  "if" : (: "if (" + handle_expression($1) +
    ") {\n" + handle_block($2) + "}\n" :),
  "nexttrigger" : (: intrigger ? "continue;" : (add_error(cur, "nexttrigger illegal outside of trigger"), "") :),
  "call" : (: handle_expression("call " + $1) +";\n" :),
  "lcall" : (: handle_expression("lcall " + $1) +";\n" :),
  "check" : (: handle_check :),
  "ok" :  "return 1;\n",
  "write" : (: "write(" + handle_expression($1) + ");\n" :),
  "lpc" : (: implode($2, "\n") + "\n" :),
  "delay" : (: handle_delay :),
  "action" : (: handle_action :),
  "return" : (: "return " + handle_expression($1) + ";\n" :),
]);

mixed handle_periodic(string arg, string *lines);
mixed handle_trigger(string arg, string *lines);
mixed handle_setup(string arg, string *lines);

private mapping funcs = ([
  "periodic" : (: handle_periodic :),
  "trigger" : (: handle_trigger :),
  "setup" : (: handle_setup :),
]);

string handle_string(string *args);
mixed handle_is(string *args);
mixed handle_variables(string *args);
mixed handle_gender(string *args);
mixed handle_list(string s1, string s2, string *args);
mixed handle_objects(string *args);
mixed handle_exits(string *args);

mapping attributes;
int dest, indent, linesync;
mixed *lines;
mixed *errors;
mixed fname, tail;
mixed setup_args;
mixed *cur_vars;
mixed *globals;

mixed *inherits;
mixed *triggers;

void create() {
    set_privilege(1);
}

void add_error(int line, string err) {
    if (sizeof(errors) < 3)
	errors += ({ fname + ":" + line + "-> " + err });
}

void do_errors() {
    if (sizeof(errors))
	error("Script compilation failed:\n" + implode(errors, "\n") + "\n");
}

string *line_info() {
    if (linesync) {
	linesync = 0;
	return ({ cur });
    }
    return ({});
}

string *parse_long_string() {
    int first = cur;
    string ind;
    int indent;
    string firstline;

    if (cur == sizeof(lines)) return ({});
    firstline = lines[cur++];
    while (firstline[indent] == ' ')
	indent++;
    if (indent == 0)
	add_error(cur, "Bad indentation.");
    ind = repeat_string(" ", indent);

    while (cur < sizeof(lines)) {
	string line = lines[cur++];

	if (line[0..indent-1] != ind) {
	    if (line != "end")
		add_error(cur, "Missing end.");
	    break;
	}
    }
    return map(lines[first..cur-2], (: $1[$(indent)..] :));
}

mixed *parse_block(int oldind, int indent) {
    int needend = -1;
    mixed *ret = ({});

    while (cur < sizeof(lines)) {
	int newind;
	string line = lines[cur++];

	if (line == "" || line[0] == '#') {
	    linesync = 1;
	    continue;
	}

	if (line[0] == ' ') {
	    newind = 1;
	    while (line[newind] == ' ')
		newind++;
	    line = line[newind..];
	} else newind = 0;

	if (indent == -1) {
	    if (newind == 0)
		add_error(cur, "Bad indentation.");
	    indent = newind;
	}

	if (newind < indent) {
	    if (line == "end") {
		if (newind == oldind) {
		    needend = 0;
		    break;
		} else if (needend)
		    add_error(cur, "Bad indentation for 'end'.");
	    }
	    cur--;
	    break;
	} else
	if (newind > indent) {
	    if (arrayp(ret[<1]))
		add_error(cur, "Bad indentation.");
	    cur--;
	    ret[<1] = ({ ret[<1] }) + line_info() + parse_block(indent, newind);
	} else {
	    ret += line_info() + ({ line });
	    needend++;
	}
    }

    if (needend > 0)
	add_error(cur-1, "Missing 'end'.");
    return ret;
}

void assert_alphanum(int line, string what) {
    if (!regexp(what, "^[A-Za-z][A-Za-z0-9_]*$"))
	add_error(line, "Illegal name '" + what + "'.");
}

void handle_grouping() {
    int newind;

    attributes = ([ "is" : ({ LPSCRIPT_SPECIAL, (: handle_is :) }),
      "variables" : ({ LPSCRIPT_SPECIAL, (: handle_variables :) }) ]);
    unique = linesync = cur = dest = indent = 0;
    tail = setup_args = "";
    cur_vars = globals = ({});

    while (cur < sizeof(lines)) {
	mixed part1;
	string part2, part3;
	string line = lines[cur++];

	if (line == "" || line[0] == '#') {
	    linesync = 1;
	    continue;
	}

	if (line[0] == ' ')
	    add_error(cur, "Bad indentation.");
	if (sscanf(line, "%s[%s]=%s", part1, part2, part3) == 3) {
	    assert_alphanum(cur, part1);
	    part1 = ({ "=[", part1, part2 });
	    part2 = part3;
	} else if (sscanf(line, "%s[%s]:%s", part1, part2, part3) == 3) {
	    assert_alphanum(cur, part1);
	    part1 = ({ ":[", part1, part2 });
	    part2 = part3;
	} else if (sscanf(line, "%s=%s", part1, part2) == 2) {
	    assert_alphanum(cur, part1);
	    part1 = ({ "=", part1 });
	} else if (sscanf(line, "%s:%s", part1, part2) == 2) {
	    assert_alphanum(cur, part1);
	    part1 = ({ ":", part1 });
	} else if (line == "---") {
	    tail = implode(lines[cur..], "\n");
	    lines = lines[0..dest-1];
	    return;
	} else part1 = 0;

	if (part1) {
	    if (part2 == "") {
		if (part1[0][0] == "=")		  
		    lines[dest++] = line_info() + part1 + parse_long_string();
		else
		{
		    lines[dest++] = line_info() + part1 + parse_block(0, -1);
		}
	    } else {
		if (part1[0][0] == ":")
		    add_error(cur, "Illegal function declaration");
		lines[dest++] = line_info() + part1 + ({ trim_spaces(part2) });
	    }
	} else
	    add_error(cur, "Syntax error");
    }
    lines = lines[0..dest-1];
}

/*
 * expr1 ::= <number>
 * expr1 ::= <literal>
 * expr1 ::= '(' subexp ')'
 * expr1 ::= '$' var
 */
mixed handle_expr1(string arg) {
    string rest = "";
    mixed tmp;

    switch (arg[0]) {
    case '(':
	tmp = handle_subexpression(arg[1..]);
	if (tmp[1][0] != ')')
	    add_error(cur, "expected ')'");
	return ({ tmp[0], trim_spaces(tmp[1][1..]) });
    case '0'..'9':
	if (sscanf(arg, "%d%s", tmp, rest) == 2)
	    return ({ tmp+"", trim_spaces(rest) });
	break;
    case '$':
	sscanf(arg, "$%([A-Za-z0-9_]*)%s", arg, rest);
	if (member_array(arg, cur_vars) == -1)
	    add_error(cur, "Undefined variable '" + arg + "'");
	rest = trim_spaces(rest);
	return ({ "_" + arg, rest });
    case '"':
	if (sscanf(arg, "\"%(([^\\\"]|\\.)*)\"%s", tmp, rest) != 2)
	    add_error(cur, "Missing '\"'");
	return ({ "\"" + tmp + "\"", trim_spaces(rest) });
    case 'A'..'Z': 
    case 'a'..'z':
	sscanf(arg, "%([A-Za-z]*)%s", arg, rest);
	rest = trim_spaces(rest);

	switch (arg) {
	case "who":
	    return ({ "this_body()", rest });
	case "me":
	    return ({ "this_object()", rest });
	case "here":
	    return ({ "environment(this_body())", rest });
	case "container":
	    return ({ "environment(this_object())", rest });
	case "chance":
	    tmp = handle_expr1(rest);
	    return ({ "random(100) < " + tmp[0], tmp[1] });
	}
    }
    add_error(cur, "'" + arg + "' unrecognized.");
    return ({ "", rest });
}

string handle_call(string s1, string s2, mixed *rest...) {
    return "call_other(" + s1 + ", " + s2 + 
    (sizeof(rest) > 0 ? "," : "") +
    implode(rest, ", ") + ")";
}

string handle_lcall(string s1, mixed *rest...) {
    if (s1[0] == '"' && s1[<1] == '"')
	return s1[1..<2] + "(" + implode(rest, ", ") + ")";
    else
	return handle_call("this_object()", s1, rest...);
}

mixed handle_prefix(string arg) {
    string rest = "";

    sscanf(arg, "%s %s", arg, rest);
    rest = trim_spaces(rest);

    switch (arg) {
    case "find":
	return ({ (: "present(" + $1 + ", " + $2 + ")" :), rest, 2 });
    case "call":
	return ({ (: handle_call :), rest, -3 });
    case "lcall":
	return ({ (: handle_lcall :), rest, -1 });
    case "not":
	return ({ (: "!" + $1 :), rest, 1 });
    case "new":
	return ({ (: "new(" + $1 + ")" :), rest, 1 });
    }

    return 0;
}

mixed handle_infix(string arg) {
    string rest = "";

    sscanf(arg, "%s %s", arg, rest);
    rest = trim_spaces(rest);

    switch (arg) {
    case "notequal":
	return ({ (: $1 + " != " + $2 :), rest });
    case "<":
    case ">":
    case "<=":
    case ">=":
    case "!=":
    case "==":
	return ({ (: $1 + " " + $(arg) + " " + $2 :), rest });
    }

    return 0;
}

mixed handle_assignment(string arg) {
    string var;
    string rest;
    mixed tmp;

    sscanf(arg, "%([A-Za-z0-9_]*)%s", var, rest);
    rest = trim_spaces(rest);
    if (rest[0] != '=')
	add_error(cur, "Expected '=' after '$" + var + "'");
    else
	rest = rest[1..];
    if (var == "")
	add_error(cur, "Missing variable name after '$'");

    tmp = handle_expression(rest);
    if (member_array(var, cur_vars) == -1) {
	cur_vars += ({ var });
	return ({ "_" + var + "=" + tmp, "_" + var });
    } else {
	return ({ "_" + var + "=" + tmp, 0 });
    }
}

/*
 * expr2 ::= expr1
 * expr2 ::= PREFIX expr2
 * expr2 ::= PREFIX2 expr2 expr2
 */
mixed handle_expr2(string arg) {
    int grabextra;
    string rest;
    mixed tmp;
    function f;
    int num;
    mixed *args = ({});

    if (arg == "") {
	add_error(cur, "expression expected.");
	return ({ "0", "" });
    } else if (tmp = handle_prefix(arg)) {
	num = tmp[2];
	rest = tmp[1];
	f = tmp[0];
	if (num < 0) {
	    grabextra = 1;
	    num = -(num + 1);
	}
	while (num-- > 0) {
	    tmp = handle_expr2(rest);
	    args += ({ tmp[0] });
	    rest = tmp[1];
	}
	if (grabextra) {
	    while (rest != "" && rest[0] != ')') {
		tmp = handle_expr2(rest);
		args += ({ tmp[0] });
		rest = tmp[1];
	    }
	}
	tmp[0] = evaluate(f, args...);
	tmp[1] = rest;
	return tmp;
    } else 
	return handle_expr1(arg);
}

/*
 * subexp ::= expr2 INFIX subexp
 */
mixed handle_subexpression(string arg) {
    mixed tmp, tmp2, tmp3;

    if (arg == "") {
	add_error(cur, "expression expected.");
	return ({ "0", "" });
    } else {
	tmp = handle_expr2(arg);
	if (tmp2 = handle_infix(tmp[1])) {
	    tmp3 = handle_subexpression(tmp2[1]);
	    tmp[0] = evaluate(tmp2[0], tmp[0], tmp3[0]);
	    tmp[1] = tmp3[1];
	}
	return tmp;
    }
}

string handle_expression(string arg) {
    mixed tmp;

    tmp = handle_subexpression(trim_spaces(arg));
    if (tmp[1] != "")
	add_error(cur, "junk found after valid expression");

    return tmp[0];
}

string translate_string(string str) {
    string before, var;
    string ret = "\"";

    str = replace_string(str, "\"", "\\\"");
    while (sscanf(str, "%s$%s", before, str) == 2) {
	ret += before;
	sscanf(str, "%([A-Za-z0-9_]*)%s", var, str);
	ret += "\" + _" + var + " + \"";
    }
    ret += str + "\"";
    return ret;
}

string handle_block(mixed *lines) {
    mixed *ret = ({ "", "" });
    int i;
    string tmp;

    foreach (mixed line in lines) {
	if (intp(line)) {
	    cur = line;
	} else
	if (arrayp(line)) {
	    string keyword, args;
	    function f;

	    keyword = line[0];
	    sscanf(keyword, "%s %s", keyword, args);
	    if (f = keywords[keyword] ) {
		ret[<2] += evaluate(f, args, line[1..]);
	    } else {
                debug_message("handle_block(1): keyword "+keyword+" is a "+typeof(f));
		add_error(cur, "Unknown keyword '" + keyword + "'.");
              }
	}
	else if (line[0] == '!') {
	    if (member_array(M_ACTIONS, inherits) != -1)
		ret[<2] += "respond(" + translate_string(line[1..]) + ");\n";
	    else
		ret[<2] += "this_object()->do_game_command(" + translate_string(line[1..]) + ");\n";
	} else if (line[0] == '$') {
	    mixed tmp2;

	    tmp2 = handle_assignment(line[1..]);
	    if (tmp2[1])
		ret[<1] += "mixed " + tmp2[1] + ";\n";
	    ret[<2] += tmp2[0] + ";\n";
	} else {
	    string keyword, args;
	    function f;

	    keyword = line;
	    sscanf(keyword, "%s %s", keyword, args);
	    if (f = keywords[keyword]) {
		ret[<2] += evaluate(f, args, ({}));
	    } else {
               debug_message("handle_block(2): keyword "+keyword+" is a "+typeof(f));
		add_error(cur, "Unknown keyword '" + keyword + "'.");
              }
	    if (keyword == "delay")
		ret += ({ "", "" });
	}
    }
    tmp = "";
    for (i = 0; i < sizeof(ret); i += 2)
	tmp += "{\n" + ret[i+1] + ret[i] + ";\n}\n";

    return tmp;
}

string handle_action(string arg) {
    if (member_array(M_ACTIONS, inherits) != -1)
	return "simple_action(\"" + arg + "\", this_object());";
    else
	return "this_body()->simple_action(\"" + arg + "\", this_object());";
}

string sub_file_name() {
    mixed *tmp = split_path(fname);
    return tmp[0] + "tmp_" + tmp[1][0..<5] + "_" + (unique++) + ".scr";
}

string handle_delay(string arg) {
    int idx = unique++;
    string args, rest;
    mixed *locals = cur_vars - globals;

    if (intrigger) 
	add_error(cur, "Delay illegal inside trigger (move to a function).\n");

    if (sizeof(locals)) {
	args = "mixed _" + implode(locals, ", mixed _");
	rest = ", _" + implode(locals, ", _");
    }
    else args = rest = "";

    return "call_out(\"unnamed" + idx + "\", " + arg + rest + "); }\n}\n\nvoid unnamed" + idx + "(" + args + ") {\n{\n";
}

string handle_check(string arg, mixed *lines) {
    string ret = "";

    foreach (string line in lines) {
	string left = "", right = "";
	string tmpl, tmpr;

	if (sscanf(line, "%s:%s", left, right) != 2)
	    add_error(cur, "Missing ':'");
	tmpl = handle_expression(left);
	tmpr = handle_expression(right);
	ret += "if (" + tmpl + ")\n  return " + tmpr + ";\n";
    }
    ret += "return 1;";
    return ret;
}

string handle_oneof(string arg, mixed *block) {
    string ret;

    ret = "switch (random(" + sizeof(block) + ")) {\n";

    for (int i = 0; i < sizeof(block); i++) {
	ret += "case " + i + ": {\n";
	ret += handle_block( ({ block[i] }) );
	ret += "break; }\n";
    }
    return ret + "}\n";
}

mixed handle_periodic(string arg, mixed *lines) {
    int min, max;
    string time;

    if (sscanf(arg, "%d to %d", min, max) == 2) {
	time = min + " + random(" + (max-min) + ")";
    } else {
	sscanf(arg, "%d", min);
	time = "" + (min || 2);
    }

    cur_vars = globals;
    return ({ "setup", "f = function(function f) { " + handle_block(lines) + " call_out(f, " + time + ", f); }; call_out(f, " + time + ", f)" });
}

mixed handle_setup(string arg, mixed *lines) {
    setup_args = arg;
    cur_vars = globals;
    return ({ "setup", handle_block(lines) });
}

mixed handle_trigger(string arg, mixed *lines) {
    int num;
    int oldlen;

    intrigger = 1;
    oldlen = strlen(arg);
    arg = replace_string(arg, "*", "%s");
    num = strlen(arg) - oldlen;

    cur_vars = globals;
    triggers += ({ ({ arg, num, handle_block(lines) }) });

    intrigger = 0;
    return 0;
}

varargs string regenerate(mixed *lines, int level) {
    string ind = repeat_string("  ", level);
    string ret = "";

    foreach (mixed item in lines) {
	if (intp(item)) continue;
	if (arrayp(item)) {
	    ret += ind + item[0] + "\n" + regenerate(item[1..], level + 1) + ind + "end\n";
	} else
	    ret += ind + item + "\n";
    }
    return ret;
}

// This allows certain attributes which should represent a single object
// (eg wielded = weapon) to optionally accept a random choice :
// wielded =
//   weapon1
//   weapon2
// end
string one_object(mixed *args)
{
  string *output = ({});
  foreach (mixed line in args)
  {
    if (arrayp(line))
    {
      string fn = sub_file_name();
      string id, rest;
      object ob;

      if (sscanf(line[0], "%s=%s", id, rest) != 2 || id == "" || rest != "")
        add_error(cur, "Syntax error in subobject.\n");
      unguarded(1, (: write_file, fn, regenerate(line[1..]), 1 :));
      if (ob = find_object(fn))
        destruct(ob); // we want it to recompile too
      line = fn;
    }
    output += ({ sprintf("  \"%s\" ", line) });
  }
  if(sizeof(output)>1)
    return "choice( ({" + implode(output, ", ") + "}) )";
  else
    return output[0];
}

mixed handle_wield(mixed *args)
{
  return ({ "setup", "set_wielding(" + one_object(args) + ")" });
}

mixed handle_wear(mixed *args)
{
  return ({ "setup", "set_wearing(" + one_object(args) + ")" });
}

mixed handle_objects(mixed *args) {
    string ret = "set_objects( ([\n";
    string tmp;

    foreach (mixed line in args) {
	if (arrayp(line)) {
	    string fn = sub_file_name();
	    string id, rest;
	    object ob;

	    if (sscanf(line[0], "%s=%s", id, rest) != 2 || id == "" || rest != "")
		add_error(cur, "Syntax error in subobject.\n");
	    unguarded(1, (: write_file, fn, regenerate(line[1..]), 1 :));
	    if (ob = find_object(fn))
		destruct(ob); // we want it to recompile too
	    line = fn;
	}
	if (sscanf(line, "%s(%s", line, tmp) == 2) {
	    if (tmp[<1] == ')')
		tmp = tmp[0..<2];
	    ret += sprintf("  \"%s\" : ({ %s }),\n", line, tmp);
	} else {
	    ret += sprintf("  \"%s\" : 1,\n", line);
	}
    }
    return ({ "setup", ret + "]) )" });
}

mixed handle_mapping(string rfunc, string func, mixed *args) {
    string ret = func + "( ([\n";

    foreach (string line in args) {
	string exit, value;

	if (sscanf(line, "%s:%s", exit, value) != 2) {
	    add_error(cur, "Illegal exit value");
	    continue;
	}
	exit = trim_spaces(exit);
	value = trim_spaces(value);
	ret += sprintf("  \"%s\" : \"%s\",\n", exit, value);
    }
    return ({ rfunc, ret + "]) )" });
}

mixed handle_int_mapping(string rfunc, string func, mixed *args) {
    string ret = func + "( ([\n";

    foreach (string line in args) {
	string key, value;

	if (sscanf(line, "%s:%s", key, value) != 2) {
	    add_error(cur, "Illegal int map value");
	    continue;
	}
	key = trim_spaces(key);
	value = trim_spaces(value);
	ret += sprintf("  \"%s\" : %s,\n", key, value);
    }
    return ({ rfunc, ret + "]) )" });
}

string handle_string(mixed *args) {
    string ret = implode(map(args, (: stringp($1) ? $1 : "%-%" :)), "");
    int ret_size;

    ret = replace_string(ret, " %-% ", "\n\n");
    ret = replace_string(ret, "%-%", "");
    ret = replace_string(ret, "\"", "\\\"");
    ret_size = sizeof(ret);

    if (ret_size > 65) {
	string tmp = "";

	for (int i = 0; i <= ret_size; i += 65) {
	    int j;

	    if (i+64 >= ret_size)
		j = ret_size - 1;
	    else j = i+64;

	    tmp += "\"" + ret[i..j] + "\"\n";
	}

	return tmp;
    }

    return "\"" + ret + "\"";
}

mixed handle_flags(mixed *args) {
    string ret = "0";
    args = map(explode(args[0], ","), (: trim_spaces :));
    foreach (string arg in args) {
	switch (arg) {
	case "attached" : ret += " | ATTACHED"; break;
	}
    }
    return ({ "setup", "set_flag(" + ret + ")" });
}

mixed handle_list(string rfunc, string func, mixed *args) {
    args = map(explode(args[0], ","), (: trim_spaces :));

    return ({ rfunc, func + "(\"" + implode(args, "\", \"") + "\")" });
}

void get_attributes(string fname) {
    object ob = load_object(fname);
    mapping tmp;

    if (!ob) return;
    foreach (string prog in deep_inherit_list(ob)) {
	tmp = prog->lpscript_attributes();
	if (tmp) attributes += tmp;
    }
    tmp = ob->lpscript_attributes();
    if (tmp) attributes += tmp;
}

mixed handle_variables(mixed *args) {
    args = map(explode(args[0], ","), (: trim_spaces :));

    globals += args;
    return ({ -1, "mixed _" + implode(args, ";\nmixed _") });
}

mixed handle_is(mixed *args) {
    args = map(explode(args[0], ","), (: trim_spaces :));

    foreach (string file in args) {
	if (file[0] == '"' && file[<1] == '"') {
	    get_attributes(file);
	    inherits += ({ file });
	} else if (inheritable[file]) {
	    get_attributes(inheritable[file]);
	    inherits += ({ inheritable[file] });
	} else
	    add_error(cur, "'" + file + "' unknown.");
    }
    return 0;
}

mixed handle_gender(mixed *args) {
    int gen;

    switch (args[0]) {
    case "male":
	gen = 1;
	break;
    case "female":
	gen = 2;
	break;
    case "neuter":
	gen = 0;
	break;
    }

    return ({ "setup", "set_gender(" + gen + ")" });
}

int handle_boolean(string arg) {
    switch (arg) {
    case "true":
	return 1;
	break;
    case "false":
	return 0;
	break;
    }
}

*handle_attribute(mixed entry, mixed *args) {
    switch (entry[0]) {
    case LPSCRIPT_LIST:
	return handle_list(entry[1], entry[2], args);
    case LPSCRIPT_STRING:
	return ({ entry[1], entry[2] + "(" + handle_string(args) + ")" });
    case LPSCRIPT_INT:
	return ({ entry[1], entry[2] + "(" + args[0] + ")" });
    case LPSCRIPT_OBJECTS:
	return handle_objects(args);
    case LPSCRIPT_WIELD:
  return handle_wield(args);
    case LPSCRIPT_WEAR:
  return handle_wear(args);
    case LPSCRIPT_GENDER:
	return handle_gender(args);
    case LPSCRIPT_BOOLEAN:
	return ({ entry[1], entry[2] + "(" + handle_boolean (args[0]) + ")" });
    case LPSCRIPT_MAPPING:
	return handle_mapping(entry[1], entry[2], args);
    case LPSCRIPT_INT_MAPPING:
	return handle_int_mapping(entry[1], entry[2], args);
    case LPSCRIPT_SPECIAL:
	return evaluate(entry[1], args);
    case LPSCRIPT_FLAGS:
	return handle_flags(args);
    case LPSCRIPT_TWO:
	return evaluate(entry[1], args[0], args[1..]);
    }
    return 0;
}

void handle_parsing() {
    cur = 0;
    inherits = ({});
    triggers = ({});

    for (int i = 0; i < sizeof(lines); i++) {
	int j;
	mixed arr = lines[i];
	mixed entry;

	if (intp(arr[0])) {
	    cur = arr[0];
	    j = 1;
	} else 
	    j = 0;

	switch (arr[j]) {
	case "=":
	    if (entry = attributes[arr[j+1]]) {
		cur++;
		lines[i] = handle_attribute(entry, arr[j+2..]);
	    } else
		add_error(cur++, "Unknown attribute '" + arr[j+1] + "'.");
	    break;
	case "=[":
	    if (entry = attributes[arr[j+1]]) {
		cur++;
		lines[i] = handle_attribute(entry, arr[j+2..]);
	    } else
		add_error(cur++, "Unknown attribute '" + arr[j+1] + "'.");
	    break;
	case ":":
	    cur_vars = globals;
	    lines[i] = ({ arr[j+1], handle_block(arr[j+2..]) });
	    break;
	case ":[":
	    if (entry = funcs[arr[j+1]]) {
		cur++;
		lines[i] = evaluate(entry, arr[j+2], arr[j+3..]);
	    } else
		add_error(cur++, "Unknown function '" + arr[j+1] + "'.");
	    break;
	default:
	    error("Internal error.\n");
	}
    }
}

private nomask void handle_generation(string outname) {
    string header = implode(inherits, (: $1 + "inherit \"" + $2 + "\";\n" :), "") + "\n";
    string ret = "";
    string globals = "";
    string protos = "";
    mixed *tmp;
    string actions = "";

    if (sizeof(triggers)) {
	for (int i = 0; i < sizeof(triggers); i++)
	    actions += "case " + i + ":\n" + triggers[i][2] + "return;\n";

	ret += "*patterns = ({ ";
	for (int i = 0; i < sizeof(triggers); i++) {
	    if (i) ret += ", ";
	    ret += "\"" + triggers[i][0] + "\"";

	}
	ret += "});\n";
	ret += "*num = ({ ";
	for (int i = 0; i < sizeof(triggers); i++) {
	    if (i) ret += ", ";
	    ret += triggers[i][1];
	}
	ret += "});\n";	    
	/*******************************/
	ret += @END
	
void receive_outside_msg(string str) {
string _1, _2, _3, _4, _5, _6;
if (str[<1] == '\n') str = str[0..<2];
for (int i = 0; i < sizeof(patterns); i++) {
if (sscanf(str, patterns[i], _1, _2, _3, _4, _5, _6) == num[i]) {
switch (i) {
END + actions + "}}}}";
	/*******************************/
    }

    tmp = unique_array(lines - ({ 0 }), (: $1[0] :));
    foreach (mixed *item in tmp) {
	if (item[0][0] == -1) {
	    foreach (mixed *block in item)
	    globals += block[1] + ";\n";
	    continue;
	} else if (item[0][0] == "setup") {
	    protos += "void setup(" + setup_args + ");\n";
	    ret += "\nvoid setup(" + setup_args + ") {\nfunction f;\n";
	} else {
	    protos += "mixed " + item[0][0] + "();\n";
	    ret += "\nmixed " + item[0][0] + "() {\n";
	}

	foreach (mixed *block in item)
	ret += block[1] + ";\n";
	ret += "}\n\n";
    }

    unguarded(1, (: write_file, outname,"#pragma no_warnings\n" + header + globals + protos + tail + ret, 1 :));
}

object compile(string scrname) {
    mixed tmp = split_path(scrname);
    mixed path = tmp[0];
    mixed name = tmp[1][0..<5]; /* no .scr */
    mixed outname = path + "tmp_" + name + ".c";
    mixed text;

    fname = scrname;
    text = read_file(scrname);
    if (!text) return 0;
    lines = explode(text, "\n");
    errors = ({});
    handle_grouping();
    do_errors();
    handle_parsing();
    do_errors();
    handle_generation(outname);

    return load_object(outname);
}
