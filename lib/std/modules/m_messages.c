/* Do not remove the headers from this file! see /USAGE for more info. */

#include <mudlib.h>

/* General message handling.  Inherit it in anything that needs it.
 *
 * -Beek
 */

//:MODULE
// The message module.  The correct way to compose and send any messages
// To users is using this module, as it will automatically get the grammar
// right for each person involved.

/* More simul conversion fall out */
string punctuate(string);
nosave private string vowels = "aeiouAEIOU";

#define A_SHORT(x) (objectp(x) ? x->a_short() : (member_array(x[0], vowels) == -1 ? "a " : "an ") + x)
#define SHORT(x) (objectp(x) ? x->short() : x)
mapping messages = ([]);
string def_message_type;

string _a_short(mixed x)
{
  if(objectp(x))
    return x->a_short();
// Check for "a"/"an" prefix - if missing, add it
  if(stringp(x))
    if(strsrch(lower_case(x), "a ") != 0)
      if(strsrch(lower_case(x), "an ") != 0)
      {
        if(member_array(x[0],vowels) == -1)
          return "a " + x;
        else
          return "an " + x;
      }
  return x;
}

string _the_short(mixed x)
{
  if(objectp(x))
    return x->the_short();

// Check for "the" prefix - if missing, add it
  if(stringp(x))
    if(strsrch(lower_case(x), "the ") != 0)
      return "the " + x;

  return x;
}

void set_def_msgs(string type){ def_message_type=type; }

void add_msg(string cls, string msg)
{
  if (!messages)
    messages = ([]);
  if (arrayp(messages[cls]))
    messages[cls] += ({ msg });
  else if (messages[cls])
    messages[cls]=({ messages[cls], msg });
  else
    messages[cls]=msg;
}

string query_msg(string which)
{
  return messages[which] || MESSAGES_D->get_messages(def_message_type)[which];
}

void set_msgs(string cls, string *msgs)
{
  if (!messages)
    messages = ([]);
  if(!msgs || !sizeof(msgs))
  {
    map_delete(messages, cls);
    return;
  }
  messages[cls] = msgs;
}

void clear_messages(){ messages = ([]); }

string *query_msg_types()
{
  return clean_array(keys(messages) +
      keys(MESSAGES_D->get_messages(def_message_type)));
}

string *handle_obs(mixed *obs, string res, mapping has)
{
  string *ret = ({});
  mapping items = ([]);
  string t_short;

  if(objectp(obs[0]) && has[obs])
    ret = ({"them"});
  else
  {
    foreach(mixed ob in obs)
    {
      t_short = SHORT(ob);
      if(member_array(t_short, keys(items)) < 0)
        items += ([ t_short : 1 ]);
      else
        items[t_short] ++;
    }
    obs = keys(items);

    if (res[<2..<1]=="a ")
    {
      res = res[0..<3];
      foreach(mixed ob in obs)
      {
        if(items[ob]>1)
          ret += ({ items[ob] + " " + M_GRAMMAR->pluralize(SHORT(ob)) });
        else
          ret +=  ({ _a_short(ob) });
      }
    } else if (res[<4..<1] == "the ") {
      res = res[0..<5];
      foreach(mixed ob in obs)
      {
        if(items[ob]>1)
          ret += ({ "the " + items[ob] + " " + M_GRAMMAR->pluralize(SHORT(ob)) });
        else
          ret += ({ _the_short(ob) });
      }
    } else if (res[<2..<1] == "A ") {
      res = res[0..<3];
      foreach(mixed ob in obs)
      {
        if(items[ob]>1)
          ret += ({ items[ob] + " " + M_GRAMMAR->pluralize(SHORT(ob)) });
        else
          ret += ({ capitalize(_a_short(ob)) });
      }
    } else if (res[<4..<1] == "The ") {
      res = res[0..<5];
      foreach(mixed ob in obs)
      {
        if(items[ob]>1)
          ret += ({ "the " + items[ob] + " " + M_GRAMMAR->pluralize(SHORT(ob)) });
        else
          ret += ({ capitalize(_the_short(ob)) });
      }
    } else
      foreach(mixed ob in obs)
      {
        if(items[ob]>1)
          ret += ({ items[ob] + " " + M_GRAMMAR->pluralize(SHORT(ob)) });
        else
          ret += ({ SHORT(ob) });
      }
    has[obs]++;
  }
  return ({ res, ret });
}

