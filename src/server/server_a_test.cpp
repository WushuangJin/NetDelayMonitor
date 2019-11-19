#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <stdio.h> 
#include <stdlib.h> 
#include <utility>
#include <string>
#include <sstream>
#include <queue> 


using namespace std;
string filePath = "map.txt";
unordered_map <char*, city_info> map;


int process_file(string filePath) {;
    ifstream file;
    file.open(filePath,ios::in);
    if(!file.is_open()) {
        return 0;
    } else {
        int idx;
        char* city_name;
        std::string strLine;
        city_info city;

        while(getline(file,strLine))
        {
            if(strLine.empty())
                continue;
            const char *str_line = strLine.c_str();
            char c = *str_line;
            if((c >= 'A' && c <= 'Z') ||(c >= 'a' && c <= 'z')) {
                printf("find it");
                strcpy(city_name, strLine.c_str());
                idx = 1;
                map[city_name] = city;
            } else if (idx == 1) {
                map[city_name].prop_speed = stoi(strLine);
                idx++;
            } else if (idx == 2) {
                map[city_name].tran_speed = stoi(strLine);
                idx++;
            } else {
                istringstream in(strLine);
                vector<string> temp;
                string t;
                while (in >> t) {
                    temp.push_back(t);
                }
                int from = stoi(temp[0]);
                int to = stoi(temp[1]);
                int dis = stoi(temp[2]);
                printf("edge from %d to %d\n", from, to);
                unordered_map<int, vector<edge> > curr_map = map[city_name].graph;
                if(curr_map.find(from) == curr_map.end()) {
                    vector<edge> edges_to;
                    curr_map.insert(make_pair(from, edges_to));
                    curr_map[from].push_back(make_pair(to, dis));
                }
                if(curr_map.find(to) == curr_map.end()) {
                    vector<edge> edges_from;
                    curr_map.insert(make_pair(to, edges_from));
                    curr_map[to].push_back(make_pair(from,dis));
                } 
                idx++;
            }
            cout<<strLine <<endl;            
        }
    }
    return 0;
}

// int compute(char* city_id, int start) {
//     unordered_map<int, vector<edge> > neighbors = map[city_id].graph;
//     int N = neighbors.size();
//     vector<bool>visited(N+1,false);
//     struct cmp {
//         bool operator()(const std::pair<int,int> &left, const std::pair<int,int> &right) {
//             return left.second < right.second;
//         }
//     };
//     priority_queue<int, vector<edge>, cmp> pq;
//     pq.push(make_pair(start,0));
//     vector<bool> visited(N+1,false);
//     int res = 0;
//     int cnt_visited =0;
// }

int main()
{   
    process_file(filePath);
    printf("the size of map : %lu \n", map.size());
    // receive map info from aws
    // compute distance 
    // char* city_id = "A";
    // int start_idx = 1;
    // int dis = compute(city_id, start_idx);
    return 0;
}

