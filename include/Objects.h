#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
using namespace std;

/// @brief class that represents a Transition between two nodes
class Transition
{
private:
    string condition; /*represents the condition to check*/

public:
    Transition(string condition);
    string getCondition();
    void setCondition(string condition);
    bool evaluateSingleEquation(string &expression);
    char evaluateLogicalEquation(string str);
    bool solve(string str);
    bool checkCondition(unordered_map<string, double *> &variables, unordered_map<string, double> &tempVariables);
    friend ostream &operator<<(ostream &os, Transition &obj);
};

/// @brief hash function for Transitions
struct TransitionHash
{
    std::size_t operator()(const Transition &t) const
    {
        std::hash<std::string> hasher;
        Transition tCopy = t;
        return hasher(tCopy.getCondition());
    }
};

/// @brief equal function for Transitions
struct TransitionEqual
{
    bool operator()(const Transition &t1, const Transition &t2) const
    {
        Transition t1Copy = t1;
        Transition t2Copy = t2;
        return t1Copy.getCondition() == t2Copy.getCondition();
    }
};

/// @brief class that represents a single Node
class Node
{
private:
    string name;                                                                    /*the name of the node*/
    string description;                                                             /*a description of the node*/
    vector<Transition> transitionKeys;                                              /*an arraylist for all the transition of its edges, very useful*/
    unordered_map<Transition, string, TransitionHash, TransitionEqual> transitions; /*an hashmap that contains couples that represent each edge (condition, destination) */
    unordered_map<string, double *> cauchy;                                         /*an hashmap that stores initial values for differential equations*/
    bool firstVisit;                                                                /*true if we just arrived into a node*/
    string instructions;    /*all the instructions inside the node*/
    unordered_map<string, vector<double>> map_ode_solver_values;
    vector<double> ode_solver_values;
    vector<double> ode_solver_times;
    double delta;
    double numSeconds;
    unordered_map<string, double> fileValues;

public:
    Node();
    Node(string name, string description, string instructions, bool firstVisit, double delta, double numSeconds);
    string getName();
    void setName(string name);
    string getDescription();
    void setDescription(string description);
    void setFileValues(unordered_map<string, double> newValues);
    unordered_map<Transition, string, TransitionHash, TransitionEqual> getTransitions();
    vector<Transition> getTransitionKeys();
    void setTransitions(unordered_map<Transition, string, TransitionHash, TransitionEqual> &transitions);
    void addTransition(string condition, string destination);
    string checkTransitions(unordered_map<string, double *> &sharedVariables, unordered_map<string, double> &tempVariables);
    double ode_solver(string, double, int, double, double, unordered_map<string, double *> &sharedVariables);
    void executeNodeInstructions(unordered_map<string, double *> &sharedVariables, unordered_map<string, double> &tempVariables, int time);
    bool operator==(const Node &other);
    friend ostream &operator<<(ostream &os, Node &obj);
    void setFirstVisit(bool value);
    bool getFirstVisit();
    string getActualInstructions();
};

enum Status
{
    OFF,
    RUNNING,
    PAUSE
};

/// @brief class that represents one Automata
class Automata
{
private:
    string name;                                        /*the name of the Automata*/
    vector<Node> nodes;                                 /*all the nodes*/
    Node initialNode;                                   /*the initial node*/

    vector<Node> finalNodes;                            /*the final nodes*/
    unordered_map<string, double *> *automataVariables; /*hashmap that contains all the automata variables (variableName, 1.0)*/
    unordered_map<string, double> *tempVariables;
    Status currentStatus;                   /*current status (OFF, RUNNING, PAUSE)*/
    unordered_map<string, Node> nodesNames; /*hashmap that connects nodes' names to the actual nodes*/
    int time_inside_node;                   /*current node life timer*/

public:
    Node currentNode;                                   /*the current node*/
    Automata(string name, vector<Node> nodes, Node initialNode, vector<Node> finalNodes, unordered_map<string, double *> &automataVariables, Status status, int time_inside_node);
    vector<Node> getNodes();
    void setNodes(vector<Node> &nodes);
    Node getInitialNode();
    void setInitialNode(Node &initialNode);
    Node getCurrentNode();
    void setCurrentNode(Node &currentNode);
    vector<Node> getFinalNodes();
    void setFinalNodes(vector<Node> &finalNodes);
    unordered_map<string, double *> *getAutomataVariables();
    void setAutomataVariables(unordered_map<string, double *> &automataVaribles);
    void setTempVariables(unordered_map<string, double> &tempVariables);
    Status getCurrentStatus();
    void setCurrentStatus(Status status);
    unordered_map<string, Node> getNodesNames();
    void setNodesNames(unordered_map<string, Node> &nodesNames);
    bool checkForChanges();
    void setName(string name);
    string getName();
    friend ostream &operator<<(ostream &os, Automata &obj);
    string getInstructions();
    string getCurrentNodeName();
};

/// @brief class that represents one System
class System
{
private:
    vector<Automata> automata;                        /*all the Automata*/
    unordered_map<string, string> AutomataDependence; /*hashmap that contains all automata-variables dependence*/
    unordered_map<string, double *> automataVariables;
    unordered_map<string, double> tempVariables;

public:
    double delta;
    double numSeconds;
    System(vector<Automata> automata, unordered_map<string, string> &AutomataDependence, unordered_map<string, double *> &automataVariables, unordered_map<string, double> &tempVariables, double delta, double numSeconds);
    vector<Automata> getAutomata();
    unordered_map<string, string> getAutomataDependence();
    unordered_map<string, double> getVariables();
    void refreshVariables();
    friend ostream &operator<<(ostream &os, System &obj);
};
