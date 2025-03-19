#include "../include/tools.h"
#include <algorithm>
#include <cctype>

#ifdef DEBUG_MODE
#define DEBUG_COMMENT(comment) std::cout << "[DEBUG] " << comment << std::endl;
#else
#define DEBUG_COMMENT(comment)
#endif

/// @brief prints the system variables map
/// @param map the pointer to unordered map
void printMap(unordered_map<string, double *> &sharedVariables)
{
    for (auto &pair : sharedVariables)
    {
        std::cout << pair.first << ": " << *(pair.second) << "\n";
    }
}

/// @brief splits the string into different parts given a delimiter and returns a vector of substrings;
/// @param s the string to split
/// @param delimiter the delimiter
/// @return a vector of substrings
vector<string> split_string(const string &s, char delimiter)
{
    vector<string> tokens;
    istringstream ss(s);
    string token;

    while (getline(ss, token, delimiter))
    {
        tokens.push_back(token);
    }

    return tokens;
}

/// @brief Replace occurrences of a substring in a string.
/// This function replaces all occurrences of the substring 'remove' in the string 'origin'
/// with the substring 'to_replace'. It performs replacements only if the substring 'remove'
/// is found in positions where it represents a variable name.
/// @param origin The original string.
/// @param remove The substring to be replaced.
/// @param to_replace The substring to replace 'remove' with.
/// @return The modified string after replacement.
string replace_var(string origin, string remove, string to_replace)
{
    int next;
    int pos = origin.find(remove, 0);
    bool replace_string;
    while (pos != string::npos)
    {
        next = pos + remove.length();
        replace_string = false;
        if (pos + 1 < origin.length() && pos - 1 >= 0)
        { // something left and something right
            if (!isalpha(origin[next]) && !isdigit(origin[next]) && !isalpha(origin[pos - 1]) && !isdigit(origin[pos - 1]))
                replace_string = true;
        }
        else if (pos + 1 >= origin.length())
        { // something left
            if (!isalpha(origin[pos - 1]) && !isdigit(origin[pos - 1]))
                replace_string = true;
        }
        else
        { // something right
            if (!isalpha(origin[next]) && !isdigit(origin[next]))
                replace_string = true;
        }

        if (replace_string)
        {
            origin.replace(pos, remove.length(), to_replace);
        }
        pos = origin.find(remove, pos + 1);
    }
    setlocale(LC_ALL, "C");

    return origin;
}
