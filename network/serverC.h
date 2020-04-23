// struct send_info_C {
//     int start_idx;
//     int end_idx;
//     int file_size;
    
//     char map_id;
//     float prop_speed;
//     int trans_speed;
//     unordered_map<int, vector<pair<int,float>>> graph;
    
// };

#include <vector>
using namespace std;
struct msg_c {
    bool map_existed;
    bool start_existed;
    bool end_existed;
    float res_dis;
    float tp;
    float tt;
    float total;
    int len;
    int route[100];
};
