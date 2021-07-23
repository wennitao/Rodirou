#include "AIController.h"
#include <bits/stdc++.h>

using namespace std;

extern int ai_side;
std::string ai_name = "your_name_here";

const int INF = 0x3f3f3f3f ;

struct status {
    pair<int, int> pos[2] ;
    int fence_cnt[2] ;
    int fence[8][8] ;

    void print() {
        cerr << "(" << pos[0].first << " " << pos[0].second << ") (" << pos[1].first << " " << pos[1].second << ")" << endl ;
        for (int i = 0; i < 8; i ++) {
            for (int j = 0; j < 8; j ++) 
                cerr << fence[i][j] ;
            cerr << endl ;
        }
    }

    bool end() {
        if (pos[0].first == 0 || pos[1].first == 8) return 1 ;
        return 0 ;
    }
} ;

status cur_status ;

//init function is called once at the beginning
void init() {
	cur_status.pos[0] = make_pair (8, 4) ;
    cur_status.pos[1] = make_pair (0, 4) ;
    cur_status.fence_cnt[0] = cur_status.fence_cnt[1] = 10 ;
}

inline void do_action (status &st, pair<int, pair<int, int> > action, int side) {
    if (action.first == -1) return ;
    if (action.first == 0) {
        st.pos[side] = action.second ;
    } else {
        st.fence_cnt[side] -- ;
        st.fence[action.second.first][action.second.second] = action.first ;
    }
}

struct edge {
    pair<int, pair<int, int> > action ;
    int node_id ;
    edge () {} 
    edge (pair<int, pair<int, int> > _action, int _node_id) : action (_action), node_id (_node_id) {}
} ;
vector<edge> G[1000010] ;
int cnt[1000010], win[1000010], node_cnt = 1 ;

int dx[4] = {0, 0, 1, -1}, dy[4] = {1, -1, 0, 0} ;

inline bool move_valid (const status &st, int x, int y, int d) {
    int xx = x + dx[d], yy = y + dy[d] ;
    if (xx < 0 || xx >= 9 || yy < 0 || yy >= 9) return 0 ;
    if (d == 0 && ((x < 8 && st.fence[x][y] == 1) || (x - 1 >= 0 && st.fence[x - 1][y] == 1))) return 0 ;
    if (d == 1 && ((xx < 8 && st.fence[xx][yy] == 1) || (xx - 1 >= 0 && st.fence[xx - 1][yy] == 1))) return 0 ;
    if (d == 2 && ((y < 8 && st.fence[x][y] == 2) || (y - 1 >= 0 && st.fence[x][y - 1] == 2))) return 0 ;
    if (d == 3 && ((yy < 8 && st.fence[xx][yy] == 2) || (yy - 1 >= 0 && st.fence[xx][yy - 1] == 2))) return 0 ;
    return 1 ;
}

int dis[9][9] ;
inline int bfs (status &st, int sx, int sy, int side) {
    if (sx == side * 8) return 0 ;
    memset (dis, 0x3f, sizeof dis) ;
    dis[sx][sy] = 0 ;
    queue<pair<int, int> > que ;
    que.push (make_pair (sx, sy)) ;
    while (!que.empty()) {
        pair<int, int> pos = que.front(); que.pop() ;
        for (int d = 0; d < 4; d ++) {
            if (!move_valid (st, pos.first, pos.second, d)) continue ;
            int x = pos.first + dx[d], y = pos.second + dy[d] ;
            if (dis[x][y] < 1e9) continue ;
            dis[x][y] = dis[pos.first][pos.second] + 1 ;
            if (x == side * 8) return dis[x][y] ;
            que.push (make_pair (x, y)) ;
        }
    }
    return INF ;
}

inline pair<int, int> calDis (status &st) {
    int d1 = bfs (st, st.pos[0].first, st.pos[0].second, 0) ;
    int d2 = bfs (st, st.pos[1].first, st.pos[1].second, 1) ;
    return make_pair (d1, d2) ;
}

