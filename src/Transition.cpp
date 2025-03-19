#include "../include/Objects.h"
#include <iostream>
#include <stack>
#include "../include/tools.h"
#include "../include/tinyexpr.h"

#ifdef DEBUG_MODE
#define DEBUG_COMMENT(comment) std::cout << "[DEBUG] " << comment << std::endl;
#else
#define DEBUG_COMMENT(comment)
#endif

/// @brief constructor
/// @param condition the condition of the transition
Transition::Transition(string condition)
{
    this->condition = condition;
}

/// @brief return the condition of the transition
/// @return the condition
string Transition::getCondition()
{
    return condition;
}

/// @brief set the condition of the transition
/// @param condition the condition of the transition
void Transition::setCondition(string condition)
{
    this->condition = condition;
}

/// @brief evaluate the single expression like "(5>9)" --> 0 (false))
/// @param expression the expression to evaluate
/// @return the evaluation of the expression
bool Transition::evaluateSingleEquation(string &expression)
{

    for (size_t i = 0; i < expression.length(); ++i)
    {
        if (expression[i] == '[')
        {
            expression[i] = '(';
        }
        else if (expression[i] == ']')
        {
            expression[i] = ')';
        }
    }

    size_t pos = expression.find("<=");
    if (pos != string::npos)
    {
        string left = expression.substr(0, pos);
        string right = expression.substr(pos + 2);
        double a = te_interp(left.c_str(), 0);
        double b = te_interp(right.c_str(), 0);
        return a <= b;
    }

    pos = expression.find(">=");
    if (pos != string::npos)
    {
        string left = expression.substr(0, pos);
        string right = expression.substr(pos + 2);
        double a = te_interp(left.c_str(), 0);
        double b = te_interp(right.c_str(), 0);
        return a >= b;
    }

    pos = expression.find(">");
    if (pos != string::npos)
    {
        string left = expression.substr(0, pos);
        string right = expression.substr(pos + 1);
        double a = te_interp(left.c_str(), 0);
        double b = te_interp(right.c_str(), 0);
        return a > b;
    }

    pos = expression.find("<");
    if (pos != string::npos)
    {
        string left = expression.substr(0, pos);
        string right = expression.substr(pos + 1);
        double a = te_interp(left.c_str(), 0);
        double b = te_interp(right.c_str(), 0);
        return a < b;
    }

    pos = expression.find("==");
    if (pos != string::npos)
    {
        string left = expression.substr(0, pos);
        string right = expression.substr(pos + 2);
        double a = te_interp(left.c_str(), 0);
        double b = te_interp(right.c_str(), 0);
        return a == b;
    }

    pos = expression.find("!=");
    if (pos != string::npos)
    {
        string left = expression.substr(0, pos);
        string right = expression.substr(pos + 2);
        double a = te_interp(left.c_str(), 0);
        double b = te_interp(right.c_str(), 0);
        return a != b;
    }
    // Return false for invalid expressions
    return false;
}

/// @brief Evaluate ONLY logical expression with true false like "(1 & 1) | (1 | 0)"
/// @param str the logical expression to evaluate
/// @return 0 or 1 (false, true)
char Transition::evaluateLogicalEquation(string str)
{
    stack<char> arr;

    // traversing string from the end.
    for (int i = str.length() - 1; i >= 0; i--)
    {
        if (str[i] == '[')
        {
            vector<char> s;
            while (arr.top() != ']')
            {
                s.push_back(arr.top());
                arr.pop();
            }
            arr.pop();

            // for NOT operation
            if (s.size() == 3)
            {
                s[2] == '1' ? arr.push('0') : arr.push('1');
            }
            // for AND and OR operation
            else if (s.size() == 5)
            {
                int a = s[0] - 48, b = s[4] - 48, c;
                s[2] == '&' ? c = a &&b : c = a || b;
                arr.push((char)c + 48);
            }
        }
        else
        {
            arr.push(str[i]);
        }
    }
    return arr.top();
}

/// @brief Scraping the initial equation to obtain a simple logical expression and then solve it
/// @param str the initial equation
/// @return the evaluation
bool Transition::solve(string str)
{
    string expressionFinal = "";
    string expressionTemp = "";
    int flag = 0;
    for (char c : str)
    {
        if (c == '(') // Identifying the beginning of comparison between numbers
        {
            flag = 1;
        }
        else if (c == ')') // Identifying the ending of comparison between numbers
        {
            flag = 0;
            expressionFinal += evaluateSingleEquation(expressionTemp) ? "1" : "0";
            expressionTemp = "";
        }

        if (flag) // build the string with two numbers to be comparised
        {
            if (c != '(')
                expressionTemp += c;
        }
        else // build the final logic expression string to be analyzed
        {
            if (c != ')')
                expressionFinal += c;
        }
    }
    return evaluateLogicalEquation(expressionFinal) == '1' ? 1 : 0;
}

/// @brief perform pre-processing to make evaluation easier
/// @param str the string to pre-process
/// @return the processed string to evaluate
string preProcessing(string str)
{
    string newStr = "";
    for (char c : str)
    {
        newStr += c;
        if (c == '&' || c == '|')
        {
            newStr[newStr.size() - 1] = ',';
            newStr += c;
            newStr += ',';
        }
    }
    return newStr;
}

/// @brief replace the variables in the condition with the real values and check them
/// @param variables the map containing the couple <name, value>
/// @return the evaluation
bool Transition::checkCondition(unordered_map<string, double *> &variables, unordered_map<string, double> &justModified)
{
    string condition = getCondition();
    int pos;

    DEBUG_COMMENT("\nCondizione pre-replace: " << condition << "\n");

    for (pair<string, double *> pair : variables)
    {
        if (justModified.contains(pair.first))
        {
            condition = replace_var(condition, pair.first, to_string(justModified[pair.first]));
        }
        else
        {
            condition = replace_var(condition, pair.first, to_string(*(pair.second)));
        }
    }

    DEBUG_COMMENT("Condizione post-replace: " << condition << "\n");

    bool output = solve(preProcessing(condition));

    DEBUG_COMMENT("Valutato: " << output << "\n");

    return output;
}

/// @brief to string
ostream &operator<<(ostream &os, Transition &obj)
{
    os << obj.getCondition();
    return os;
}
