#include "../include/Objects.h"
#include "../include/tools.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include "../include/tinyexpr.h"

#ifdef DEBUG_MODE
#define DEBUG_COMMENT(comment) std::cout << "[DEBUG] " << comment << std::endl;
#else
#define DEBUG_COMMENT(comment)
#endif

/// @brief empty constructor
Node::Node()
{
}

/// @brief constructor
/// @param name the name of the node
/// @param description the description of the node
Node::Node(string name, string description, string instructions, bool firstVisit, double delta, double numSeconds)
{
    this->name = name;
    this->description = description;
    this->instructions = instructions;
    this->firstVisit = firstVisit;
    this->delta = delta;
    this->numSeconds = numSeconds;
}

/// @brief returns the name of the node
/// @return Node.name
string Node::getName()
{
    return this->name;
}

/// @brief change the name of the node
/// @param name the new name
void Node::setName(string name)
{
    this->name = name;
}

/// @brief returns the description of the node
/// @return Node.description
string Node::getDescription()
{
    return this->description;
}

/// @brief change the description of the node
/// @param description the new description
void Node::setDescription(string description)
{
    this->description = description;
}

/// @brief returns all the transitions of the node
/// @return Node.transitions
unordered_map<Transition, string, TransitionHash, TransitionEqual> Node::getTransitions()
{
    return this->transitions;
}

/// @brief change the transitions of the node
/// @param transitions the new transitions
void Node::setTransitions(unordered_map<Transition, string, TransitionHash, TransitionEqual> &transitions)
{
    this->transitions = transitions;
}

/// @brief override of the '==' operator
/// @param other the other object
/// @return true if equals, false otherwise
bool Node::operator==(const Node &other)
{
    return this->name == other.name;
}

/// @brief adds a transition to the transitions of the node
/// @param condition
/// @param destination
void Node::addTransition(string condition, string destination)
{
    Transition *aux = new Transition(condition);
    this->transitionKeys.push_back(*aux);
    this->transitions[*aux] = destination;
}

/// @brief returns all the transitions' condition
/// @return Node.transitionKeys
vector<Transition> Node::getTransitionKeys()
{
    return transitionKeys;
}

/// @brief set the value of firstVisit
/// @param value the value to assign
void Node::setFirstVisit(bool value)
{
    this->firstVisit = value;
}

/// @brief return the value of firstVisit
/// @return value if firstVisit
bool Node::getFirstVisit()
{
    return this->firstVisit;
}

/// @brief return the actual node istruction
/// @return the actual node istructions
string Node::getActualInstructions()
{
    return this->instructions;
}

void Node::setFileValues(unordered_map<string, double> newValues){
    for (const auto& pair : newValues) {
        fileValues[pair.first] = pair.second;
    }
}

/// @brief resolve a first order differential equation
/// @param eq the equation
/// @param cauchy initial condition
/// @param t0 time to return
/// @param h delta
/// @param t_final final time
/// @return
double Node::ode_solver(string eq, double cauchy, int t0, double h, double t_final, unordered_map<string, double *> &sharedVariables)
{
    long num_steps = static_cast<long>(t_final / h) + 1;
    vector<string> aux = split_string(eq, '=');

    DEBUG_COMMENT("aux[0]: " << aux[0] << ", [1]: " << aux[1] << "\n");

    string copia = aux[1];

    vector<string> var = split_string(eq, '\'');

    // if the string has variables, then we replace their values
    for (pair<string, double *> pair : sharedVariables)
    {
        if (pair.first != var[0])
            copia = replace_var(copia, pair.first, to_string(*(pair.second)));

        /*int pos = copia.find(pair.first);
        while (pos != string::npos)
        {
            copia.replace(pos, pair.first.length(), to_string(*(pair.second)));
            pos = copia.find(pair.first);
        }*/
    }
    /*aux[1] = copia;
    int i;
    for (i = 0; (i < num_steps - 1) && (i < t0); ++i)
    {
        // replacing variables with their actual values

        // DEBUG_COMMENT("Pre-Replace: " << aux[1] << "\n");

        aux[1] = replace_var(aux[1], var[0], to_string(ystar[i]));

        aux[1] = replace_var(aux[1], "t", to_string(t[i]));

        // DEBUG_COMMENT("Post-Replace: " << aux[1] << "\n");
        DEBUG_COMMENT("k1= " << aux[1].c_str() << "\n");
        double k1 = te_interp(aux[1].c_str(), 0);
        DEBUG_COMMENT("Operazione: " << ystar[i] << "+" << h << "*" << k1 << "\n");

        ystar[i + 1] = ystar[i] + h * k1;
        t[i + 1] = t[i] + h;
        aux[1] = copia;
    }*/

    /*for (int i = 0; i < ode_solver_values.size(); i++)
    {
        cout << "Pos: " << i << "->" << ode_solver_values[i] << "\n";
    }*/

    aux[1] = copia;
    double new_value;
    double new_time;

    aux[1] = replace_var(aux[1], var[0], to_string(ode_solver_values[t0 - 1]));
    aux[1] = replace_var(aux[1], "t", to_string(ode_solver_times[t0 - 1]));
    DEBUG_COMMENT("k1= " << aux[1].c_str() << "\n");
    double k1 = te_interp(aux[1].c_str(), 0);
    DEBUG_COMMENT("Operazione: " << ode_solver_values[t0 - 1] << "+" << h << "*" << k1 << "\n");
    new_value = ode_solver_values[t0 - 1] + h * k1;
    new_time = ode_solver_times[t0 - 1] + h;
    ode_solver_values.push_back(new_value);
    ode_solver_times.push_back(new_time);

    return ode_solver_values[t0];
}