void expand (status &st, int v, int side) {
    std::vector<std::pair<int, int> > nxt_pos ;
    for (int d = 0; d < 4; d ++) {
        int x = st.pos[side].first, y = st.pos[side].second ;
        int xx = x + dx[d], yy = y + dy[d] ;
        if (!move_valid (st, x, y, d)) continue ;
        if (xx == st.pos[1 - side].first && yy == st.pos[1 - side].second) {
            int xxx = xx + dx[d], yyy = yy + dy[d] ;
            if (move_valid (st, xx, yy, d)) {
                nxt_pos.push_back (std::make_pair (xxx, yyy)) ;
            } else {
                int st_d = d < 2 ? 2 : 0 ;
                xxx = xx + dx[st_d], yyy = yy + dy[st_d] ;
                if (move_valid (st, xx, yy, d)) 
                    nxt_pos.push_back (std::make_pair (xxx, yyy)) ;

                st_d ++ ;
                xxx = xx + dx[st_d], yyy = yy + dy[st_d] ;
                if (move_valid (st, xx, yy, d)) 
                    nxt_pos.push_back (std::make_pair (xxx, yyy)) ;
            }
        } else {
            nxt_pos.push_back (std::make_pair (xx, yy)) ;
        }
    }
    for (auto pos : nxt_pos) {
        G[v].push_back (edge (make_pair (0, pos), ++ node_cnt)) ;
    }

    if (st.fence_cnt[side]) {
        for (int i = 0; i < 8; i ++)
            for (int j = 0; j < 8; j ++) {
                if (st.fence[i][j]) continue ;
                if (!((i >= 1 && st.fence[i - 1][j]) || (i < 7 && st.fence[i + 1][j]))) {
                    st.fence[i][j] = 1 ;
                    pair<int, int> dis = calDis (st) ;
                    if (dis.first < 1e9 && dis.second < 1e9)
                        G[v].push_back (edge (make_pair (1, make_pair (i, j)), ++ node_cnt)) ;
                }

                if (!((j >= 1 && st.fence[i][j - 1]) || (j < 7 && st.fence[i][j + 1]))) {
                    st.fence[i][j] = 2 ;
                    pair<int, int> dis = calDis (st) ;
                    if (dis.first < 1e9 && dis.second < 1e9)
                        G[v].push_back (edge (make_pair (2, make_pair (i, j)), ++ node_cnt)) ;
                }
                
                st.fence[i][j] = 0 ;
            }
    }
}

int simulate (status st, int side) {
    bool ban[3][9][9] = {0} ;
    int step = 0 ;
    while (!st.end()) {
        // st.print() ;
        step ++ ;
        std::vector<std::pair<int, int> > nxt_pos ;
        for (int d = 0; d < 4; d ++) {
            int x = st.pos[side].first, y = st.pos[side].second ;
            int xx = x + dx[d], yy = y + dy[d] ;
            if (!move_valid (st, x, y, d)) continue ;
            if (xx == st.pos[1 - side].first && yy == st.pos[1 - side].second) {
                int xxx = xx + dx[d], yyy = yy + dy[d] ;
                if (move_valid (st, xx, yy, d)) {
                    nxt_pos.push_back (std::make_pair (xxx, yyy)) ;
                } else {
                    int st_d = d < 2 ? 2 : 0 ;
                    xxx = xx + dx[st_d], yyy = yy + dy[st_d] ;
                    if (move_valid (st, xx, yy, d)) 
                        nxt_pos.push_back (std::make_pair (xxx, yyy)) ;

                    st_d ++ ;
                    xxx = xx + dx[st_d], yyy = yy + dy[st_d] ;
                    if (move_valid (st, xx, yy, d)) 
                        nxt_pos.push_back (std::make_pair (xxx, yyy)) ;
                }
            } else {
                nxt_pos.push_back (std::make_pair (xx, yy)) ;
            }
        }
        pair<int, int> dest_pos ;
        for (auto pos : nxt_pos) {
            // cerr << pos.first << " " << pos.second << " " << bfs (st, pos.first, pos.second, side) << " " << bfs (st, st.pos[side].first, st.pos[side].second, side) << endl ;
            if (bfs (st, pos.first, pos.second, side) < bfs (st, st.pos[side].first, st.pos[side].second, side)) {
                dest_pos = pos; break ;
            }
        }
        // cerr << side << " (" << st.pos[side].first << " " << st.pos[side].second << ") -> (" << dest_pos.first << " " << dest_pos.second << ")" << endl ;

        vector<pair<int, pair<int, int> > > fence ;
        for (int i = 0; i < 8; i ++)
            for (int j = 0; j < 8; j ++) {
                if (st.fence[i][j]) continue ;
                if (!ban[1][i][j] && !((i >= 1 && st.fence[i - 1][j] == 1) || (i < 7 && st.fence[i + 1][j] == 1)))
                    fence.push_back (make_pair (1, make_pair (i, j))) ;
                if (!ban[2][i][j] && !((j >= 1 && st.fence[i][j - 1] == 2) || (j < 7 && st.fence[i][j + 1] == 2)))
                    fence.push_back (make_pair (2, make_pair (i, j))) ;
            }
        
        int random = rand() % 10 + 1;
        if (random <= 7) {
            // cerr << side << " move to " << "(" << dest_pos.first << " " << dest_pos.second << ")" << endl ;
            do_action (st, make_pair (0, dest_pos), side) ;
        } else {
            if (!fence.empty() && st.fence_cnt[side]) {
                int random_cnt = 0 ;
                while (1) {
                    random = rand() % fence.size() ;
                    random_cnt ++ ;
                    if (random_cnt >= 20) break ;
                    if (ban[fence[random].first][fence[random].second.first][fence[random].second.second]) continue ;
                    do_action (st, fence[random], side) ;
                    pair<int, int> dis = calDis (st) ;
                    if (dis.first < 1e9 && dis.second < 1e9) break ;
                    ban[fence[random].first][fence[random].second.first][fence[random].second.second] = 1 ;
                    st.fence[fence[random].second.first][fence[random].second.second] = 0 ;
                    st.fence_cnt[side] ++ ;
                }
                if (random_cnt >= 20)
                    do_action (st, make_pair (0, dest_pos), side) ;
                else {
                    // cerr << side << " place fence " << fence[random].first << " " << fence[random].second.first << " " << fence[random].second.second << endl ;
                }
            } else {
                // cerr << side << " move to " << "(" << dest_pos.first << " " << dest_pos.second << ")" << endl ;
                do_action (st, make_pair (0, dest_pos), side) ;
            }
        }
        side = 1 - side ;
    }
    // cerr << "step:" << step << endl ;
    if (st.pos[0].first == 0) return 0 ;
    if (st.pos[1].first == 8) return 1 ;
}

