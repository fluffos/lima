/* Do not remove the headers from this file! see /USAGE for more info. */

// RST_D by Tsath 2022-10-21
// Some code from DOC_D, and uses same autodoc formats.

#include <security.h>
#include <log.h>

inherit M_REGEX;
inherit M_DAEMON_DATA;

#define RST_DIR "/help/rst"

//: MODULE
// The RST (reStructured Text) daemon handles finding source files which have been modified and
// updating the RST documentation.

//: TODO
// Finish the daemon.
// And check that it works.

// Public functions --------------------------------------------------------

//: FUNCTION compose_message
// The lowest level message composing function; it is passed the object
// for whom the message is wanted, the message string, the array of people
// involved, and the objects involved.  It returns the appropriate message.
// Usually this routine is used through the higher level interfaces.
varargs string compose_message(object forwhom,
                               string msg,
                               object *who,
                               mixed *obs...)
{
  // Do nothing, just documentation test.
}

private
void continue_scan();
private
int last_time;
private
mixed *files_to_do;
private
mixed *dirs_to_do;

private
void delete_directory(string directory)
{
  if (file_size(directory) == -1)
    return;
  foreach (mixed *file in get_dir(sprintf("%s/", directory), -1))
  {
    string target = sprintf("%s/%s", directory, file[0]);
    if (file[1] == -2)
      delete_directory(target);
    else
      rm(target);
  }
  rmdir(directory);
}

private
void make_directories()
{
  /* Assume that if the filesize is -1 that a directory needs to be created */
  if (file_size(RST_DIR) == -1)
    mkdir(RST_DIR);
  if (file_size(RST_DIR + "/api") == -1)
    mkdir(RST_DIR + "/api");
  if (file_size(RST_DIR + "/daemon") == -1)
    mkdir(RST_DIR + "/daemon");
  if (file_size(RST_DIR + "/command") == -1)
    mkdir(RST_DIR + "/command");
  if (file_size(RST_DIR + "/player_command") == -1)
    mkdir(RST_DIR + "/player_command");
}

//: FUNCTION scan_mudlib
//
// Recursively searches the mudlib for files which have been changed
// since the last time the docs were updated, and recreates the documentation
// for those files.
void scan_mudlib()
{
  printf("Starting RST scan ...\n");
  files_to_do = ({});
  dirs_to_do = ({"/daemons/", "/cmds/"});
  if (!last_time)
  {
    make_directories();
  }
  continue_scan();
}

//: FUNCTION complete_rebuild
//
// Rebuild all the data, regardless of modification time
void complete_rebuild()
{
  last_time = 0;
  scan_mudlib();
}

// Everything below here is
private:
// ---------------------------------------------------------------------

nosave private string *filtered_dirs = ({"/data/", "/ftp/", "/help/", "/include/",
                                         "/log/", "/open/", "/tmp/", "/user/", "/wiz/",
                                         "/contrib/"});

string *mod_name(string file)
{
  string subdir = 0;
  if (strlen(file) > 9 && file[0..8] == "/daemons/")
    subdir = "daemon";
  else if (strsrch(file, "cmds/player") != -1) // Extend this if you have other player dirs.
    subdir = "player command";
  else if (strsrch(file, "cmds") != -1)
    subdir = "command";
  else
    subdir = "api";

  sscanf(file, "%s.c", file);
  return ({file[strsrch(file, "/", -1) + 1..], subdir});
}

string func_name(string bar)
{
  sscanf(bar, "%s(", bar);
  return bar;
}

string command_link(string cmd, string type)
{
  if (type == "player command")
    return "`" + cmd + " <player_command/" + cmd + ">`_";
  if (type == "command")
    return "`Command: " + cmd + " <command/" + cmd + ">`_";
  if (type == "daemon")
    return "`Daemon: " + cmd + " <daemon/" + cmd + ">`_";
}

