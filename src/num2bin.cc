#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

std::string num2bin(unsigned int a) 
{
  std::string str;
  for(int i = 0; i < 32; ++i)
    {
      if (i % 8 == 0)
        {
          str += ' ';
          str += ' ';
    }
      else if (i % 4 == 0)
        str += ' ';
  
      if (a & (1 << i))
        str += '1';
      else
        str += '0';
    }

  std::reverse(str.begin(), str.end());
  return "  ->   " + str;
}

int main(int argc, char** argv)
{
  if (argc == 1)
    {
      std::string line;
      while(std::getline(std::cin, line))
        {
          if (!line.empty())
            {
              int number;
              std::istringstream str(line); str >> number;

              std::cout << std::setw(16) << number
                        << " "
                        << std::setw(40) << num2bin(number)
                        << std::endl;
            }
          else
            {
              std::cout << std::endl;
            }
        }
    }
  else
    {
      for(int i = 1; i < argc; ++i)
        {
          std::string line;
          std::ifstream in(argv[1]);

          while(std::getline(in, line))
            {
              std::cout << "Line: " << line.length() << std::endl;
              if (line.empty())
                {
                  int number;
                  std::istringstream str(line); str >> number;

                  std::cout << std::setw(16) << number
                            << " " 
                            << std::setw(60) << num2bin(number)
                            << std::endl;
                }
              else
                {
                  std::cout << std::endl;
                }

              line = "";
            }
      
          in.close();
        }
    }    
}

/* EOF */
