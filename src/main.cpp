/*#include "../include/Objects.h"
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{

    Node q1("q1", "first node");
    Node q2("q2", "second node");
    Node q3("q3", "third node");

    q1.addTransition("x>=5", q2);  // q1 -> q2
    q2.addTransition("x>=10", q3); // q2 -> q3

    vector<Node> nodes = {q1, q2, q3};
    Node initialNode = q1;
    vector<Node> finalNodes = {q3};
    unordered_map<string, double> variables;
    variables["x"] = 1;
    Status status = RUNNING;

    Automata automata(nodes, initialNode, finalNodes, variables, status);
    automata.setCurrentNode(initialNode);
    cout << automata;

    return 0;
}
*/

#include "../include/json.hpp"
#include "../include/Objects.h"
#include <iostream>
#include <fstream>
using namespace std;
using json = nlohmann::json;

int main(int argc, char const *argv[])
{

    std::ifstream f("../settings.json");
    json data = json::parse(f);
    vector<Automata> arrAutomata;
    vector<Node> arrNodes;
    Node startNode;
    vector<Node> finalNodes;
    for (json automata : data["automata"])
    {
        for (json node : automata["node"])
        {
            Node n(node["name"], node["description"]);
            arrNodes.push_back(n);
            if (node["flag"] == "start")
            {
                startNode = n;
            }
            else if (node["flag"] == "final")
            {
                finalNodes.push_back(n);
            }
        }

        unordered_map<string, double> variables;
        for (json variable : automata["variables"])
        {
            string var = variable["value"];
            variables[variable["name"]] = stod(var);
        }

        int i = -1;
        for (Node n : arrNodes) // adding transictions to nodes
        {
            i++;
            for (json node : automata["node"])
            {
                if (node["name"] == n.getName())
                {

                    for (json transition : node["transitions"])
                    {
                        Node to;
                        for (Node n1 : arrNodes)
                        {
                            to = (n1.getName() != transition["to"]) ? to : n1;
                            /*if (n1.getName() == transition["to"])
                            {
                                to = n1;
                            }*/
                        }
                        arrNodes[i].addTransition(transition["condition"], to);
                    }
                }
            }
        }

        Status status = RUNNING;
        arrAutomata.push_back(Automata(arrNodes, startNode, finalNodes, variables, status));
        arrNodes.clear(); // empty for next automata creation
    }

    for (Automata a : arrAutomata)
    {
        cout << a;
    }

    return 0;
}