void process_file(string fname)
{
  string file = read_file(fname);
  string line, prototype;
  string *lines, match;
  string *fixme = ({});
  string *todo = ({});
  string *tmpstr;
  string *cmd_info = ({});
  string *module_info = ({});
  string *c_functions = ({});
  string rstfile, rstout, description = "";
  int i, len;

  TBUG("Processing " + fname);

  /* If the file has not been modified since the last time that DOC_D
   * scanned, there is no reason for it to be checked again -- Tigran */
  if (last_time &&
      get_dir(fname, -1)[0][2] < last_time)
    return;

  if (!file)
    return;
  lines = explode(file, "\n");
  tmpstr = mod_name(fname);

  rstfile = RST_DIR + "/" + replace_string(tmpstr[1], " ", "_") + "/" + tmpstr[0] + ".rst";
  TBUG("Out file: " + rstfile);

  switch (tmpstr[1])
  {
  case "daemon":
    tmpstr += ({"Daemon " + tmpstr[0] + "\n"});
    break;
  case "player command":
    tmpstr += ({"Player Command *" + tmpstr[0] + "*\n"});
    break;
  case "command":
    tmpstr += ({"Command *" + tmpstr[0] + "*\n"});
    break;
  default:
    tmpstr += ({tmpstr[0] + "\n"});
    break;
  }

  rstout = repeat_string("*", strlen(tmpstr[2])) + "\n";
  rstout += tmpstr[2];
  rstout += repeat_string("*", strlen(tmpstr[2])) + "\n\n";
  rstout += "Documentation for the " + tmpstr[0] + " " + tmpstr[1] +
            (tmpstr[1] == "player command" ? ".\n\n" : " in *" + fname + "*.\n\n");

  len = sizeof(lines);
  while (i < len)
  {
    if (regexp(lines[i], "^[ \t]*//###"))
    {
      while (sscanf(lines[i], "%*(^[ \t]*//###)%s", line))
      {
        fixme += ({trim(line) + " (line " + (i + 1) + ")"});
        i++;
      }
    }
    else if (lines[i][0..2] == "//:")
    {
      line = lines[i][3..];
      line = trim(line);
      TBUG("Function is: '" + line + "'");
      i++;
      if (line == "TODO")
      {
        string t = "";
        while (lines[i][0..1] == "//")
        {
          t += lines[i][2..];
          i++;
        }
        todo += ({t});
      }
      else if (line == "MODULE")
      {
        while (lines[i][0..1] == "//")
        {
          module_info += ({lines[i][2..] + "\n"});
          i++;
        }
      }
      else if (line == "COMMAND" || line == "PLAYERCOMMAND" || line == "ADMINCOMMAND")
      {
        while (lines[i][0..1] == "//")
        {
          cmd_info += ({lines[i][2..] + "\n"});
          i++;
        }
      }
      else if (sscanf(line, "HOOK %s", line) == 1)
      {
        /*
        outfile = "/help/autodoc/hook/" + line;
        write_file(outfile, "Hook "+line+":\nCalled by module "
            +mod_name(fname)+" (file: "+fname+")\n\n");
            */
        while (lines[i][0..1] == "//")
        {
          /* write_file(outfile, lines[i][2..]+"\n"); */
          i++;
        }
      }
      else if (sscanf(line, "FUNCTION %s", line) == 1)
      {
        while ((i < sizeof(lines)) && (lines[i][0..1] == "//"))
        {
          description += "\n" + lines[i][2..];
          i++;
        }
        //### regexp() doesn't match any ";", had to replace_string() them
        match = regexp(map(lines[i..i + 19], (
                                                 : replace_string($1, ";", "#")
                                                 :)),
                       "\\<" + line + "\\>", 1);
        if (sizeof(match) > 0)
        {
          if (sscanf(implode(lines[i + match[1] - 1..i + match[1] + 4], "\n"),
                     "%([ \t]*([a-zA-Z_][a-zA-Z0-9_* \t\n]*|)\\<" + line + "\\>[ \t\n]*\\([ \t\na-zA-Z_0-9*,.]*(\\)|))",
                     prototype))
          {
            c_functions += ({"\n\n.. c:function:: " + prototype + "\n" + description + "\n\n"});
          }
        }
        description = "";
      }
      else if (line == AUTODOC_MUDNAME)
      {
        /* outfile = "/help/autodoc/"+MUD_AUTODOC_DIR+"/" + mod_name(fname);
        write_file(outfile,"**** "+fname+" ****\n\n"); */
        while (lines[i][0..1] == "//")
        {
          /* write_file(outfile, lines[i][2..]+"\n"); */
          i++;
        }
      }
      else
      {
        LOG_D->log(LOG_AUTODOC, "Bad header tag: " + fname + " line " + i + ": " + line + "\n");
      }
      /* printf("Writing to: %O\n", outfile); */
    }
    else
      i++;
  }

  if (sizeof(module_info) > 0)
  {
    string moduleh = "Module Information";

    rstout += moduleh + "\n" + repeat_string("=", strlen(moduleh)) + "\n\n";
    foreach (string mi in module_info)
    {
      rstout += mi;
    }
    rstout += "\n";
  }

  if (sizeof(cmd_info) > 0)
  {
    string cfunch = tmpstr[1] == "player command" ? "Player Command" : "Command";
    int todoi = 1;
    int usage = 0;

    rstout += cfunch + "\n" + repeat_string("=", strlen(cfunch)) + "\n\n";
    foreach (string cinf in cmd_info)
    {
      if (strlen(cinf) > 2 && cinf[0..6] == "$$ see:")
      {
        string *sees = explode(cinf[7..], ",");
        rstout += "See: ";
        foreach (string s in sees)
        {
          rstout += command_link(trim(s), tmpstr[1]) + " ";
        }
        rstout += "\n\n";
      }
      else
      {
        if (strsrch(cinf, "USAGE") != -1)
        {
          // Ugly, but seems to cover most situations.
          usage = 1;
          cinf = replace_string(trim(cinf), "USAGE:", "USAGE");
          cinf = trim(replace_string(trim(cinf), "USAGE ", "USAGE::\n\n\t"));
          rstout += trim(cinf) + "\n";
          continue;
        }
        if (trim(cinf) == "")
          usage = 0;

        if (usage)
          rstout += "\t" + trim(cinf) + "\n";
        else
          rstout += cinf;
      }
    }
    rstout += "\n\n";
  }

  if (sizeof(c_functions) > 0)
  {
    string cfunch = "Functions";
    int todoi = 1;

    rstout += cfunch + "\n" + repeat_string("=", strlen(cfunch)) + "\n\n";
    foreach (string cfunc in c_functions)
    {
      rstout += cfunc;
    }
  }

  if (sizeof(todo) > 0)
  {
    string todoh = "TODO list";
    int todoi = 1;

    rstout += todoh + "\n" + repeat_string("=", strlen(todoh)) + "\n\n";

    foreach (string t in todo)
    {
      rstout += todoi + ". " + implode(todo, " ") + "\n";
      todoi++;
    }
    rstout += "\n";
  }

  if (sizeof(fixme) > 0)
  {
    string fixmeh = "List of Fix Me's";

    rstout += fixmeh + "\n" + repeat_string("-", strlen(fixmeh)) + "\n\n";

    foreach (string fixstr in fixme)
    {
      rstout += ":Fix me: " + fixstr + "\n";
    }
  }

  rm(rstfile);
  write_file(rstfile, rstout);
}

