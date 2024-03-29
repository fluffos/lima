/* Do not remove the headers from this file! see /USAGE for more info. */

/*
 * Originally the utility port handler for the i3 router
 * Cowl@Orion
 *
 * 11-19-95 Cowl: Added args for gateways
 * 12-27-95 Cowl: added home dirs for wizards and converted
 *                to a web server ( almost )
 * 01-05-96 Cowl: Converted socket to a binary, for graphics
 *                save for a minor bug, convert_to_actual_path() works.
 * 01-07-96 Cowl: fixed convert_to_actual_path()
 *                added gateway security
 *
 * Jan 20, 1997 Myth@Icon of Sin
 *  o Added a write callback function for large files.
 *
 * Jan 26, 1997 Rust
 *  o removed CGI security for a bit. Fixed up a bunch of stuff,
 *    and added POST support.
 *
 * Jan 18, 1988 Tigran
 *  o Fixed CGI stuff so that Content-type: is sent
 *  o Made more errors pass properly
 *  o \n is translated to \r\n
 *  o other miscellaneous changes.
 *
 * TODO:
 *      * Hit logging and related stats
 *      * Support for multiple arguments to cgis
 *      * Better Error handling (only 404 so far ;) )

 *
 */
#include <http_d.h>
#include <log.h>
#include <ports.h>
#include <socket.h>

class client_request
{
   int any_input_yet_p;
   int end_of_input;
   int keepalive;
   int remaining_content_length;
   int reached_end_of_headers;
   mapping headers;
   string content;
   string http_version;
   string method;
   string request;
}

mapping content_types =
    (["pfr":"application/font-tdpfr",
        "movie":"video/x-sgi-movie", "man":"application/x-troff-man", "ms":"application/x-troff-ms",
           "me":"application/x-troff-me", "t":"application/x-troff", "tr":"application/x-troff",
         "roff":"application/x-troff", "enc":"application/pre-encrypted", "ckl":"application/x-fortezza-ckl",
          "crl":"application/x-pkcs7-crl", "p7s":"application/x-pkcs7-signature", "p7m":"application/x-pkcs7-mime",
          "p7c":"application/x-pkcs7-mime", "jsc":"application/x-javascript-config",
          "pac":"application/x-ns-proxy-autoconfig", "js":"application/x-javascript",
        "mocha":"application/x-javascript", "pl":"application/x-perl", "tcl":"application/x-tcl",
           "sh":"application/x-sh", "csh":"application/x-csh", "ai":"application/postscript",
          "eps":"application/postscript", "ps":"application/postscript", "exe":"application/octet-stream",
          "bin":"application/octet-stream", "jar":"application/java-archive", "cpio":"application/x-cpio",
         "gtar":"application/x-gtar", "tar":"application/x-tar", "shar":"application/x-shar",
          "zip":"application/x-zip-compressed", "sit":"application/x-stuffit", "hqx":"application/mac-binhex40",
          "avi":"video/x-msvideo", "qt":"video/quicktime", "mov":"video/quicktime", "moov":"video/quicktime",
         "mpv2":"video/x-mpeg2", "mp2v":"video/x-mpeg2", "mpeg":"video/mpeg", "mpg":"video/mpeg", "mpe":"video/mpeg",
          "mpv":"video/mpeg", "vbs":"video/mpeg", "mpegv":"video/mpeg", "ra":"audio/x-pn-realaudio",
          "ram":"audio/x-pn-realaudio", "mp2":"audio/x-mpeg", "mpa":"audio/x-mpeg", "abs":"audio/x-mpeg",
        "mpega":"audio/x-mpeg", "wav":"audio/x-wav", "aif":"audio/x-aiff", "aiff":"audio/x-aiff", "aifc":"audio/x-aiff",
           "au":"audio/basic", "snd":"audio/basic", "fif":"application/fractals", "ief":"image/ief", "png":"image/png",
          "pcd":"image/x-photo-cd", "bmp":"image/x-MS-bmp", "rgb":"image/x-rgb", "ppm":"image/x-portable-pixmap",
          "pgm":"image/x-portable-greymap", "pbm":"image/x-portable-bitmap", "pnm":"image/x-portable-anymap",
          "xwd":"image/x-xwindowdump", "xpm":"image/x-xpixmap", "xbm":"image/x-xbitmap", "ras":"image/x-cmu-raster",
         "tiff":"image/tiff", "tif":"image/tiff", "jpeg":"image/jpeg", "jpg":"image/jpeg", "jpe":"image/jpeg",
         "jfif":"image/jpeg", "pjpeg":"image/jpeg", "pjp":"image/jpeg", "gif":"image/gif", "vcf":"text/x-vcard",
         "texi":"application/x-texinfo", "dvi":"application/x-dvi", "latex":"application/x-latex",
          "tex":"application/x-tex", "pdf":"application/pdf", "rtf":"application/rtf", "html":"text/html",
          "htm":"text/html", "txt":"text/plain", "text":"text/plain", ]);

