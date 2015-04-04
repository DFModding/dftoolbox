#include <string.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>

std::string escape(const std::string& text)
{
  std::string res;
  for(std::string::size_type i = 0; i < text.length(); ++i)
    {
      if (text[i] == '"')
        res += "\\\"";
      else
        res += text[i];
    }
  return res;
}

bool has_prefix(const std::string& lhs, const std::string rhs)
{
  if (lhs.length() < rhs.length())
    return false;
  else
    return lhs.compare(0, rhs.length(), rhs) == 0;
}

class POFileReader
{
private:
  struct Token
  {
    std::string keyword;
    std::string content;
  };

  std::vector<std::string>& dict;

  std::string from_charset;
  std::string to_charset;

  std::string current_msgid;
  std::string current_msgid_plural;
  std::map<int, std::string> msgstr_plural;

  int line_num;

  enum { WANT_MSGID, WANT_MSGSTR, WANT_MSGSTR_PLURAL, WANT_MSGID_PLURAL } state;

public:
  POFileReader(std::istream& in, std::vector<std::string>& dict_)
    : dict(dict_)
  {
    state = WANT_MSGID;
    line_num = 0;
    char c = in.get();
    if(c == (char) 0xef) { // skip UTF-8 intro that some texteditors produce
      in.get();
      in.get();
    } else {
      in.unget();
    }
    tokenize_po(in);
  }

  void parse_header(const std::string& header)
  {
    // Seperate the header in lines
    typedef std::vector<std::string> Lines;
    Lines lines;
    
    std::string::size_type start = 0;
    for(std::string::size_type i = 0; i < header.length(); ++i)
      {
        if (header[i] == '\n')
          {
            lines.push_back(header.substr(start, i - start));
            start = i+1;
          }
      }

    for(Lines::iterator i = lines.begin(); i != lines.end(); ++i)
      {
        if (has_prefix(*i, "Content-Type: text/plain; charset=")) {
          from_charset = i->substr(strlen("Content-Type: text/plain; charset="));
        }
      }

    if (from_charset.empty() || from_charset == "CHARSET")
      {
        std::cerr << "Error: Charset not specified for .po, fallback to ISO-8859-1" << std::endl;
        from_charset = "ISO-8859-1";
      }

    //to_charset = dict.get_charset();

    //if (to_charset.empty())
    { // No charset requested from the dict, use utf-8
      //to_charset = "utf-8";
      //dict.set_charset(from_charset);
      //}
    }
  }

  void add_token(const Token& token)
  {
    switch(state) 
      {
      case WANT_MSGID:
        if (token.keyword == "msgid") 
          {
            current_msgid = token.content;
            state = WANT_MSGID_PLURAL;
          }
        else if (token.keyword.empty())
          {
            //std::cerr << "Got EOF, everything looks ok." << std::endl;
          }
        else
          {
            std::cerr << "tinygettext: expected 'msgid' keyword, got '" << token.keyword 
                      << "' at line " << line_num << std::endl;
          }
        break;
    
      case WANT_MSGID_PLURAL:
        if (token.keyword == "msgid_plural") 
          {
            current_msgid_plural = token.content;
            state = WANT_MSGSTR_PLURAL;
          } 
        else
          {
            state = WANT_MSGSTR;
            add_token(token);
          }
        break;

      case WANT_MSGSTR:
        if (token.keyword == "msgstr") 
          {
            if (current_msgid == "") 
              { // .po Header is hidden in the msgid with the empty string
                parse_header(token.content);
              }
            else
              {
                dict.push_back(current_msgid);
              }
            state = WANT_MSGID;
          } 
        else
          {
            std::cerr << "tinygettext: expected 'msgstr' keyword, got " << token.keyword 
                      << " at line " << line_num << std::endl;
          }
        break;

      case WANT_MSGSTR_PLURAL:
        if (has_prefix(token.keyword, "msgstr[")) 
          {
            int num;
            if (sscanf(token.keyword.c_str(), "msgstr[%d]", &num) != 1) 
              {
                std::cerr << "Error: Couldn't parse: " << token.keyword << std::endl;
              } 
            else 
              {
                msgstr_plural[num] = token.content;
              }
          }
        else 
          {
            dict.push_back(current_msgid);

            state = WANT_MSGID;
            add_token(token);
          }
        break;
      }
  }
  
  inline int getchar(std::istream& in) 
  {
    int c = in.get();
    if (c == '\n')
      line_num += 1;
    return c;
  }
  
  void tokenize_po(std::istream& in)
  {
    enum State { READ_KEYWORD, 
                 READ_CONTENT,
                 READ_CONTENT_IN_STRING,
                 SKIP_COMMENT };

    State state = READ_KEYWORD;
    int c;
    Token token;

    while((c = getchar(in)) != EOF)
      {
        //std::cout << "Lexing char: " << char(c) << " " << state << std::endl;
        switch(state)
          {
          case READ_KEYWORD:
            if (c == '#')
              {
                state = SKIP_COMMENT;
              }
            else
              {
                // Read a new token
                token = Token();
                
                do { // Read keyword 
                  token.keyword += c;
                } while((c = getchar(in)) != EOF && !isspace(c));
                in.unget();

                state = READ_CONTENT;
              }
            break;

          case READ_CONTENT:
            while((c = getchar(in)) != EOF)
              {
                if (c == '"') { 
                  // Found start of content
                  state = READ_CONTENT_IN_STRING;
                  break;
                } else if (isspace(c)) {
                  // skip
                } else { // Read something that may be a keyword
                  in.unget();
                  state = READ_KEYWORD;
                  add_token(token);
                  break;
                }
              }
            break;

          case READ_CONTENT_IN_STRING:
            if (c == '\\') {
              c = getchar(in);
              if (c != EOF)
                {
                  if (c == 'n') token.content += '\n';
                  else if (c == 't') token.content += '\t';
                  else if (c == 'r') token.content += '\r';
                  else if (c == '"') token.content += '"';
                  else
                    {
                      std::cout << "Unhandled escape character: " << char(c) << std::endl;
                    }
                }
              else
                {
                  std::cout << "Unterminated string" << std::endl;
                }
            } else if (c == '"') { // Content string is terminated
              state = READ_CONTENT;
            } else {
              token.content += c;
            }
            break;

          case SKIP_COMMENT:
            if (c == '\n')
              state = READ_KEYWORD;
            break;
          }
      }
    add_token(token);
  }
};


int main(int argc, char** argv)
{
  std::vector<std::string> dictionary_en;
  std::vector<std::string> dictionary_nw;

  std::ifstream en_in(argv[1]);
  std::ifstream nw_in(argv[2]);

  POFileReader reader1(en_in, dictionary_en);
  POFileReader reader2(nw_in, dictionary_nw);
  
  //  std::cout << dictionary_en.size() << " "
  //            << dictionary_nw.size() << std::endl;

  for(int i = 0; i < int(dictionary_en.size()); ++i)
    {
      std::cout << "msgid \"" << escape(dictionary_en[i]) << "\"" << std::endl;
      std::cout << "msgstr \"" << escape(dictionary_nw[i]) << "\"" << std::endl;
      std::cout << std::endl;
    }

  return 0;
}

