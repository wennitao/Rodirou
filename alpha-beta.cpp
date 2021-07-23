#include "AIController.h"
#include <bits/stdc++.h>

using namespace std;

extern int ai_side;
std::string ai_name = "alpha-beta search";

const int INF = 0x3f3f3f3f ;

// right, left, down, up
int dx[4] = {0, 0, 1, -1}, dy[4] = {1, -1, 0, 0} ;

struct status {
    pair<int, int> pos[2] ;
    int fence_cnt[2] ;
    int fence[8][8] ;
} ;

status cur_status ;
int max_value = -INF ;
std::pair<int, std::pair<int, int> > best_action ;

void init() {
    cur_status.pos[0] = std::make_pair (8, 4) ;
    cur_status.pos[1] = std::make_pair (0, 4) ;

    cur_status.fence_cnt[0] = cur_status.fence_cnt[1] = 10 ;

    max_value = ai_side ? INF : -INF ;
}

/* The following notation is based on player 0's perspective
 * Rows are labeled 0 through 8 from player 1's side to player 0's side
 * Columns are labeled 0 through 8 from player 0's left to right
 * A coordinate is recorded as the row followed by the column, for example, player 0's pawn starts on (8, 4)
 * A pawn move is recorded as the new coordinate occupied by the pawn
 * A fence placement is recorded as the coordinate of the square whose bottom-right corner fits the center of the wall
 * A typecode is defined as follows: 0: pawn move
 *                                   1: vertical fence placement
 *                                   2: parallel fence placement
 * An action is defined as (typecode, (row-coordinate, column-coordinate))
 * You need to analyze your opponent's action and return your own action
 * If the typecode of your opponent's action is '-1', it means that you are on the offensive.
 */


inline void do_action (status &st, pair<int, pair<int, int> > action, int side) {
    if (action.first == -1) return ;
    if (action.first == 0) {
        st.pos[side] = action.second ;
    } else {
        st.fence_cnt[side] -- ;
        st.fence[action.second.first][action.second.second] = action.first ;
    }
}

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

int cal_cost (status &st) {
    std::pair<int, int> res = calDis (st) ;
    if (res.first == 0) return INF ;
    if (res.second == 0) return -INF ;
    return (res.second - st.fence_cnt[1]) - (res.first - st.fence_cnt[0]);
}

clock_t s ;

int dfs (int depth, status &st, int alpha, int beta, bool side) {
    if (depth == 4 || st.pos[0].first == 0 || st.pos[1].first == 8) {
        int res = cal_cost (st) ;
        // std::cerr << depth << " "; st.print(); std::cerr << " " << res << std::endl ;
        return res ;
    }

    if (((float)clock() - s) / CLOCKS_PER_SEC >= 1.7) {
        if (side == 0) return alpha ;
        else return beta ;
    }
    
    // move
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
                if (move_valid (st, x, y, d)) 
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
    if (side == 0) {
        // move
        for (int i = 0; i < nxt_pos.size(); i ++) {
            std::pair<int, int> pos = nxt_pos[i] ;
            std::pair<int, int> pre_pos = st.pos[side] ;
            st.pos[side] = pos ;
            int res = dfs (depth + 1, st, alpha, beta, !side) ;
            st.pos[side] = pre_pos ;
            alpha = std::max (alpha, res) ;
            if (depth == 0 && alpha > max_value) {
                max_value = alpha ;
                best_action = std::make_pair (0, pos) ;
            }
            if (beta <= alpha) {
                return alpha ;
            }
        }
        // place fence
        if (st.fence_cnt[side] > 0) {
            
        }
        return alpha ;
    } else {
        for (int i = 0; i < nxt_pos.size(); i ++) {
            std::pair<int, int> pos = nxt_pos[i] ;
            std::pair<int, int> pre_pos = st.pos[side] ;
            st.pos[side] = pos ;
            int res = dfs (depth + 1, st, alpha, beta, !side) ;
            st.pos[side] = pre_pos ;
            beta = std::min (beta, res) ;
            if (depth == 0 && beta < max_value) {
                max_value = beta ;
                best_action = std::make_pair (0, pos) ;
            }
            if (beta <= alpha) {
                return beta ;
            }
        }
        if (st.fence_cnt[side] > 0) {
            // place fence
            
        }
        return beta ;
    }
}

std::pair<int, std::pair<int, int> > action(std::pair<int, std::pair<int, int> > loc) {
    s = clock() ;

    do_action (cur_status, loc, 1 - ai_side) ;

    status st = cur_status ;

    // special judge
    std::vector<std::pair<int, int> > nxt_pos ;
    for (int d = 0; d < 4; d ++) {
        int x = st.pos[ai_side].first, y = st.pos[ai_side].second ;
        int xx = x + dx[d], yy = y + dy[d] ;
        if (!move_valid (st, x, y, d)) continue ;
        if (xx == st.pos[1 - ai_side].first && yy == st.pos[1 - ai_side].second) {
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
    for (int i = 0; i < nxt_pos.size(); i ++) {
        std::pair<int, int> cur = nxt_pos[i] ;
        if (cur.first == ai_side * 8) {
            best_action = std::make_pair (0, cur) ;
            do_action (cur_status, best_action, ai_side) ;
            return best_action ;
        }
    }
    
    dfs (0, st, -INF, INF, ai_side) ;
    do_action (cur_status, best_action, ai_side) ;
    max_value = ai_side ? 2e9 : -2e9 ;
    return best_action ;
}