private
nosave mapping sending = ([]);
private
nosave object http_sock;

// This only operates on the first 2 chars.
int hex_str_to_int(string str)
{
   int result;

   switch (str[0])
   {
   case 'a':
   case 'b':
   case 'c':
   case 'd':
   case 'e':
   case 'f':
      result = (str[0] - 'a' + 10) * 16;
      break;
   case 'A':
   case 'B':
   case 'C':
   case 'D':
   case 'E':
   case 'F':
      result = (str[0] - 'A' + 10) * 16;
      break;
   default:
      result = (str[0] - '0') * 16;
   }

   switch (str[1])
   {
   case 'a':
   case 'b':
   case 'c':
   case 'd':
   case 'e':
   case 'f':
      return result + (str[1] - 'a' + 10);
      break;
   case 'A':
   case 'B':
   case 'C':
   case 'D':
   case 'E':
   case 'F':
      return result + (str[1] - 'A' + 10);
      break;
   default:
      return result + (str[1] - '0');
   }
}

string html_decode(string str)
{
   string outstr = "";
   int i = 0;

   for (i = 0; i < strlen(str); i++)
   {
      switch (str[i])
      {
      case '+':
         outstr = outstr + " ";
         break;
      case '%':
         outstr = sprintf("%s%c", outstr, hex_str_to_int(str[i + 1..]));
         i += 2;
         break;
      default:
         outstr = sprintf("%s%c", outstr, str[i]);
      }
   }
   return outstr;
}

mapping form_parse(string s)
{
   mapping result = ([]);
   string *entries = explode(s, "&");
   string *halves;

   foreach (string entry in entries)
   {
      halves = explode(entry, "=");
      if (sizeof(halves) == 1)
      {
         // Tmp hack, because explode() is currently not sane. "foo="
         // is only returning ({"foo"})
         if (entry[ < 1] == '=')
         {
            result[html_decode(halves[0])] = 0;
            continue;
         }
         result[html_decode(entry)] = 0;
         continue;
      }
      result[html_decode(halves[0])] = html_decode(halves[1]);
   }
   return result;
}
private
string expand_user(string path)
{
   string part;
   int i = strsrch(path, '/');
   if (i == -1)
   {
      part = "";
      i = strlen(path);
   }
   else
   {
      part = path[i + 1..];
   }
   return join_path(join_path(WIZ_DIR, path[1..i]), join_path(HTTP_USER_HOME, part));
}

// Rewritten by Rust.
private
string convert_to_actual_path(string path)
{
   /* Zif added this.  its not the most elegant way of doing
    * It i am sure, however people were able to do any number
    * of ../'s to get to any file in the mudlib...
    * and I tried to use replace_string on that and had no luck.
    *
    * i.e. lima.mudlib.org:3003/../secure/master.c
    *      lima.mudlib.org:3003/../../../../secure/master.c
    */

   path = evaluate_path(path);
   if (path[0] == '/')
      path = path[1..];

   if (path[0] == '~')
      path = expand_user(path);
   else
   {
      if (path[0..4] == "scgi/")
      {
         path = join_path(SECURE_CGI_DIR, path[5..]);
      }
      else
      {
         path = join_path(HTTP_ROOT, path);
      }
   }

   if (path[ < 1] == '/')
      return join_path(path, DEFAULT_PAGE);
   else
      return path;
}

