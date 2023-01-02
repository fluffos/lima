/* Do not remove the headers from this file! see /USAGE for more info. */

/*
 * Frame layout module for fancy frames.
 * Tsath 2022/2023. Heavily inspired by frame layouts done by Diavolo@Merentha.
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

#define FALLBACK_THEME "ascii"
#define COL_GRADIENT 0
#define COL_TITLE 1
#define COL_ACCENT 2
#define COL_WARNING 3

inherit M_COLOURS;

mapping themes = (["single":({"┌", "─", "┬", "┐", "│", "├", "┼", "┤", "└", "┴", "┘"}),
                   "double":({"╔", "═", "╦", "╗", "║", "╠", "╬", "╣", "╚", "╩", "╝"}),
                    "ascii":({"+", "-", "-", "+", "|", "|", "|", "|", "+", "-", "+"}),
                    "lines":({"-", "-", "-", "-", " ", " ", "-", " ", "-", "-", "-"}),
                     "none":({" ", " ", " ", " ", " ", " ", " ", " ", " ", " ", " "})]);

mapping colours = (["fire":({"160,166,172,178,184,190,226,220,214,208,202,196", "", "", ""}),
                   "steel":({"239,240,241,242,255,237,238,239,240,241", "", "", ""}),
                   "fairy":({"093,099,105,111,117,123,159,153,147,141,135,129", "", "", ""}),
                    "cool":({"021,027,033,039,045,051,087,081,075,069,063,057", "", "", ""}),
                    "pink":({"165,171,177,183,189,195,231,225,219,213,207,201", "", "", ""}),
                   "blues":({"058,059,060,061,062,063", "", "", ""}),
                    "dusk":({"130,131,132,133,134,135", "", "", ""}),
                   "sunny":({"226,227,228,229,230,231", "", "", ""}),
                    "neon":({"088,089,090,091,092,093", "228", "231", "197"}),
                  "nature":({"022,028,034,040,046,083,077,071,065,059", "", "", ""}),
                    "none":({""}),
]);
string *bits;

/* Strings */
private
string title, header_content, footer_content, content, theme;
/* Ints */
private
int width, header_margin, text_margin;
/* Booleans*/
private
int add_header, add_footer;
private
string *hcolours;

void select_theme(string t)
{
    theme = t;

    if (member_array(theme, keys(themes)) != -1)
        bits = themes[theme];
    else
        bits = themes[FALLBACK_THEME];
}

void create()
{
    object shell = this_user()->query_shell_ob();
    width = (this_user()->query_screen_width() ? this_user()->query_screen_width() - 2 : 79);
    hcolours = (this_user()->frames_colour() != "none" ? colours[this_user()->frames_colour()] : 0);
    select_theme(this_user()->frames_theme());
    header_margin = 2;
    text_margin = 1;
    add_header = 0;
    add_footer = 0;
}

string *query_colour_themes()
{
    return sort_array(keys(colours), 1);
}

string *query_themes()
{
    return keys(themes);
}

void set_title(string s)
{
    if (XTERM256_D->colourp(s))
        title = s;
    else
        title = "<" + hcolours[COL_TITLE] + ">" + s + "<res>";
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
    if (XTERM256_D->colourp(hc))
        header_content = hc;
    else
        header_content = "<" + hcolours[COL_ACCENT] + ">" + hc + "<res>";
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

void set_hcolours(string *hc)
{
    hcolours = hc;
}

private
string simple_header()
{
    string out = "";
    out += bits[RD] + repeat_string(bits[H], width - 2) + bits[LD] + "\n";
    return out;
}

private
string create_header()
{
    string out = "";
    int i = 0;
    int simple_header = theme == "lines" || theme == "none";
    int header_width = colour_strlen(title) + (text_margin * 2);
    string *headers = explode(header_content || "", "\n");

    if (!title)
        return simple_header();

    if (add_header)
        header_margin++;

    if (!simple_header)
    {

        out += repeat_string(" ", header_margin) + bits[RD];
        while (i < header_width)
        {
            out += bits[H];
            i++;
        }

        out += bits[LD] + "\n";
    }

    out += bits[RD] + (add_header ? bits[HD] : "") + repeat_string(bits[H], header_margin - (add_header ? 2 : 1)) + bits[DL] +
           repeat_string(" ", text_margin) + title + repeat_string(" ", text_margin) + bits[DR] +
           repeat_string(bits[H], width - header_width - header_margin - (add_header ? 4 : 3)) + (add_header ? bits[HD] : "") + bits[LD] + "\n";

    if (!simple_header)
    {
        out += bits[D] + (add_header ? bits[D] : "") + repeat_string(" ", header_margin - (add_header ? 2 : 1)) + bits[RU];

        i = 0;
        while (i < header_width)
        {
            out += bits[H];
            i++;
        }
        out += bits[LU] + repeat_string(" ", width - header_width - header_margin - (add_header ? 4 : 3)) +
               (add_header ? bits[D] : "") + bits[D] + "\n"; // End of Title box section
    }

    foreach (string h in headers)
    {
        int col_lendiff = strlen(h) - colour_strlen(h);
        int content_width = width - 6 + col_lendiff;
        if (header_margin > 0)
            out += bits[D] + (add_header && !simple_header ? bits[D] : "") + " " +
                   sprintf("%-" + content_width + "." + content_width + "s", h) + " " +
                   (add_header ? bits[D] : "") + bits[D] + "\n";
    }

    if (simple_header)
        out += "";
    else if (add_header)
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
        int col_lendiff = strlen(f) - colour_strlen(f);
        int content_width = width - 6 + col_lendiff;
        out += bits[D] + bits[D] + " " +
               sprintf("%-" + content_width + "." + content_width + "s", f) + " " +
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
        int col_lendiff = strlen(c) - colour_strlen(c);
        int content_width = width - 4 + col_lendiff;
        out += bits[D] + " " +
               sprintf("%-" + content_width + "." + content_width + "s", c) + " " +
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

varargs private string h_colours(string output, string colstring)
{
    string frames = implode(bits, "");
    mixed *pieces = pcre_assoc(output, ({"[" + frames + "]"}), ({1}));
    string *bits = pieces[0];
    string *matches = pieces[1];
    string new_out = "";
    string *colours = explode(colstring || hcolours[COL_GRADIENT], ",");
    int i = 0;
    int position = 0;
    if (!sizeof(colours))
        return output;
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

string demo_string(string theme)
{
    int w = this_user()->query_screen_width();
    return themes[theme][RD] + themes[theme][H] + themes[theme][H] + themes[theme][H] +
           repeat_string(themes[theme][H], (w / 2) - 20) +
           themes[theme][HD] + themes[theme][H] + themes[theme][H] + themes[theme][H] +
           repeat_string(themes[theme][H], (w / 2) - 20) +
           themes[theme][H] + themes[theme][H] + themes[theme][H] + themes[theme][LD];
}

string colour_demo(string theme, string colour)
{
    if (member_array(theme, keys(themes)) == -1)
        theme = "single";
    return h_colours(demo_string(theme), colours[colour]);
}

string render()
{
    string out = "";

    if (!bits)
        error("Need to set frame theme before render() using frame->set_theme().\n" +
              "Current themes: " + format_list(query_themes()) + ".");

    out = create_header();

    if (content)
        out += create_content();

    if (footer_content)
        out += create_footer();
    else
        out += end_frame();

    if (hcolours && this_user()->terminal_mode() != "plain")
        out = h_colours(out);
    return out;
}