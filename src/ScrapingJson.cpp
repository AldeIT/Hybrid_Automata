#include "../include/UtilsJson.h"
#include "../include/json.hpp"
#include "../include/tinyexpr.h"
#include "../include/tools.h"
#include <string>
#include <iostream>

#ifdef DEBUG_MODE
#define DEBUG_COMMENT(comment) std::cout << "[DEBUG] " << comment << std::endl;
#else
#define DEBUG_COMMENT(comment)
#endif

using namespace std;
using json = nlohmann::json;

/// @brief Scrapes JSON data and constructs a System object based on the parsed data.
/// @param c The path to the JSON file.
/// @return A System object constructed from the parsed JSON data.
System UtilsJson::ScrapingJson(string c)
{
    std::ifstream f(c);
    json data = json::parse(f);
    vector<Automata> arrAutomata;
    unordered_map<string, string> automataDependence;
    vector<Node> arrNodes;
    Node startNode;
    vector<Node> finalNodes;
    unordered_map<string, double *> variables;
    int j = 0;
    int store = 0;
    setlocale(LC_ALL, "C");
    // set global variables
    string h_string = data["system"]["global"]["delta"];
    double system_delta = stod(h_string);
    string tfinal_string = data["system"]["global"]["finaltime"];

    double system_numSeconds = stod(tfinal_string);

    // find all the automata in settings.json
    for (json automata : data["system"]["automata"])
    {
        j = 0;
        store = 0;

        // find all the nodes for each automata
        for (json node : automata["node"])
        {
            Node n(node["name"], node["description"], node["instructions"], (node["flag"] == "start") ? true : false, system_delta, system_numSeconds);
            arrNodes.push_back(n);
            if (node["flag"] == "start")
            {
                startNode = n;
                store = j;
            }
            else if (node["flag"] == "final")
            {
                finalNodes.push_back(n);
            }
            j++;

            string tmp = node["instructions"];
            tmp.erase(std::remove(tmp.begin(), tmp.end(), ' '), tmp.end());
            tmp.erase(std::remove(tmp.begin(), tmp.end(), '\n'), tmp.end());
            vector<string> distinctInstructions = split_string(tmp, ';'); // splitting at ; character
            vector<string> aux;
            for (string s : distinctInstructions) // loop single istructions
            {
                aux = split_string(s, '=');
                if (aux[0].find('\'') == string::npos)
                {
                    automataDependence[aux[0]] = automata["name"];
                }
            }
        }

        // find all the variables for each automata
        double *y;
        for (json variable : automata["variables"])
        {
            string var = variable["value"];
            y = new double(te_interp(var.c_str(), 0));
            variables[variable["name"]] = y;
            automataDependence[variable["name"]] = automata["name"];
        }

        // adding transictions to nodes
        int i = -1;
        for (Node n : arrNodes)
        {
            i++;
            for (json node : automata["node"])
            {
                if (node["name"] == n.getName())
                {

                    for (json transition : node["transitions"])
                    {
                        string to;
                        for (Node n1 : arrNodes)
                        {
                            to = (n1.getName() != transition["to"]) ? to : n1.getName();
                        }
                        arrNodes[i].addTransition(transition["condition"], to);
                    }
                }
            }
        }

        Status status = RUNNING;
        Automata c(automata["name"], arrNodes, arrNodes[store], finalNodes, variables, status, 1);
        arrAutomata.push_back(c);
        // empty for next automata creation
        arrNodes.clear();
    }
    unordered_map<string, double> tempVariables;
    return System(arrAutomata, automataDependence, variables, tempVariables, system_delta, system_numSeconds);
}