string *handle_ob(mixed ob, string res, mapping has)
{
  string bit;

  if (objectp(ob) && has[ob])
    bit = "it";
  else
  {
    if (res[<2..<1]=="a ")
    {
      res = res[0..<3];
      bit = _a_short(ob);
    } else if (res[<4..<1] == "the ") {
      res = res[0..<5];
      bit = _the_short(ob);
    } else if (res[<2..<1] == "A ") {
      res = res[0..<3];
      bit = capitalize(_a_short(ob));
    } else if (res[<4..<1] == "The ") {
      res = res[0..<5];
      bit = capitalize(_the_short(ob));
    } else
      bit = SHORT(ob);
    has[ob]++;
    has[bit]++;
    }
  return ({ res, bit });
}

//:FUNCTION compose_message
//The lowest level message composing function; it is passed the object
//for whom the message is wanted, the message string, the *of people
//involved, and the objects involved.  It returns the appropriate message.
//Usually this routine is used through the higher level interfaces.
varargs string compose_message(object forwhom, string msg, object *who, 
  mixed *obs...)
{
  mixed ob;
  mixed *fmt;
  string res;
  int i;
  int c;
  int num, subj;
  string str;
  string bit;
  mapping has = ([]);
  mixed tmp;

  fmt = reg_assoc(msg, ({ "\\$[NnVvTtPpOoRr][a-z0-9]*" }), ({ 1 }) );
  fmt = fmt[0]; // ignore the token info for now

  res = fmt[0];
  i=1;
  while (i<sizeof(fmt))
  {
    c = fmt[i][1];
    if (fmt[i][2] && fmt[i][2]<'a')
    {
      if (fmt[i][3] && fmt[i][3] < 'a')
      {
        subj = fmt[i][2] - '0';
        num = fmt[i][3] - '0';
        str = fmt[i][4..<0];
      } else {
        subj = 0;
        num = fmt[i][2] - '0';
        str = fmt[i][3..<0];
      }
    } else {
      subj = 0;
      num = ((c == 't' || c == 'T') ? 1 : 0); // target defaults to 1, not zero
      str = fmt[i][2..<0];
    }
    switch (c)
    {
      case 'o':
      case 'O':
        ob = obs[num];
        if (arrayp(ob))
        {
          if(sizeof(ob))
          {
            tmp = handle_obs(ob, res, has);
            res = tmp[0];
            ob = tmp[1];
            bit = format_list(ob);
          } else {
            tmp = ({ res });
            for (int z = 0; z < sizeof(ob); z++)
            {
              tmp = handle_ob(ob[z], res, has);
              ob[z] = tmp[1];
            }
            res = tmp[0];
            bit = format_list(ob);
          }
        } else {
          tmp = handle_ob(ob, res, has);
          res = tmp[0];
          bit = tmp[1];
        }
        break;
      case 't':
      case 'T':
/* Only difference between $n and $t is that $t defaults to $n1o */
/* Fall through */
        if (str=="")
          str = "o";
      case 'n':
      case 'N':
        if (str=="")
          str = "s";
        if (str != "p")
        {
          if(str!="d")
          {
/* Handle reflexification */
          if (subj < sizeof(who) &&
             (who[subj] == who[num]) && has[who[subj]])
             {
// objective: You kick yourself, Beek kicks himself.
              if (str == "o")
              {
                if (forwhom == who[subj])
                  bit = "yourself";
                else
                  bit = who[subj]->query_reflexive();
              }
// subjective: You prove you are stupid,
// Beek proves he is stupid.
              if (str == "s")
              {
                if (forwhom == who[subj])
                  bit = "you";
                else
                  bit = who[subj]->query_subjective();
              }
              break;
            }
/* Other pronouns */
            if (who[num]==forwhom)
            {
              bit = "you";
              has[who[num]]++;
              break;
            }
            if (has[who[num]])
            {
              if (str[0]=='o')
                bit = who[num]->query_objective();
              else
                bit = who[num]->query_subjective();
              break;
            }
            has[who[num]]++;
            bit = who[num]->a_short();
            break;
          }
        }
        has[who[num]]++;
        bit = who[num]->short();
        break;
      case 'R':
      case 'r':
        if (forwhom == who[num])
          bit = "yourself";
        else
          bit = who[num]->query_reflexive();
        break;
      case 'v':
      case 'V':
/* hack for contractions */
        if (i + 1 < sizeof(fmt) && fmt[i+1][0..2] == "'t ")
        {
          str += "'t";
          fmt[i+1] = fmt[i+1][2..];
        }

        if (num >= sizeof(who) || who[num]!=forwhom)
          bit = M_GRAMMAR->pluralize(str);
        else
          bit = str;
        break;
      case 'p':
      case 'P':
        //Bit of defensive coding here
        if (num>=sizeof(who))
        {
          bit = "WHOSE(?)";
          break;
        }
        else
        if (forwhom == who[num])
        {
          bit = "your";
          break;
        }
        if (has[who[num]])
        {
          bit = who[num]->query_possessive();
          break;
        }
        bit = who[num]->query_named_possessive();
        has[who[num]]++;
        break;
    }
// hack to prevent errors.
    if (!bit)
      bit = "";
    if (c < 'a')
      bit = capitalize(bit);
//### Hack to avoid inheriting a mixin.  Better one needed.
    if (fmt[i+1][0] == '.')
      res += M_GRAMMAR->punctuate(bit) + fmt[i+1][1..];
    else
      res += bit + fmt[i+1];
    i+=2;
  }
  if ( strlen(res) > 0 && res[<1] != '\n' )
    res += "\n";
  return res;
}

