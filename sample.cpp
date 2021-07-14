#include "AIController.h"
#include <utility>

extern int ai_side;
std::string ai_name = "your_name_here";

//init function is called once at the beginning
void init() {
	/* Your code here */
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

std::pair<int, std::pair<int, int> > action(std::pair<int, std::pair<int, int> > loc) {
	/* Your code here */
	if (ai_side == 0)
		return std::make_pair(0, std::make_pair(7, 4));
	else
		return std::make_pair(0, std::make_pair(1, 4));
}
