import subprocess
import numpy as np
import sys

logfile = open('battle.log', 'w')

def win(id):
    if id == 2:
        sys.stderr.write('draw\n')
    else:
        sys.stdout.write(str(id) + '\n')
        sys.stderr.write('ai' + str(id) + ' wins!\n')
    sys.exit(0)

class AI:

    def __init__(self, path, id):
        self.path = path
        if path == 'human':
            self.human = 1
        else:
            self.human = 0
        self.id = id

    def send(self, message):
        value = str(message) + '\n'
        value = bytes(value, 'UTF-8')
        self.proc.stdin.write(value)
        self.proc.stdin.flush()

    def receive(self):
        return self.proc.stdout.readline().strip().decode()

    def init(self):
        if self.human == 0:
            print(self.path)
            self.proc = subprocess.Popen(self.path,
                                         shell = True,
                                         stdin = subprocess.PIPE,
                                         stdout = subprocess.PIPE)
            print("ok")
            self.send(self.id)
            self.name = self.receive()
            print(self.name)
        else:
            pass

    def action(self, a, b, c):
        if self.human == 1:
            value = sys.stdin.readline().strip().split(' ')
        else:
            self.send(str(a) + ' ' + str(b) + ' ' + str(c))
            value = self.receive().split(' ')
        return int(value[0]), int(value[1]), int(value[2])

