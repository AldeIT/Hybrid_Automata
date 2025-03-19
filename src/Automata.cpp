#include "../include/Objects.h"
#include <iostream>

#ifdef DEBUG_MODE
#define DEBUG_COMMENT(comment) std::cout << "[DEBUG] " << comment << std::endl;
#else
#define DEBUG_COMMENT(comment)
#endif

/// @brief constructor
/// @param name the name of the automata
/// @param nodes the nodes of the automata
/// @param initialNodes the initial node of the automata
/// @param finalNodes the final nodes of the automata
/// @param automataVariables the variables of the automata
/// @param status the curren status of the automata
/// @param time_inside_node timer that represents the current time inside of a node
Automata::Automata(string name, vector<Node> nodes, Node initialNode, vector<Node> finalNodes, unordered_map<string, double *> &automataVariables, Status status, int time_inside_node)
{
    this->name = name;
    this->nodes = nodes;

    for (int i = 0; i < nodes.size(); i++)
    {
        nodesNames[nodes.at(i).getName()] = this->nodes.at(i);
    }

    this->initialNode = initialNode;
    this->currentNode = initialNode;
    this->finalNodes = finalNodes;
    this->automataVariables = &automataVariables;
    this->currentStatus = status;
    this->time_inside_node = time_inside_node;
}

/// @brief return the nodes of the automata
/// @return the nodes of the automata
vector<Node> Automata::getNodes()
{
    return this->nodes;
}

/// @brief set the automata's nodes
/// @param nodes the nodes to set
void Automata::setNodes(vector<Node> &nodes)
{
    this->nodes = nodes;
}

/// @brief return the initial node of the automata
/// @return the initial node
Node Automata::getInitialNode()
{
    return this->initialNode;
}

/// @brief set the initial node of the automata
/// @param initialNode the node to set
void Automata::setInitialNode(Node &initialNode)
{
    this->initialNode = initialNode;
}

/// @brief return the current node of the automata
/// @return the current node
Node Automata::getCurrentNode()
{
    return this->currentNode;
}

/// @brief set the current node of the automata
/// @param currentNode the node to set
void Automata::setCurrentNode(Node &currentNode)
{
    this->currentNode = currentNode;
}

/// @brief return the final nodes of the automata
/// @return the final nodes
vector<Node> Automata::getFinalNodes()
{
    return this->finalNodes;
}

/// @brief set the final nodes of the automata
/// @param finalNodes the nodes to set
void Automata::setFinalNodes(vector<Node> &finalNodes)
{
    this->finalNodes = finalNodes;
}

/// @brief return the automata variables
/// @return the automata variables
unordered_map<string, double *> *Automata::getAutomataVariables()
{
    return this->automataVariables;
}

/// @brief set the map string->double of the variables
/// @param automataVariables the new map
void Automata::setAutomataVariables(unordered_map<string, double *> &automataVariables)
{
    this->automataVariables = &automataVariables;
}

/// @brief Set the temporary variables for the automaton. This method sets the temporary variables used by the automaton to the provided map.
/// @param tempVariables A reference to an unordered_map<string, double> containing the temporary variables.
void Automata::setTempVariables(unordered_map<string, double> &tempVariables)
{
    this->tempVariables = &tempVariables;
}

/// @brief return the current status
/// @return the current status
Status Automata::getCurrentStatus()
{
    return this->currentStatus;
}

/// @brief set the current status
/// @param status the new status
void Automata::setCurrentStatus(Status status)
{
    this->currentStatus = status;
}

/// @brief return the map string->nodes
/// @return the current map
unordered_map<string, Node> Automata::getNodesNames()
{
    return this->nodesNames;
}

/// @brief set the map of names->nodes
/// @param nodeNames the nnew map
void Automata::setNodesNames(unordered_map<string, Node> &nodeNames)
{
    this->nodesNames = nodeNames;
}

/// @brief set the autonamata's name
/// @param name pf the automata
void Automata::setName(string name)
{
    this->name = name;
}

/// @brief get the autonamata's name
/// @return the automata name
string Automata::getName()
{
    return this->name;
}

/// @brief get the autonamata's name
/// @return the automata name
string Automata::getInstructions()
{
    return getCurrentNode().getActualInstructions();
}

/// @brief check if we are in the conditions to change the current node
/// @return 1 if the current node changed, 0 otherwise
bool Automata::checkForChanges()
{
    currentNode.executeNodeInstructions(*automataVariables, *tempVariables, time_inside_node);
    time_inside_node++;

    string newNode = currentNode.checkTransitions(*automataVariables, *tempVariables);

    if (newNode != currentNode.getName())
    {
        nodesNames[newNode].setFirstVisit(true);
        time_inside_node = 1;
    }

    if (newNode == currentNode.getName())
        return 0;

    setCurrentNode(nodesNames[newNode]);
    return 1;
}

/// @brief Get the name of the current node.
/// @return The name of the current node.
string Automata::getCurrentNodeName()
{
    return this->currentNode.getName();
}

/// @brief to string
ostream &operator<<(ostream &os, Automata &obj)
{
    os << "Automata Name: " << obj.getName() << "\n\n";
    os << "Nodes: \n";
    for (Node n : obj.getNodes())
    {
        os << "- " << n << "\n";
    }
    os << "Initial Node: " << obj.getInitialNode().getName() << "\n";
    os << "Current Node: " << obj.getCurrentNode().getName() << "\n";
    os << "Final Nodes: \n";
    for (Node n : obj.getFinalNodes())
    {
        os << "- " << n.getName() << "\n";
    }
    os << (obj.getFinalNodes().empty() ? "No final node declared\n" : "");
    os << "Status: " << (obj.getCurrentStatus() == 0 ? "OFF" : (obj.getCurrentStatus() == 1 ? "RUNNING" : "PAUSE")) << "\n\n";
    return os;
}
