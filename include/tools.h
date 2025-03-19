#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

using namespace std;

void printMap(unordered_map<string, double *> &sharedVariables);

vector<string> split_string(const string &s, char delimiter);

string replace_var(string origini, string remove, string to_replace);