class Board:

    def __init__(self):
        self.fence = np.zeros((8, 8), dtype = int)
        self.reachable = np.zeros((9, 9), dtype = int)
        self.move = [[0, -1], [0, 1], [-1, 0], [1, 0]]
        self.pawn = [[8, 4], [0, 4]]
        self.fenceLeft = [10, 10]

    def dis(self, a, b, c, d):
        return abs(a - c) + abs(b - d)

    def fenceCheck(self, a, b, c, d):
        if a == c:
            x = min(b, d)
            if a != 0 and self.fence[a - 1][x] == 1 or a != 8 and self.fence[a][x] == 1:
                return False
        else:
            x = min(a, c)
            if b != 0 and self.fence[x][b - 1] == 2 or b != 8 and self.fence[x][b] == 2:
                return False
        return True

    def dfs(self, x, y):
        if self.reachable[x][y] == 1:
            return
        self.reachable[x][y] = 1
        for k in range(4):
            if 0 <= x + self.move[k][0] <= 8 and 0 <= y + self.move[k][1] <= 8 and self.fenceCheck(x, y, x + self.move[k][0], y + self.move[k][1]):
                self.dfs(x + self.move[k][0], y + self.move[k][1])

    def reachCheck(self, x, y):
        for i in range(9):
            for j in range(9):
                self.reachable[i][j] = 0
        self.dfs(x, y)

    def actionCheck(self, id, a, b, c):
        if a == 0:
            if b < 0 or b > 8 or c < 0 or c > 8:
                sys.stderr.write('Illegal input with typecode 0: out of index [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                return False
            if self.dis(self.pawn[id][0], self.pawn[id][1], b, c) == 1:
                if not self.fenceCheck(self.pawn[id][0], self.pawn[id][1], b, c):
                    sys.stderr.write('Illegal input with typecode 0: block by fence [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                    return False
                if [b, c] == self.pawn[id ^ 1]:
                    sys.stderr.write('Illegal input with typecode 0: block by other pawn [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                    return False
            elif self.dis(self.pawn[id][0], self.pawn[id][1], b, c) == 2:
                if self.dis(self.pawn[id][0], self.pawn[id][1], self.pawn[id ^ 1][0], self.pawn[id ^ 1][1]) != 1 or \
                   self.dis(self.pawn[id ^ 1][0], self.pawn[id ^ 1][1], b, c) != 1:
                    sys.stderr.write('Illegal input with typecode 0: tiao ni ma ne [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                    return False
                if not self.fenceCheck(self.pawn[id][0], self.pawn[id][1], self.pawn[id ^ 1][0], self.pawn[id ^ 1][1]) or \
                   not self.fenceCheck(self.pawn[id ^ 1][0], self.pawn[id ^ 1][1], b, c):
                    sys.stderr.write('Illegal input with typecode 0: block by fence [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                    return False
                if abs(self.pawn[id][0] - b) == 1 and abs(self.pawn[id][1] - c) == 1:
                    tb = self.pawn[id][0] + 2 * (self.pawn[id ^ 1][0] - self.pawn[id][0])
                    tc = self.pawn[id][1] + 2 * (self.pawn[id ^ 1][1] - self.pawn[id][1])
                    if tb < 0 or tb > 8 or tc < 0 or tc > 8:
                        return True
                    if self.fenceCheck(self.pawn[id ^ 1][0], self.pawn[id ^ 1][1], tb, tc):
                        sys.stderr.write('Illegal input with typecode 0: tiao ni ma ne [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                        return False
            else:
                sys.stderr.write('Illegal input with typecode 0: distance not equal 1 [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                return False
            return True
        elif a == 1 or a == 2:
            if b < 0 or b > 7 or c < 0 or c > 7:
                sys.stderr.write('Illegal input with typecode ' + str(a) + ': out of index [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                return False
            if self.fenceLeft[id] <= 0:
                sys.stderr.write('Illegal input with typecode ' + str(a) + ': run out of your fence [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                return False
            self.fenceLeft[id] -= 1
            if self.fence[b, c] != 0:
                sys.stderr.write('Illegal input with typecode ' + str(a) + ': block by another fence [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                return False
            if a == 1:
                if b != 7 and self.fence[b + 1][c] == 1 or b != 0 and self.fence[b - 1][c] == 1:
                    sys.stderr.write('Illegal input with typecode ' + str(a) + ': block by another fence [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                    return False
            else:
                if c != 7 and self.fence[b][c + 1] == 2 or c != 0 and self.fence[b][c - 1] == 2:
                    sys.stderr.write('Illegal input with typecode ' + str(a) + ': block by another fence [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
                    return False
            self.fence[b, c] = a

            self.reachCheck(self.pawn[0][0], self.pawn[0][1])
            flag0 = False
            for i in range(9):
                if self.reachable[0][i]:
                    flag0 = True
                    break
            self.reachCheck(self.pawn[1][0], self.pawn[1][1])
            flag1 = False
            for i in range(9):
                if self.reachable[8][i]:
                    flag1 = True
                    break

            self.fence[b, c] = 0
            if not (flag0 and flag1):
                sys.stderr.write('Illegal input with typecode 2: reachable error [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
            return flag0 and flag1
        else:
            sys.stderr.write('Illegal input: typecode error [' + str(a) + ' ' + str(b) + '  ' + str(c) + ']\n')
            return False

    def action(self, id, a, b, c):
        if a == 0:
            self.pawn[id] = [b, c]
            if id == 0 and b == 0:
                return win(0)
            if id == 1 and b == 8:
                return win(1)
        else:
            self.fence[b, c] = a

    def show(self):
        mp = np.empty((35, 51), dtype = int)
        for i in range(35):
            for j in range(51):
                mp[i][j] = ord(' ')
        for i in range(9):
            for j in range(9):
                mp[i * 4][j * 6] = ord('-')
                mp[i * 4][j * 6 + 1] = ord('-')
                mp[i * 4][j * 6 + 2] = ord('-')
                mp[i * 4 + 1][j * 6] = ord('|')
                mp[i * 4 + 1][j * 6 + 1] = ord(' ')
                mp[i * 4 + 1][j * 6 + 2] = ord('|')
                mp[i * 4 + 2][j * 6] = ord('-')
                mp[i * 4 + 2][j * 6 + 1] = ord('-')
                mp[i * 4 + 2][j * 6 + 2] = ord('-')

        for i in range(8):
            for j in range(8):
                if self.fence[i][j] == 1:
                    mp[i * 4][j * 6 + 4] = ord('*')
                    mp[i * 4 + 1][j * 6 + 4] = ord('*')
                    mp[i * 4 + 2][j * 6 + 4] = ord('*')
                    mp[i * 4 + 3][j * 6 + 4] = ord('*')
                    mp[i * 4 + 4][j * 6 + 4] = ord('*')
                    mp[i * 4 + 5][j * 6 + 4] = ord('*')
                    mp[i * 4 + 6][j * 6 + 4] = ord('*')
                elif self.fence[i][j] == 2:
                    mp[i * 4 + 3][j * 6] = ord('*')
                    mp[i * 4 + 3][j * 6 + 1] = ord('*')
                    mp[i * 4 + 3][j * 6 + 2] = ord('*')
                    mp[i * 4 + 3][j * 6 + 3] = ord('*')
                    mp[i * 4 + 3][j * 6 + 4] = ord('*')
                    mp[i * 4 + 3][j * 6 + 5] = ord('*')
                    mp[i * 4 + 3][j * 6 + 6] = ord('*')
                    mp[i * 4 + 3][j * 6 + 7] = ord('*')
                    mp[i * 4 + 3][j * 6 + 8] = ord('*')

        mp[self.pawn[0][0] * 4 + 1][self.pawn[0][1] * 6 + 1] = ord('0')
        mp[self.pawn[1][0] * 4 + 1][self.pawn[1][1] * 6 + 1] = ord('1')

        sys.stderr.write('   ')
        for i in range(51):
            if i % 6 == 1:
                sys.stderr.write(str(i // 6))
            else:
                sys.stderr.write(' ')
        sys.stderr.write('\n')

        for i in range(35):
            if i % 4 == 1:
                sys.stderr.write(' ' + str(i // 4) + ' ')
            else:
                sys.stderr.write('   ')
            for j in range(51):
                sys.stderr.write(chr(mp[i][j]))
            sys.stderr.write('\n')


def judge():
    board = Board()
    ai0 = AI(sys.argv[1], 0)
    try:
        ai0.init()
    except:
        sys.stderr.write('Time out: ai0 timeout in init function\n')
        win(1)
    ai1 = AI(sys.argv[2], 1)
    try:
        ai1.init()
    except:
        sys.stderr.write('Time out: ai1 timeout in init function\n')
        win(0)

    for turns in range(1, 101):
        board.show()
        if turns == 1:
            a, b, c = ai0.action(-1, 0, 0)
        else:
            a, b, c = ai0.action(a, b, c)
        if not board.actionCheck(0, a, b, c):
            win(1)
        logfile.write(str(a) + ' ' + str(b) + ' ' + str(c) + ' ')
        sys.stderr.write('ai0 take action: [' + str(a) + ' ' + str(b) + ' ' + str(c) + ']\n')
        board.action(0, a, b, c)
        board.show()
        a, b, c = ai1.action(a, b, c)
        if not board.actionCheck(1, a, b, c):
            win(0)
        logfile.write(str(a) + ' ' + str(b) + ' ' + str(c) + ' ')
        sys.stderr.write('ai1 take action: [' + str(a) + ' ' + str(b) + ' ' + str(c) + ']\n')
        board.action(1, a, b, c)
    win(2)

if (not len(sys.argv) == 3):
    sys.stderr.write('usage:   python3 judge.py ai0Path ai1Path\n')
    sys.stderr.write('         python3 judge.py human ai1Path\n')
    sys.stderr.write('example: python3 judge.py ./sample_ai ./sample_ai\n')
    sys.stderr.write('         python3 judge.py human ./sample_ai\n')
    sys.exit(1)
judge()
