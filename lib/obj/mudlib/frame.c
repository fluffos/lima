/* Do not remove the headers from this file! see /USAGE for more info. */

/*
 * Frame layout module for fancy frames.
 * Tsath 2022/2023
 */

#define RD 0
#define H 1
#define HD 2
#define LD 3
#define D 4
#define DR 5
#define X 6
#define DL 7
#define RU 8
#define HU 9
#define LU 10

mapping themes = (["single":({"┌", "─", "┬", "┐", "│", "├", "┼", "┤", "└", "┴", "┘"}),
                   "double":({"╔", "═", "╦", "╗", "║", "╠", "╬", "╣", "╚", "╩", "╝"})]);
string *bits;

/* Strings */
private
string title, header_content, footer_content, content, hcolours;
/* Ints */
private
int width, header_margin, text_margin;
/* Booleans*/
private
int add_header, add_footer;
;

void create()
{
    width = this_user() ? this_user()->query_screen_width() - 2 : 79;
    header_margin = 2;
    text_margin = 1;
    add_header = 0;
    add_footer = 0;
}

void select_theme(string theme)
{
    bits = themes[theme];
}

string *query_themes()
{
    return keys(themes);
}

void set_title(string s)
{
    title = s;
}

void set_width(int w)
{
    width = w;
}

void set_header_margin(int hm)
{
    header_margin = hm;
}

void set_text_margin(int tm)
{
    text_margin = tm;
}

void set_header_content(string hc)
{
    header_content = hc;
    add_header = 1;
}

void set_footer_content(string fc)
{
    footer_content = fc;
    add_footer = 1;
}

void set_content(string c)
{
    content = c;
}

void set_hcolours(string hc)
{
    hcolours = hc;
}

private
string create_header()
{
    string out = "";
    int i = 0;
    int header_width = strlen(title) + (text_margin * 2);
    string *headers = explode(header_content || "", "\n");

    if (!title)
        return "";

    if (add_header)
        header_margin++;

    out += repeat_string(" ", header_margin) + bits[RD];
    while (i < header_width)
    {
        out += bits[H];
        i++;
    }

    out += bits[LD] + "\n";
    out += bits[RD] + (add_header ? bits[HD] : "") + repeat_string(bits[H], header_margin - (add_header ? 2 : 1)) + bits[DL] + sprintf("%|" + header_width + "s", title) + bits[DR] +
           repeat_string(bits[H], width - header_width - header_margin - (add_header ? 4 : 3)) + (add_header ? bits[HD] : "") + bits[LD] + "\n";

    out += bits[D] + (add_header ? bits[D] : "") + repeat_string(" ", header_margin - (add_header ? 2 : 1)) + bits[RU];

    i = 0;
    while (i < header_width)
    {
        out += bits[H];
        i++;
    }
    out += bits[LU] + repeat_string(" ", width - header_width - header_margin - (add_header ? 4 : 3)) +
           (add_header ? bits[D] : "") + bits[D] + "\n"; // End of Title box section

    foreach (string h in headers)
    {
        if (header_margin > 0)
            out += bits[D] + (add_header ? bits[D] : "") + " " +
                   sprintf("%-" + (width - 6) + "." + (width - 6) + "s", h) + " " +
                   (add_header ? bits[D] : "") + bits[D] + "\n";
    }

    if (add_header)
        out += bits[D] + (add_header ? bits[RU] : "") +
               repeat_string(bits[H], width - 4) +
               (add_header ? bits[LU] : "") + bits[D] + "\n";

    return out;
}

private
string create_footer()
{
    string out = "";
    int i = 0;
    string *footers = explode(footer_content || "", "\n");

    out += bits[D] + bits[RD] +
           repeat_string(bits[H], (width - 4)) +
           bits[LD] + bits[D] + "\n";

    foreach (string f in footers)
    {
        out += bits[D] + bits[D] + " " +
               sprintf("%-" + (width - 6) + "." + (width - 6) + "s", f) + " " +
               bits[D] + bits[D] + "\n";
    }

    out += bits[RU] + bits[HU] + repeat_string(bits[H], width - 4) +
           bits[HU] + bits[LU] + "\n"; // End of Title box section

    return out;
}

private
string create_content()
{
    string out = "";
    string *contents = explode(content || "", "\n");

    foreach (string c in contents)
    {
        out += bits[D] + " " +
               sprintf("%-" + (width - 4) + "." + (width - 4) + "s", c) + " " +
               bits[D] + "\n";
    }
    return out;
}

private
string end_frame()
{
    string out = "";
    out += bits[RU] + repeat_string(bits[H], width - 2) +
           bits[LU] + "\n"; // End of Title box section
    return out;
}

private
string use_colour(string *cols, int length)
{
    int col_index = floor(sizeof(cols) * ((0.0 + length) / width));

    col_index = CLAMP(col_index, 0, sizeof(cols) - 1);
    return "<" + cols[col_index] + ">";
}

private
string h_colours(string output)
{
    string *colours = explode(hcolours, ",");
    string frames = implode(bits, "");
    mixed *pieces = pcre_assoc(output, ({"[" + frames + "]"}), ({1}));
    string *bits = pieces[0];
    string *matches = pieces[1];
    string new_out = "";
    int i = 0;
    int position = 0;
    while (i < sizeof(bits))
    {
        if (strsrch(bits[i], "\n") != -1)
            position = 0;

        new_out += (matches[i] ? use_colour(colours, position) + bits[i] + "<res>" : bits[i]);
        position += strlen(bits[i]);
        i++;
    }

    return new_out;
}

string render()
{
    string out = "";

    if (!bits)
        error("Need to set frame theme before render() using frame->set_theme().\n" +
              "Current themes: " + format_list(query_themes()) + ".");

    if (title)
        out = create_header();

    if (content)
        out += create_content();

    if (footer_content)
        out += create_footer();
    else
        out += end_frame();

    if (hcolours)
        out = h_colours(out);
    return out;
}