private
void write_callback(object socket)
{
   string file;
   int i1, i2, i3;
   if (!socket)
      return;
   if (!sending[socket])
   {
      socket->remove();
      return;
   }
   file = sending[socket][0];
   i1 = sending[socket][1];

   i2 = HTTP_BLOCK_SIZE;

   i3 = file_size(file);

   // In case the file gets deleted in the middle of a transfer.
   if (i3 < 0)
   {
      socket->remove();
      return;
   }

   if (i1 + i2 > file_size(file))
   {
      i2 = file_size(file) - i1;
      map_delete(sending, socket);
   }
   else
   {
      sending[socket] = ({file, i1 + i2});
   }
   socket->send(read_buffer(file, i1, i2));
}

private
void remove_connection(object s)
{
   if (s)
   {
      s->remove();
   }
}

varargs private nomask void http_send(mixed text, object socket, string ext)
{
   string send;
   string day_of_week;
   string month;
   mixed body;
   int day_of_month;
   int hour;
   int minute;
   int second;
   int year;
   string content;
   string time_string = ctime(time());

   sscanf(time_string, "%s %s %d %d:%d:%d %d", day_of_week, month, day_of_month, hour, minute, second, year);
   time_string = sprintf("%s, %d %s %d %d:%d:%d GMT", day_of_week, day_of_month, month, year, hour, minute, second);

   content = content_types[ext];
   if (!content)
      content = "text/html";
   if (stringp(text) && strlen(text) > 0)
      body = replace_string(text, "\n", "\r\n");
   send = sprintf("HTTP/1.1 200 OK\r\n"
                  "Date: %s\r\n"
                  "Server: Lima/1.1\r\n"
                  "Connection: close\r\n"
                  "Content-Type: %s\r\n"
                  "Content-Length: %i\r\n"
                  "\r\n",
                  time_string, content, body ? sizeof(body) : file_size(sending[socket][0]));
   socket->send(send);
   if (body)
      socket->send(body);
   else
      write_callback(socket);
   return;
}

private
nomask void output_error(string header, string text, object socket)
{
   socket->send(
       sprintf("<html><head><title>%s</title></head><body><h1>%s</h1><p>%s\n</body></html>\r\n", header, header, text));
   /* Hack to avoid connection reset by peer */
   call_out(( : remove_connection($(socket)) :), 0);
}

private
nomask void handle_post_request(class client_request req, object sock)
{
   mapping form_info = form_parse(req.content);
   string file = convert_to_actual_path(req.request);
   string result, err;

   if (file_size(file) < 1)
   {
      output_error("404 Not found", "The requested URL was not found", sock);
      return;
   }

   err = catch (result = file->main(form_info, req.headers));
   if (!result)
   {
      output_error("502 Bad Gateway", err, sock);
   }
   sock->send(result + "\n");
   sock->remove();
}
private
nomask void handle_get_request(class client_request req, object socket)
{
   string extention, args;
   string file;

   sscanf(req.request, "%s?%s", file, args);
   if (!file)
   {
      file = req.request;
   }
   file = convert_to_actual_path(file);
   if (file_size(file) < 1)
   {
      output_error("404 Not found", "The requested URL was not found", socket);
      return;
   }
   sscanf(file, "%s.%s", file, extention);
   // if it's extention is '.c' then we assume it is a gateway and
   // we send the output of the gateway object's main() to the client.
   // if the extention is anything other than '.c' we'll treat it as
   // an HTML document. gateways can only be called from a secure dir.
   switch (extention)
   {
      string result, err;
      mixed foo;

   case "c":
      if (args)
         err = catch (result = call_other(file, "main", args));
      else
         err = catch (result = call_other(file, "main"));
      if (!result)
         output_error("502 Bad Gateway", err, socket);
      http_send(result + "\n", socket);
      socket->remove();
      break;

   default:
      sending[socket] = ({file + "." + extention, 0});
      socket->set_write_callback(( : write_callback:));
      http_send("", socket, extention);
      //	http_send(write_callback(socket),socket,extention);
      break;
   }
}

mapping requests = ([]);

class client_request alloc_request(object socket)
{
   class client_request req = new (class client_request);
   req.keepalive = 0;
   req.headers = ([]);
   req.reached_end_of_headers = 0;
   req.any_input_yet_p = 0;
   req.end_of_input = 0;
   req.remaining_content_length = 0;
   req.content = "";
   requests[socket] = req;
   return req;
}

