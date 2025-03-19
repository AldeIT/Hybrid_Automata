#include "../include/Objects.h"
#include <iostream>

#ifdef DEBUG_MODE
#define DEBUG_COMMENT(comment) std::cout << "[DEBUG] " << comment << std::endl;
#else
#define DEBUG_COMMENT(comment)
#endif

/// @brief Constructs a System object with the given parameters.
///
/// Initializes the System with the provided set of automata, dependence map, automata variables map, temporary variables map,
/// time step delta, and total number of seconds to simulate.
///
/// @param automata A vector containing Automata objects representing the system's state.
/// @param AutomataDependence An unordered_map containing the dependence relationships between automata.
/// @param automataVariables An unordered_map containing pointers to double variables associated with automata.
/// @param tempVariables An unordered_map containing temporary double variables used in calculations.
/// @param delta The time step for each simulation iteration.
/// @param numSeconds The total number of seconds to simulate.
System::System(vector<Automata> automata, unordered_map<string, string> &AutomataDependence, unordered_map<string, double *> &automataVariables, unordered_map<string, double> &tempVariables, double delta, double numSeconds)
{
    this->automata = automata;
    this->AutomataDependence = AutomataDependence;
    this->automataVariables = automataVariables;
    this->tempVariables = tempVariables;
    for (int j = 0; j < this->automata.size(); j++)
    {
        this->automata[j].setAutomataVariables(this->automataVariables);
        this->automata[j].setTempVariables(this->tempVariables);
    }
    this->delta = delta;
    this->numSeconds = numSeconds;
}

/// @brief return the Automata of the System
/// @return the Automata of the System
vector<Automata> System::getAutomata()
{
    return this->automata;
}

/// @brief return the Automata Dependence
/// @return the automata Automata Dependence
unordered_map<string, string> System::getAutomataDependence()
{
    return this->AutomataDependence;
}

/// @brief Refreshes the automata variables with the temporary variables.
///
/// Updates the values of the automata variables with the values stored in the temporary variables.
/// After updating, clears the temporary variables map.
void System::refreshVariables()
{

    for (auto const &key : tempVariables)
    {
        *automataVariables[key.first] = key.second;
    }

    tempVariables.clear();
}

/// @brief Retrieves a copy of the current values of the system variables.
///
/// Creates a copy of the current values stored in the system's automata variables
/// and returns it as an unordered map.
///
/// @return An unordered map containing the current values of the system variables.
unordered_map<string, double> System::getVariables()
{
    unordered_map<string, double> temp;
    for (const auto &pair : this->automataVariables)
    {
        temp[pair.first] = *(pair.second);
    }
    return temp;
}

/// @brief to string
ostream &operator<<(ostream &os, System &obj)
{
    os << "\nSystem: \n\n";
    for (Automata s : obj.getAutomata())
    {
        os << s << "\n\n";
    }

    os << "Dependence - Map \n";
    for (auto const &x : obj.getAutomataDependence())
    {
        os << "- " << x.first << " : " << x.second << ";\n";
    }

    return os << "\n\n\n";
}
