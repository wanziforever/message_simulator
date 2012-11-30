#include <iostream>
#include <sstream>
#include <string>
using namespace std;

int main()
{
  string s("   Somewhere     down the road");
  istringstream iss(s);

  do
  {
    string sub;
    iss >> sub;
    cout << "Substring: " << sub << endl;
  } while (iss);

  string b("");
  int bb = 3;
  std::stringstream(b) >> bb;
  cout << "convert to int " << bb << endl;

  return 0;
}