//:FUNCTION action
//Make the messages for a given group of people involved.  The return
//value will have one *per person, as well as one for anyone else.
//inform() can be used to send these messages to the right people.
//see: inform

varargs string *action(object *who, mixed msg, mixed *obs...)
{
  int i;
  string *res;

  if (arrayp(msg))
    msg = choice(msg);
  res = allocate(sizeof(who)+1);
  for (i=0; i<sizeof(who); i++)
    res[i] = compose_message(who[i], msg, who, obs...);
  res[sizeof(who)]=compose_message(0, msg, who, obs...);
  return res;
}

//### This now always indents continuation lines.  Might want a flag at the
//### end to enable or disable that.
//:FUNCTION inform
//Given an *of participants, and an *of messages, and either an
//object or *of objects, deliver each message to the appropriate
//participant, being careful not to deliver a message twice.
//The last arg is either a room, in which that room is told the 'other'
//message, or an *of people to recieve the 'other' message.
void inform(object *who, string *msgs, mixed others)
{
  int i;
  mapping done = ([]);

  for (i=0; i<sizeof(who); i++)
  {
    if (done[who[i]]) continue;
    done[who[i]]++;
    tell(who[i], msgs[i], MSG_INDENT);
  }
  if (arrayp(others))
    map_array(others - who, (: tell_from_outside($1, $(msgs[<1]), MSG_INDENT) :));
  else if (others)
    tell_from_inside(others, msgs[sizeof(who)], MSG_INDENT, who);
}

//:FUNCTION simple_action
//Generate and send messages for an action involving the user and possibly
//some objects
varargs void simple_action(mixed msg, mixed *obs...)
{
  string us;
  string others;
  object *who;

  if( !sizeof( msg )) return;
/* faster to only make who once */
  who = ({ this_object() });
  if (arrayp(msg))
    msg = msg[random(sizeof(msg))];

  us = compose_message(this_object(), msg, who, obs...);
  others = compose_message(0, msg, who, obs...);

  tell(this_object(), us, MSG_INDENT);
  tell_from_outside(all_inventory(this_object()),others,MSG_INDENT);
  tell_environment(this_object(), others, MSG_INDENT, who);
}

//:FUNCTION my_action
//Generate and send a message that should only be seen by the person doing it
varargs void my_action(mixed msg, mixed *obs...)
{
  string us;
  object *who;

  if (!sizeof( msg ))
    return;
  who = ({ this_object() });
  if (arrayp(msg))
    msg = msg[random(sizeof(msg))];
  us = compose_message(this_object(), msg, who, obs...);
  tell(this_object(), us, MSG_INDENT);
}

//:FUNCTION other_action
//Generate and send a message that should only be seen by others
varargs void other_action(mixed msg, mixed *obs...)
{
  string others;
  object *who;

  if( !sizeof(msg)) return;
  who = ({ this_object() });
  if (arrayp(msg))
    msg = msg[random(sizeof(msg))];
  others = compose_message(0, msg, who, obs...);
  tell_from_outside(all_inventory(this_object()),others,MSG_INDENT);
  tell_environment(this_object(), others, MSG_INDENT, who);
}

//:FUNCTION targetted_action
//Generate and send a message involving the doer and a target (and possibly
//other objects)
varargs void targetted_action(mixed msg, object target, mixed *obs...)
{
  string us, them, others;
  object *who;

  if( !sizeof(msg))
    return;
  who = ({ this_object(), target });
  if (arrayp(msg))
    msg = msg[random(sizeof(msg))];
  us = compose_message(this_object(), msg, who, obs...);
  them = compose_message(target, msg, who, obs...);
  others = compose_message(0, msg, who, obs...);
  tell(this_object(), us, MSG_INDENT);
  tell(target, them, MSG_INDENT);
  tell_from_outside(all_inventory(this_object()),others,MSG_INDENT);
  tell_from_outside(all_inventory(target),others,MSG_INDENT);
  tell_environment(this_object(), others, MSG_INDENT, who);
}
