#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <exception>
#include <map>

using namespace std;

class graph {
public:
    graph() = default;

    void findPath(const string &fromVertexName, const string &toVertexName) {
        if(this->empty){
            cout << "Graph is empty" << endl;
            return;
        }
        int from = -1, to = -1;
        for (int i = 0; i < this->_vertexNames.size(); i++)
            if (this->_vertexNames.at(i) == fromVertexName)
            from = i;
        else if (this->_vertexNames.at(i) == toVertexName)
            to = i;

        if (from == -1)
            throw invalid_argument("Vertex with name '" + fromVertexName + "' not found");
        else if (to == -1)
            throw invalid_argument("Vertex with name '" + toVertexName + "' not found");
        int min = INT_MAX;
        vector<vector<int>> minPaths(0);
        for (auto &&path:_findPaths(from, to)) {
            int pathWeight = _getPathWeight(path);
            if (pathWeight == min)
                minPaths.push_back(path);
            else if (pathWeight < min) {
                min = pathWeight;
                minPaths.clear();
                minPaths.push_back(path);
            }
        }
        cout << "=============== min paths ================" << endl;
        for (auto &&path: minPaths) {
            _printPath(path);
        }

    }

    bool isDirected() const {
        return directed;
    }

    bool isWeight() const {
        return weight;
    }

    bool isPseudo() const {
        return pseudo;
    }

    bool isEmpty() const {
        return empty;
    }

    const vector<string> &getVertexNames() const {
        return _vertexNames;
    }

    const vector<vector<int>> &getAdjacencyMatrix() const {
        return _adjacencyMatrix;
    }

    friend istream &operator>>(istream &is, graph &graph) {
        try {
            string line;
            getline(is, line);
            stringstream str_stream(line);
            while (str_stream >> line) graph._vertexNames.push_back(line);
            graph._normalizeNames();

            for (int i = 0; i < graph._vertexNames.size(); i++) {
                getline(is, line);
                if (!line.empty()) {
                    stringstream items(line);
                    graph._adjacencyMatrix.emplace_back(0);
                    for (int j = 0; j < graph._vertexNames.size(); j++)
                        if (items >> line)
                            graph._adjacencyMatrix.at(i).push_back(stoi(line));
                        else
                            graph._adjacencyMatrix.at(i).emplace_back(0);
                } else graph._adjacencyMatrix.emplace_back(graph._vertexNames.size(), 0);
            }
            graph._checkDirected();
            graph._checkWeight();
            graph._checkPseudo();
            if (!graph._vertexNames.empty()) graph.empty = false;
        } catch (exception &ex) {
            throw ex;
        }
        return is;
    }

    friend ostream &operator<<(ostream &os, const graph &graph) {
        if(graph.isEmpty()){
            os << "Graph is empty";
            return os;
        }
        os << graph._getVertexNamesString() << endl;
        if (graph.directed) os << "directed ";
        if (graph.pseudo) os << "pseudograph";
        else os << "graph";
        if (graph.weight) os << " with weight";
        os << endl;
        if (graph.directed) {
            for (int i = 0; i < graph._adjacencyMatrix.size(); i++)
                for (int j = 0; j < graph._adjacencyMatrix.size(); j++)
                    if (graph._adjacencyMatrix.at(i).at(j) != 0)
                        os << graph._getEdgeString(i, j, graph.weight, graph.directed) << endl;
        } else {
            for (int i = 0; i < graph._adjacencyMatrix.size(); i++)
                for (int j = i; j < graph._adjacencyMatrix.size(); j++)
                    if (graph._adjacencyMatrix.at(i).at(j) != 0)
                        os << graph._getEdgeString(i, j, graph.weight, graph.directed) << endl;
        }
        return os;
    }

private:
    vector<vector<int>> _findPaths(int from, int to) {
        vector<vector<int>> result(1, vector<int>(1, from));
        return _tracing(result, to);
    }

    void _normalizeNames() {
        map<string, int> duplicate_map;
        for (auto &&name: _vertexNames)
            if (duplicate_map.find(name) == duplicate_map.end())
                duplicate_map.emplace(name, 0);
            else {
                duplicate_map.at(name) += 1;
                name = name.append(to_string(duplicate_map.at(name)));
            }
    }

