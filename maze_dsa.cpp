// Revised MazeSolver.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <queue>
#include <string>

using Position = std::pair<int,int>;

class Maze {
public:
    Maze() : rows(0), cols(0) {}
    bool load(const std::string& filename);
    bool save(const std::string& filename) const;
    void markPath(const std::vector<Position>& path, char mark = '*');
    Position getStart() const { return start; }
    Position getEnd()   const { return end; }
    bool isOpen(int r, int c) const {
        return r>=0 && r<rows && c>=0 && c<cols && grid[r][c] != '#';
    }
    void print(std::ostream& os = std::cout) const {
        for (auto& line : grid) os << line << "\n";
    }
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    const std::vector<std::string>& getGrid() const { return grid; }

private:
    std::vector<std::string> grid;
    int rows, cols;
    Position start, end;
};

bool Maze::load(const std::string& filename) {
    std::ifstream in(filename);
    if (!in) return false;
    grid.clear();
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        grid.push_back(line);
    }
    rows = grid.size();
    cols = rows ? grid[0].size() : 0;
    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j) {
            if (grid[i][j] == 'S') start = {i,j};
            if (grid[i][j] == 'E') end   = {i,j};
        }
    return true;
}

bool Maze::save(const std::string& filename) const {
    std::ofstream out(filename);
    if (!out) return false;
    for (auto& line : grid) out << line << "\n";
    return true;
}

void Maze::markPath(const std::vector<Position>& path, char mark) {
    for (auto& p : path) {
        auto& c = grid[p.first][p.second];
        if (c == ' ' || c == '.') c = mark;
    }
}

// Recursive DFS
bool dfsUtil(const Maze& maze, Position cur, Position end,
             std::vector<std::vector<bool>>& visited,
             std::vector<Position>& path)
{
    if (cur == end) {
        path.push_back(cur);
        return true;
    }
    int r = cur.first, c = cur.second;
    static const int dr[4] = {-1,1,0,0}, dc[4] = {0,0,-1,1};

    visited[r][c] = true;
    for (int i = 0; i < 4; ++i) {
        int nr = r + dr[i], nc = c + dc[i];
        if (maze.isOpen(nr,nc) && !visited[nr][nc]) {
            if (dfsUtil(maze, {nr,nc}, end, visited, path)) {
                path.push_back(cur);
                return true;
            }
        }
    }
    return false;
}

std::vector<Position> solveDFS(const Maze& maze) {
    auto S = maze.getStart(), E = maze.getEnd();
    int R = maze.getRows(), C = maze.getCols();
    std::vector<std::vector<bool>> visited(R, std::vector<bool>(C,false));
    std::vector<Position> path;
    if (dfsUtil(maze, S, E, visited, path)) {
        std::reverse(path.begin(), path.end());
    }
    return path;
}

// Iterative BFS
std::vector<Position> solveBFS(const Maze& maze) {
    auto S = maze.getStart(), E = maze.getEnd();
    int R = maze.getRows(), C = maze.getCols();
    std::vector<std::vector<bool>> visited(R, std::vector<bool>(C,false));
    std::vector<std::vector<Position>> parent(R, std::vector<Position>(C,{-1,-1}));
    std::queue<Position> q;
    visited[S.first][S.second] = true;
    q.push(S);

    static const int dr[4] = {-1,1,0,0}, dc[4] = {0,0,-1,1};
    while (!q.empty()) {
        auto [r,c] = q.front(); q.pop();
        if (std::make_pair(r,c) == E) break;
        for (int i = 0; i < 4; ++i) {
            int nr = r + dr[i], nc = c + dc[i];
            if (maze.isOpen(nr,nc) && !visited[nr][nc]) {
                visited[nr][nc] = true;
                parent[nr][nc] = {r,c};
                q.push({nr,nc});
            }
        }
    }
    // reconstruct
    std::vector<Position> path;
    if (!visited[E.first][E.second]) return path;
    for (Position at = E; at != S; at = parent[at.first][at.second]) {
        path.push_back(at);
    }
    path.push_back(S);
    std::reverse(path.begin(), path.end());
    return path;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0]
                  << " <input_maze.txt> <dfs|bfs> [output_maze.txt]\n";
        return 1;
    }
    std::string infile  = argv[1];
    std::string method  = argv[2];
    std::string outfile = (argc >= 4 ? argv[3] : "");

    Maze maze;
    if (!maze.load(infile)) {
        std::cerr << "Error: cannot load " << infile << "\n";
        return 1;
    }

    std::vector<Position> path;
    if (method == "dfs") {
        path = solveDFS(maze);
    } else if (method == "bfs") {
        path = solveBFS(maze);
    } else {
        std::cerr << "Error: unknown method '" << method << "'. Use dfs or bfs.\n";
        return 1;
    }

    if (path.empty()) {
        std::cout << "No path found.\n";
    } else {
        maze.markPath(path, '*');
        if (!outfile.empty()) {
            maze.save(outfile);
            std::cout << "Solution saved to " << outfile << "\n";
        } else {
            maze.print();
        }
    }
    return 0;
}
