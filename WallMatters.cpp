#include "AIController.h"
#include <vector>
#include <queue>
#include <utility>
#include <cstring>
#include <fstream>

extern int ai_side;
std::string ai_name = "alpha-beta search";

const int INF = 0x3f3f3f3f ;

// right, left, down, up
int dx[4] = {0, 0, 1, -1}, dy[4] = {1, -1, 0, 0} ;

struct status {
    std::pair<int, int> pos[2] ;
    int fence_cnt[2] ;
    bool wall[9][9][4] ;
    bool wall_vis[8][8] ;
    void print () {
        std::cerr << "(" << pos[0].first << ", " << pos[0].second << ") (" << pos[1].first << ", " << pos[1].second << ")" ;
        for (int i = 0; i < 9; i ++)
            for (int j = 0; j < 9; j ++)
                std::cerr << "(" << i << ", " << j << ") " << wall[i][j][0] << wall[i][j][1] << wall[i][j][2] << wall[i][j][3] << std::endl ;
    }
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

void do_action (std::pair<int, std::pair<int, int> > action, int side) {
    if (action.first == -1) return ;
    if (action.first == 0) {
        cur_status.pos[side] = action.second ;
    } else if (action.first == 1) {
        cur_status.fence_cnt[side] -- ;
        cur_status.wall_vis[action.second.first][action.second.second] = 1 ;
        cur_status.wall[action.second.first][action.second.second][0] = 1 ;
        cur_status.wall[action.second.first + 1][action.second.second][0] = 1 ;
        cur_status.wall[action.second.first][action.second.second + 1][1] = 1 ;
        cur_status.wall[action.second.first + 1][action.second.second + 1][1] = 1 ;
    } else {
        cur_status.fence_cnt[side] -- ;
        cur_status.wall_vis[action.second.first][action.second.second] = 1 ;
        cur_status.wall[action.second.first][action.second.second][2] = 1 ;
        cur_status.wall[action.second.first][action.second.second + 1][2] = 1 ;
        cur_status.wall[action.second.first + 1][action.second.second][3] = 1 ;
        cur_status.wall[action.second.first + 1][action.second.second + 1][3] = 1 ;
    }
}

int dis[2][9][9] ;

void bfs (const status &st, int sx, int sy, int id) {
    dis[id][sx][sy] = 0 ; 
    std::queue<std::pair<int, int> > que ;
    que.push (std::make_pair (sx, sy)) ;
    while (!que.empty()) {
        std::pair<int, int> p = que.front(); que.pop() ;
        int x = p.first, y = p.second ;
        for (int d = 0; d < 4; d ++) {
            int xx = x + dx[d], yy = y + dy[d] ;
            if (xx < 0 || xx >= 9 || yy < 0 || yy >= 9 || st.wall[x][y][d]) continue ;
            if (dis[id][xx][yy] < 1e9) continue ;
            dis[id][xx][yy] = dis[id][x][y] + 1 ;
            que.push (std::make_pair (xx, yy)) ;
        }
    }
}

std::pair<int, int> cal_dis (const status &st) {
    memset (dis, 0x3f, sizeof dis) ;
    bfs (st, st.pos[0].first, st.pos[0].second, 0) ;
    bfs (st, st.pos[1].first, st.pos[1].second, 1) ;
    int res1 = INF, res2 = INF ;
    for (int i = 0; i < 9; i ++) {
        res1 = std::min (res1, dis[0][0][i]) ;
        res2 = std::min (res2, dis[1][8][i]) ;
    }
    return std::make_pair (res1, res2) ;
}

int cal_cost (const status &st) {
    std::pair<int, int> res = cal_dis (st) ;
    if (res.first == 0) return INF ;
    if (res.second == 0) return -INF ;
    return (res.second - st.fence_cnt[1]) - (res.first - st.fence_cnt[0]);
}

void update_fence (status &st, int x, int y, bool c, int flag) {
    if (flag == 1) { // vertical 
        st.wall_vis[x][y] = c ;
        st.wall[x][y][0] = c ;
        st.wall[x + 1][y][0] = c ;
        st.wall[x][y + 1][1] = c ;
        st.wall[x + 1][y + 1][1] = c ;
    } else { // parallel
        st.wall_vis[x][y] = c ;
        st.wall[x][y][2] = c ;
        st.wall[x][y + 1][2] = c ;
        st.wall[x + 1][y][3] = c ;
        st.wall[x + 1][y + 1][3] = c ;
    }
}

int dfs (int depth, status &st, int alpha, int beta, bool side) {
    if (depth == 4 || st.pos[0].first == 0 || st.pos[1].first == 8) {
        int res = cal_cost (st) ;
        // std::cerr << depth << " "; st.print(); std::cerr << " " << res << std::endl ;
        return res ;
    }
    
    // move
    std::vector<std::pair<int, int> > nxt_pos ;
    for (int d = 0; d < 4; d ++) {
        int x = st.pos[side].first, y = st.pos[side].second ;
        int xx = x + dx[d], yy = y + dy[d] ;
        if (xx < 0 || xx >= 9 || yy < 0 || yy >= 9 || st.wall[x][y][d]) continue ;
        if (xx == st.pos[1 - side].first && yy == st.pos[1 - side].second) {
            int xxx = xx + dx[d], yyy = yy + dy[d] ;
            if (xxx >= 0 && xxx < 9 && yyy >= 0 && yyy < 9 && !st.wall[xx][yy][d]) {
                nxt_pos.push_back (std::make_pair (xxx, yyy)) ;
            } else {
                int st_d = d < 2 ? 2 : 0 ;
                xxx = xx + dx[st_d], yyy = yy + dy[st_d] ;
                if (xxx >= 0 && xxx < 9 && yyy >= 0 && yyy < 9 && !st.wall[xx][yy][d]) 
                    nxt_pos.push_back (std::make_pair (xxx, yyy)) ;

                st_d ++ ;
                xxx = xx + dx[st_d], yyy = yy + dy[st_d] ;
                if (xxx >= 0 && xxx < 9 && yyy >= 0 && yyy < 9 && !st.wall[xx][yy][d]) 
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
        if (st.fence_cnt[side]) {
            for (int i = std::max (st.pos[1 - side].first - 3, 0); i <= std::min (st.pos[1 - side].first + 3, 7); i ++)
                for (int j = std::max (st.pos[1 - side].second - 3, 0); j <= std::min (st.pos[1 - side].second + 3, 7); j ++) {
                    // vertical
                    if (!st.wall[i][j][0] && !st.wall[i + 1][j][0] && !st.wall[i][j + 1][1] && !st.wall[i + 1][j + 1][1] && !st.wall_vis[i][j]) {
                        st.fence_cnt[side] -- ;
                        update_fence (st, i, j, 1, 1) ;
                        std::pair<int, int> tmp_res = cal_dis (st) ;
                        if (tmp_res.first > 1e9 || tmp_res.second > 1e9) {
                            st.fence_cnt[side] ++ ;
                            update_fence (st, i, j, 0, 1) ;
                            continue ;
                        }
                        int res = dfs (depth + 1, st, alpha, beta, !side) ;
                        st.fence_cnt[side] ++ ;
                        update_fence (st, i, j, 0, 1) ;
                        alpha = std::max (alpha, res) ;
                        if (depth == 0 && alpha > max_value) {
                            max_value = alpha ;
                            best_action = std::make_pair (1, std::make_pair (i, j)) ;
                        }
                        if (beta <= alpha) {
                            return alpha ;
                        }
                    }

                    // parallel
                    if (!st.wall[i][j][2] && !st.wall[i][j + 1][2] && !st.wall[i + 1][j][3] && !st.wall[i + 1][j + 1][3] && !st.wall_vis[i][j]) {
                        st.fence_cnt[side] -- ;
                        update_fence (st, i, j, 1, 2) ;
                        std::pair<int, int> tmp_res = cal_dis (st) ;
                        if (tmp_res.first > 1e9 || tmp_res.second > 1e9) {
                            st.fence_cnt[side] ++ ;
                            update_fence (st, i, j, 0, 2) ;
                            continue ;
                        }
                        int res = dfs (depth + 1, st, alpha, beta, !side) ;
                        st.fence_cnt[side] ++ ;
                        update_fence (st, i, j, 0, 2) ;
                        alpha = std::max (alpha, res) ;
                        if (depth == 0 && alpha > max_value) {
                            max_value = alpha ;
                            best_action = std::make_pair (2, std::make_pair (i, j)) ;
                        }
                        if (beta <= alpha) {
                            return alpha ;
                        }
                    }
                }
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
            for (int i = std::max (st.pos[1 - side].first - 3, 0); i <= std::min (st.pos[1 - side].first + 3, 7); i ++)
                for (int j = std::max (st.pos[1 - side].second - 3, 0); j <= std::min (st.pos[1 - side].second + 3, 7); j ++) {
                    // vertical
                    if (!st.wall[i][j][0] && !st.wall[i + 1][j][0] && !st.wall[i][j + 1][1] && !st.wall[i + 1][j + 1][1] && !st.wall_vis[i][j]) {
                        st.fence_cnt[side] -- ;
                        update_fence (st, i, j, 1, 1) ;
                        std::pair<int, int> tmp_res = cal_dis (st) ;
                        if (tmp_res.first > 1e9 || tmp_res.second > 1e9) {
                            st.fence_cnt[side] ++ ;
                            update_fence (st, i, j, 0, 1) ;
                            continue ;
                        }
                        int res = dfs (depth + 1, st, alpha, beta, !side) ;
                        st.fence_cnt[side] ++ ;
                        update_fence (st, i, j, 0, 1) ;
                        beta = std::min (beta, res) ;
                        if (depth == 0 && beta < max_value) {
                            max_value = beta ;
                            best_action = std::make_pair (1, std::make_pair (i, j)) ;
                        }
                        if (beta <= alpha) {
                            return beta ;
                        }
                    }

                    // parallel
                    if (!st.wall[i][j][2] && !st.wall[i][j + 1][2] && !st.wall[i + 1][j][3] && !st.wall[i + 1][j + 1][3] && !st.wall_vis[i][j]) {
                        st.fence_cnt[side] -- ;
                        update_fence (st, i, j, 1, 2) ;
                        std::pair<int, int> tmp_res = cal_dis (st) ;
                        if (tmp_res.first > 1e9 || tmp_res.second > 1e9) {
                            st.fence_cnt[side] ++ ;
                            update_fence (st, i, j, 0, 2) ;
                            continue ;
                        }
                        int res = dfs (depth + 1, st, alpha, beta, !side) ;
                        st.fence_cnt[side] ++ ;
                        update_fence (st, i, j, 0, 2) ;
                        beta = std::min (beta, res) ;
                        if (depth == 0 && beta < max_value) {
                            max_value = beta ;
                            best_action = std::make_pair (2, std::make_pair (i, j)) ;
                        }
                        if (beta <= alpha) {
                            return beta ;
                        }
                    }
                }
        }
        return beta ;
    }
}

std::pair<int, std::pair<int, int> > action(std::pair<int, std::pair<int, int> > loc) {
    do_action (loc, 1 - ai_side) ;

    status st = cur_status ;

    // special judge
    std::vector<std::pair<int, int> > nxt_pos ;
    for (int d = 0; d < 4; d ++) {
        int x = st.pos[ai_side].first, y = st.pos[ai_side].second ;
        int xx = x + dx[d], yy = y + dy[d] ;
        if (xx < 0 || xx >= 9 || yy < 0 || yy >= 9 || st.wall[x][y][d]) continue ;
        if (xx == st.pos[1 - ai_side].first && yy == st.pos[1 - ai_side].second) {
            int xxx = xx + dx[d], yyy = yy + dy[d] ;
            if (xxx >= 0 && xxx < 9 && yyy >= 0 && yyy < 9 && !st.wall[xx][yy][d]) {
                nxt_pos.push_back (std::make_pair (xxx, yyy)) ;
            } else {
                int st_d = d < 2 ? 2 : 0 ;
                xxx = xx + dx[st_d], yyy = yy + dy[st_d] ;
                if (xxx >= 0 && xxx < 9 && yyy >= 0 && yyy < 9 && !st.wall[xx][yy][d]) 
                    nxt_pos.push_back (std::make_pair (xxx, yyy)) ;

                st_d ++ ;
                xxx = xx + dx[st_d], yyy = yy + dy[st_d] ;
                if (xxx >= 0 && xxx < 9 && yyy >= 0 && yyy < 9 && !st.wall[xx][yy][d]) 
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
            do_action (best_action, ai_side) ;
            return best_action ;
        }
    }
    dfs (0, st, -2e9, 2e9, ai_side) ;
    do_action (best_action, ai_side) ;
    max_value = ai_side ? 2e9 : -2e9 ;
    return best_action ;
}