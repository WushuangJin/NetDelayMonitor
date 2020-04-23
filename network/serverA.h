#include <unordered_map>
#include <vector>

using namespace std;
struct city_info{
    char map_id;
    int trans_speed;
    float prop_speed;
    bool exist;
    int num_edges; // single direction
    unordered_map<int, vector<pair<int,float>>> graph;
};

struct header {
    char map_id;
    bool exist;
};

struct graph_info {
    int trans_speed;
    float prop_speed;
    int num_edges;
};

struct edge {
    int start;
    int end;
    float dis;
};