    static bool vectorContains(const vector<int> &vector, int value) {
        for (auto &&item: vector)
            if (item == value) return true;
        return false;
    }

    vector<vector<int>> _tracing(vector<vector<int>> paths, int to) {
        vector<vector<int>> updated(0);
        bool needNext = false;
        for (auto &path : paths) {
            if (path.at(path.size() - 1) != to)
                for (int j = 0; j < this->_adjacencyMatrix.size(); j++)
                    if (this->_adjacencyMatrix.at(path.at(path.size() - 1)).at(j) != 0
                        && !vectorContains(path, j)) {
                        vector<int> added(path);
                        added.push_back(j);
                        updated.push_back(added);
                        needNext = true;
                    } else continue;
            else updated.push_back(path);
        }
        return needNext ? _tracing(updated, to) : updated;
    }

    void _printPath(vector<int> path) {
        for (int i = 0; i < path.size() - 1; i++)
            cout << this->_getEdgeString(path.at(i), path.at(i + 1), true, true) << endl;
        cout << endl << "path weight: " << _getPathWeight(path) << endl;
        cout << "==========================================" << endl;
    }

    int _getPathWeight(vector<int> path) {
        int result = 0;
        for (int i = 0; i < path.size() - 1; i++)
            result += this->_adjacencyMatrix.at(path.at(i)).at(path.at(i + 1));
        return result;
    }

    string _getVertexNamesString() const {
        string tempNames = "vertex names:[ '";
        for (auto &&vertexName:this->_vertexNames)
            tempNames.append(vertexName).append("', '");
        tempNames = tempNames.substr(0, tempNames.size() - 3);
        tempNames.append(" ]");
        return tempNames;
    }

    string _getEdgeString(int from, int to, bool useWeight, bool useDirected) const {
        return this->_vertexNames.at(from)
               + (useDirected ? " -> " : " <-> ")
               + this->_vertexNames.at(to)
               + (useWeight ? ": " + to_string(this->_adjacencyMatrix.at(from).at(to)) : "");
    }

    void _checkDirected() {
        directed = false;
        for (int i = 0; i < this->_adjacencyMatrix.size(); i++)
            for (int j = 0; j < this->_adjacencyMatrix.at(i).size(); j++)
                if (i != j && this->_adjacencyMatrix.at(i).at(j) != this->_adjacencyMatrix.at(j).at(i))
                    directed = true;
    }

    void _checkWeight() {
        weight = false;
        for (auto &i : this->_adjacencyMatrix)
            for (int j : i)
                if (j < 0 || j > 1)
                    weight = true;
    }

    void _checkPseudo() {
        pseudo = false;
        for (int i = 0; i < this->_adjacencyMatrix.size(); i++)
            if (this->_adjacencyMatrix.at(i).at(i) != 0) pseudo = true;
    }

    vector<vector<int>> _adjacencyMatrix = vector<vector<int>>(0, vector<int>(0));
    vector<string> _vertexNames = vector<string>(0);
    bool empty = true;
    bool directed = false;
    bool weight = false;
    bool pseudo = false;
};

int main(int argc, char *argv[]) {
    auto g = graph();
/*    cout << "input vertex names and adjacency matrix:" << endl;
    while (g.isEmpty()) {
        try { cin >> g; }
        catch (exception &ex) {
            cout << ex.what() << ". Please try again." << endl;
        }
    }*/

    string filename = argc > 0? argv[1] : "data.txt";
    ifstream fin(filename);
    fin >> g;
    cout << g;

    string user_input;
    while ((cout << "Go find path? Please say yes(y)!: " && cin >> user_input)
           && (user_input == "yes" || user_input == "y")) {
        try{
            string from;
            string to;
            cout << "from: ";
            cin >> from;
            cout << "to: ";
            cin >> to;
            cout << endl << "find minimal path..." << endl;
            g.findPath(from, to);
        }
        catch (exception &ex) {
            cout << ex.what() << ".Please try again." << endl;
        }
    }

    return 0;
}