void make_index(string header, string *files, string filename)
{
  string output;
  output = repeat_string("*", strlen(header)) + "\n";
  output += header + "\n";
  output += repeat_string("*", strlen(header)) + "\n\n";

  foreach (string file in files)
  {
    string *chunks = explode(file, "/");
    string cmd;
    cmd = sizeof(chunks) > 0 ? chunks[ < 1][0.. < 5] : "";
    output += "- " + command_link(cmd, "command") + "\n";
  }

  rm(filename);
  write_file(filename, output);
}

void write_indices()
{
  /* Player commands index */
  string *files = get_dir(RST_DIR + "/player_command/*.rst");
  make_index("Player Commands", files, RST_DIR + "/Player_Commands.rst");

  /* Commands index */
  files = get_dir(RST_DIR + "/command/*.rst");
  make_index("Commands", files, RST_DIR + "/Commands.rst");

  /* Daemons index */
  files = get_dir(RST_DIR + "/daemon/*.rst");
  make_index("Daemons", files, RST_DIR + "/Daemons.rst");
}

void continue_scan()
{
  mixed *files;
  mixed *item;

  for (int i = 0; i < 10; i++)
  {
    if (sizeof(dirs_to_do))
    {
      printf("Scanning %s ...\n", dirs_to_do[0]);
      files = get_dir(dirs_to_do[0], -1);
      foreach (item in files)
      {
        if (item[1] == -2)
        {
          string dir = dirs_to_do[0] + item[0] + "/";
          if (member_array(dir, filtered_dirs) != -1)
            continue;
          dirs_to_do += ({dir});
        }
        else if (item[2] > last_time && item[0][ < 2.. < 1] == ".c")
        {
          files_to_do += ({dirs_to_do[0] + item[0]});
        }
      }
      dirs_to_do[0..0] = ({});
    }
    else if (sizeof(files_to_do))
    {
      printf("Updating RST docs for %s ...\n", files_to_do[0]);
      /*
       ** We need an unguarded() for any writes that may occur... there
       ** is no user object, so protection checks will always fail.  This
       ** will terminate the checking at this daemon rather than fall
       ** off the stack and fail.  Note that we don't actually hit priv
       ** 1, but the maximum allowed.
       */
      unguarded(1, (
                       : process_file, files_to_do[0]
                       :));
      files_to_do[0..0] = ({});
    }
    else
    {
      printf("Done with sub pages.\nWriting indices.\n");
      write_indices();
      printf("Done.\n");
      last_time = time();
      save_me();
      return;
    }
  }
  call_out((
               : continue_scan:),
           1);
}

void do_sweep()
{
  scan_mudlib();
  call_out((
               : do_sweep:),
           86000);
}

void create()
{
  if (clonep())
  {
    destruct(this_object());
    return;
  }
  ::create();
  if (!last_time)
    do_sweep();
  else
    call_out((
                 : do_sweep:),
             86000);
}