private
nomask void handle_request(object socket)
{
   class client_request req;

   req = requests[socket];
   if (!req || req.end_of_input)
   {
      // A read sometimes gets called after we've groked everything...
      return;
   }
   if (!req.reached_end_of_headers)
      return;
   req.end_of_input = 1;
   //  BBUG(req);
   // uncommenting this will cause the req.method in the next line of
   // code to bug.
   //  map_delete(requests, socket);
   switch (req.method)
   {
   case "GET":
      handle_get_request(req, socket);
      break;
   case "POST":
      handle_post_request(req, socket);
      break;
   default:
      output_error("501 Method not implemented.", "Right now only the GET and POST methods are implemented.", socket);
   }
}

void add_text_to_request(string text, object socket)
{
   class client_request req;

   req = requests[socket];

   req.remaining_content_length -= strlen(text);
   req.content += text;

   if (req.remaining_content_length < 0)
   {
      req.content = req->content[0.. < (-(req.remaining_content_length)) + 1];
   }
   if (req.remaining_content_length <= 0)
   {
      handle_request(socket);
   }
}

void look_for_useful_header_info(object socket)
{
   class client_request req;

   req = requests[socket];

   if (!req.keepalive)
   {
      string connection_type;

      connection_type = req.headers["connection"];
      if (connection_type && lower_case(connection_type) == "keep-alive")
      {
         req.keepalive = -1;
      }
   }
   if (req.keepalive)
   {
      string content_length_header;
      content_length_header = req.headers["content-length"];
      if (content_length_header)
      {
         req.remaining_content_length = to_int(content_length_header);
      }
   }
}

string parse_headers(string text, object socket)
{
   class client_request req;
   string header_name, header_value;
   string *lines;
   int i = 0;

   req = requests[socket];
   lines = explode(text, "\n");
   if (!req.any_input_yet_p)
   {
      string *request_line = explode(lines[0], " ");
      request_line -= ({"\r"});
      if (sizeof(request_line) < 3)
         return;
      req.method = request_line[0];
      req.request = request_line[1];
      req.http_version = request_line[2][0.. < 2];
      i = 1;
      req.any_input_yet_p = 1;
   }
   for (; i < sizeof(lines); i++)
   {
      string line = trim(lines[i]);
      int colon_index;

      if (!strlen(line))
      {
         look_for_useful_header_info(socket);
         req.reached_end_of_headers = 1;
         if (sizeof(lines) != i + 1)
         {
            return implode(lines[i + 1..], "\n");
         }
         else
         {
            return 0;
         }
      }
      colon_index = strsrch(lines[i], ':');
      header_name = lower_case(trim(lines[i][0..(colon_index - 1)]));
      header_value = trim(lines[i][colon_index + 1..]);
      req.headers[header_name] = header_value;
   }
   look_for_useful_header_info(socket);
   /*
   if (!req.keepalive)
   {
   req.reached_end_of_headers = 1;
   }
   */
   return 0;
}

private
nomask void add_to_request(object socket, string text)
{
   class client_request req;

   req = requests[socket];
   if (!req)
   {
      req = alloc_request(socket);
   }
   // We haven't gotten to the end of headers yet.
   if (!(req.reached_end_of_headers))
   {
      text = parse_headers(text, socket);
   }
   if (text)
   {
      add_text_to_request(text, socket);
   }
   if (req.remaining_content_length <= 0 && req.reached_end_of_headers)
   {
      handle_request(socket);
   }
}

private
nomask void http_read(object socket, buffer data)
{
   string request;

   if (!data)
      return;
   // initial connect
   request = read_buffer(data);
   if (!request)
      return;

   add_to_request(socket, request);
}

private
nomask void http_close(object socket)
{
   // add logging or something later
}

nomask void remove()
{
   http_sock->remove();
}

nomask void create()
{
   string ret;
   ret = catch (http_sock = new (SOCKET, SKT_STYLE_LISTEN_B, PORT_HTTP, ( : http_read:), ( : http_close:)));
   if (ret)
   {
      if (http_sock)
      {
         http_sock->remove();
         http_sock = 0;
      }
      error(ret);
   }
}
//**  added to stop the daemon from cleaning itself up

void clean_up()
{
   return 0;
}