/// @brief execute all the node instructions
/// @param sharedVariables the system variables
void Node::executeNodeInstructions(unordered_map<string, double *> &sharedVariables, unordered_map<string, double> &tempVariables, int time)
{
    // removing all the spaces
    instructions.erase(std::remove(instructions.begin(), instructions.end(), ' '), instructions.end());
    instructions.erase(std::remove(instructions.begin(), instructions.end(), '\n'), instructions.end());

    vector<string> distinctInstructions = split_string(instructions, ';'); // splitting at ; character
    vector<string> aux;
    double *value;
    for (string s : distinctInstructions)
    {
        // DEBUG_COMMENT(s << "\n");

        if (s.find("'") != string::npos)
        {
            aux = split_string(s, '\'');
            if(fileValues.contains(aux[0])){
                tempVariables[aux[0]] = fileValues[aux[0]];
                continue;
            }

            // DEBUG_COMMENT("aux[0]: " << aux[0] << "\n");

            value = new double;
            // DEBUG_COMMENT("First Visit: " << getFirstVisit() << "\n");
            if (firstVisit)
            {
                ode_solver_values.clear();
                ode_solver_times.clear();
                ode_solver_times.push_back(0.0);
                //  DEBUG_COMMENT("First Visit, new cauchy: " << *sharedVariables[aux[0]] << "\n");
                double *newCauchy = new double;
                *newCauchy = *sharedVariables[aux[0]];
                cauchy[aux[0]] = newCauchy;
                ode_solver_values.push_back(*cauchy[aux[0]]);
            }
            *value = ode_solver(s, *cauchy[aux[0]], time, delta, numSeconds, sharedVariables);
            // DEBUG_COMMENT("New Value X: " << *value << "\n");
            tempVariables[aux[0]] = *(value);
            // sharedVariables[aux[0]] = value;
            delete(value);
            continue;
        }

        // aux[0] = leftoperand -- aux[1] = rightoperand
        aux = split_string(s, '=');
        if(fileValues.contains(aux[0])){
            tempVariables[aux[0]] = fileValues[aux[0]];
            continue;
        }

        // check if the instruction is a simple assignment
        if (aux[1].find("+") == string::npos && aux[1].find("-") == string::npos && aux[1].find("*") == string::npos && aux[1].find("/") == string::npos)
        {
            setlocale(LC_ALL, "C");
/*
#ifdef WINDOWS
            ;
#else
            replace(aux[1].begin(), aux[1].end(), '.', ',');
#endif
*/
            value = new double;
            *value = stod(aux[1]);
            tempVariables[aux[0]] = *(value);
            delete(value);
            // sharedVariables[aux[0]] = value;
        }
        else
        {
            // if the string has variables, then we replace their values
            for (pair<string, double *> pair : sharedVariables)
            {
                aux[1] = replace_var(aux[1], pair.first, to_string(*(pair.second)));

                /*int pos = aux[1].find(pair.first);
                while (pos != string::npos)
                {
                    aux[1].replace(pos, pair.first.length(), to_string(*(pair.second)));
                    pos = aux[1].find(pair.first);
                }*/
            }
            value = new double;
            *value = te_interp(aux[1].c_str(), 0); // solve the instruction
            tempVariables[aux[0]] = *(value);
            delete(value);
            // sharedVariables[aux[0]] = value;       // insert or assign the value
        }
    }
    firstVisit = false;
    // printMap(sharedVariables);
}

/// @brief checks if any transition is satisfied
/// @param sharedVariables the variables of the automata
/// @return Node (the new current node)
string Node::checkTransitions(unordered_map<string, double *> &sharedVariables)
{
    DEBUG_COMMENT("CheckTransitions, name: " << getName() << ", size transitions: " << transitionKeys.size() << "\n");
    for (Transition t : getTransitionKeys())
    {
        if (t.checkCondition(sharedVariables))
            return transitions[t];
    }
    return this->getName();
}

/// @brief override of the '<<' operator
/// @param os the current stream
/// @param obj the object to print
/// @return the new current stream
ostream &operator<<(ostream &os, Node &obj)
{
    os << "Name: " << obj.name << ", description: " << obj.description << ", transitions: \n";
    for (Transition t : obj.getTransitionKeys())
        os << "  " << t << " ----> " << obj.getTransitions()[t] << "\n";
    return os;
}