int selection (status &st, int v, int depth) {
    int cur_side = ((depth & 1) + ai_side) & 1 ;
    if (G[v].empty()) {
        expand (st, v, cur_side) ;
    }

    double max_value = -1e18 ;
    edge max_edge ;
    for (auto edg : G[v]) {
        if (!cnt[edg.node_id]) {
            max_value = 1e18; max_edge = edg ;
        } else {
            double c = 0.05 ;
            double value = win[edg.node_id] / cnt[edg.node_id] + c * sqrt (log (cnt[v]) / cnt[edg.node_id]) ;
            if (value > max_value) {
                max_value = value; max_edge = edg ;
            }
        }
    }

    // std::cerr << "depth:" << depth << endl ;
    // std::cerr << v << " " << win[v] << "/" << cnt[v] << endl ;
    // std::cerr << max_edge.node_id << " " << win[max_edge.node_id] << "/" << cnt[max_edge.node_id] << std::endl ;

    pair<int, int> pre_pos = st.pos[cur_side] ;
    do_action (st, max_edge.action, cur_side) ;
    if (!cnt[max_edge.node_id]) {
        int res = simulate (st, 1 - cur_side) ;
        if (res == cur_side) win[max_edge.node_id] ++ ;
        cnt[max_edge.node_id] ++ ;

        if (res == 1 - cur_side) win[v] ++ ;
        cnt[v] ++ ;

        if (max_edge.action.first == 0) st.pos[cur_side] = pre_pos ;
        else st.fence[max_edge.action.second.first][max_edge.action.second.second] = 0 ;

        return res ;
    } else {
        int res = selection (st, max_edge.node_id, depth + 1) ;
        if (res == 1 - cur_side) win[v] ++ ;
        cnt[v] ++ ;
        if (max_edge.action.first == 0) st.pos[cur_side] = pre_pos ;
        else st.fence[max_edge.action.second.first][max_edge.action.second.second] = 0 ;
        return res ;
    }
}

std::pair<int, std::pair<int, int> > action(std::pair<int, std::pair<int, int> > loc) {
    clock_t s = clock() ;
    srand (time (0)) ;

	do_action (cur_status, loc, 1 - ai_side) ;

    for (int i = 1; i <= node_cnt; i ++) {
        cnt[i] = win[i] = 0 ;
        G[i].clear() ;
    }
    node_cnt = 1 ;

    // while (1) {
    //     clock_t t = clock() ;
    //     if ((float)(t - s) / CLOCKS_PER_SEC >= 1.9) break ;
    //     status st = cur_status ;
    //     selection (st, 1, 0) ;
    // }

    for (int i = 0; i < 20000; i ++) {
        status st = cur_status ;
        selection (st, 1, 0) ;
    }

    int max_cnt = 0, max_win = 0 ;
    pair<int, pair<int, int> > best_action ;
    for (auto edg : G[1]) {
        if (edg.action.first != 0 && !cur_status.fence_cnt[ai_side]) continue ;
        if (cnt[edg.node_id] > max_cnt || (cnt[edg.node_id] == max_cnt && win[edg.node_id] > max_win)) {
            max_cnt = cnt[edg.node_id]; max_win = win[edg.node_id] ;
            best_action = edg.action ;
        }
    }

    do_action (cur_status, best_action, ai_side) ;
    return best_action ;
}
