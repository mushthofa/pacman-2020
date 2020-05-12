#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <map>
#include <unordered_map>
#include <set>
#include <string>

using namespace std;

constexpr int INF = 2'000'000'000;

constexpr int NumDir = 4;
enum class Dir {UP, RIGHT, DOWN, LEFT};
ostream & operator<<(ostream& os, Dir d)
{
    switch (d)
    {
    case Dir::UP :
        os << "^";
        break;
    case Dir::RIGHT:
        os << ">";
        break;
    case Dir::LEFT:
        os << "<";
        break;
    case Dir::DOWN:
        os <<"v";
        break;
    }

    return os;
}

constexpr char WALL = '#';
constexpr char FREE = ' ';
constexpr bool isWall(char c) { return c==WALL; }

enum class PacType {ROCK, PAPER, SCISSORS};

string toString(PacType pacType)
{
    switch(pacType)
    {
    case PacType::ROCK:
        return string("ROCK");
    case PacType::PAPER:
        return string("PAPER");
    case PacType::SCISSORS:
        return string("SCISSORS");
    }
    assert(false); 
}

PacType fromString(const string &pacTypeStr)
{
    for(int i = 0; i < 3; i++)
    {
        PacType t = static_cast<PacType>(i);
        if(toString(t) == pacTypeStr)
            return t;
    }

    assert(false);
}

PacType getDominatingType(PacType p)
{
    switch(p)
    {
    case PacType::ROCK:
        return PacType::PAPER;
    case PacType::PAPER:
        return PacType::SCISSORS;
    case PacType::SCISSORS:
        return PacType::ROCK;
    }
    assert(false);
}


class Pos
{
    public:
        Pos() : i(-1), j(-1) {}
        Pos(int ii, int jj) : i(ii), j(jj) { }
        Pos(const Pos &) = default;
        ~Pos() = default;

        bool isValid() const 
        { return (i>= 0 && j >= 0); }

        Pos & operator=(const Pos &a)
        {
            this->i = a.i;
            this->j = a.j;
            return *this;
        }
        
        int i,j;

        Pos step(Dir d, int IMAX, int JMAX) const
        {
            int p_i = i;
            int p_j = j;
            switch(d)
            {
                case Dir::UP:
                {
                    p_i--;
                    break;
                }
                case Dir::DOWN:
                {
                    p_i++;
                    break;
                }
                case Dir::LEFT:
                {
                    p_j--;
                    break;    
                }
                case Dir::RIGHT:
                {
                    p_j++;
                    break;
                }
            }

            p_i = (p_i + IMAX)%IMAX;
            p_j = (p_j + JMAX)%JMAX;

            return Pos(p_i, p_j);
        }
};

bool operator==(const Pos &a, const Pos &b)
{
    return a.i == b.i && a.j == b.j;
}

bool operator!=(const Pos &a, const Pos &b)
{
    return a.i != b.i || a.j != b.j;
}

bool operator <(const Pos &a, const Pos &b)
{
    if(a.i < b.i)
       return true;
    if(a.i > b.i)
        return false;
    
    return a.j < b.j;
}

ostream& operator<<(ostream& os, const Pos &p)
{
    os << "[" << p.j << "," << p.i << "]";
    return os;
}

class Board 
{
    public:
        Board(int imax, int jmax) : m_imax(imax), m_jmax(jmax) 
        {
            m_map.resize(m_imax);
        }

        Board(const Board &) = default;
        Board(Board &&) = default;
        ~Board() = default;

        void readMap()
        {
            for(int i = 0; i < m_imax; i++)
            {
                string row;
                getline(cin, row); // one line of the grid: space " " is floor, pound "#" is wall
                m_map[i] = row;
            }
        }

        int getIdxFromPos(const Pos &p)
        {
            return m_jmax*p.i + p.j;
        }

        Pos getPosFromIdx(int idx)
        {
            return Pos(idx/m_jmax, idx%m_jmax);
        }

        Pos move(const Pos &p, Dir d) const
        {
            return p.step(d, m_imax, m_jmax);
        }

        bool isValid(const Pos &p) const
        { return (p.i >= 0 && p.j >= 0 && p.i < m_imax && p.j < m_jmax); }

        bool isFree(const Pos &p) const 
        { return (isValid(p) && m_map[p.i][p.j] != WALL); } 

        bool isWall(const Pos &p) const 
        { return (isValid(p) && m_map[p.i][p.j] == WALL); } 


        void bfs(const Pos &source)
        {
            initMatrix(m_tempDists, INF);
            initMatrix(m_tempVisited, false);
            unordered_map<int, vector<Dir>> pathsToDests;

            auto idxSource = getIdxFromPos(source);

            m_tempDists[source.i][source.j] = 0;
            m_tempVisited[source.i][source.j] = true;
            pathsToDests.emplace(idxSource, vector<Dir>{});           

            queue<Pos> q;
            q.push(source);

            while(!q.empty())
            {
                Pos f = q.front();
                q.pop();
                
                assert(pathsToDests.find(getIdxFromPos(f)) != pathsToDests.end());
                auto currentPathToDest = pathsToDests[getIdxFromPos(f)];
                int curDist = m_tempDists[f.i][f.j];
                
                for(int i = 0; i < NumDir; i++)
                {
                    Dir d = static_cast<Dir>(i);
                    Pos n = move(f, d);
                    if(isFree(n) && !m_tempVisited[n.i][n.j])
                    {
                        m_tempDists[n.i][n.j] = curDist + 1;
                        q.push(n);
                        m_tempVisited[n.i][n.j] = true;
                        
                        currentPathToDest.push_back(d);
                        pathsToDests.emplace(getIdxFromPos(n), currentPathToDest);
                        currentPathToDest.pop_back();
                    }
                }
            }

            m_shortestPaths.emplace(idxSource, pathsToDests);
        }

        void preCalcDist()
        {
            for(int i = 0; i<m_imax; i++)
                for(int j = 0; j<m_jmax; j++)
                    {
                        if(isFree(Pos(i, j)));
                            bfs(Pos(i, j));
                    }

                
        }
    
        vector<Dir> getShortestPath(const Pos &s, const Pos &d)
        {
            auto idxSource = getIdxFromPos(s);
            auto idxDest = getIdxFromPos(d);

            auto it = m_shortestPaths.find(idxSource);
            assert(it != m_shortestPaths.end());
        
            auto myShortestPaths = m_shortestPaths[idxSource];
            auto it2 = myShortestPaths.find(idxDest);
            if(it2 == myShortestPaths.end())
                return {};
            
            return myShortestPaths[idxDest];
        }

        int getShortestDist(const Pos &s, const Pos &d)
        {
            auto idxSource = getIdxFromPos(s);
            auto idxDest = getIdxFromPos(d);

            auto it = m_shortestPaths.find(idxSource);
            assert(it != m_shortestPaths.end());
        
            auto myShortestPaths = m_shortestPaths[idxSource];
            
            auto it2 = myShortestPaths.find(idxDest);
            if(it2 == myShortestPaths.end())
                return INF;
            
            return myShortestPaths[idxDest].size();
        }

        void printShortestPath(const Pos &s, const Pos &d)
        {
            cerr << "Distance from " <<s <<" to " <<d <<" = " << getShortestDist(s, d) << endl;
            map<Pos, Dir> inPath;
            
            Pos prev = s;
            Pos next = s;
            auto shortestPath = getShortestPath(s, d);
            for (auto d : shortestPath)
            {
                prev = next;
                next = move(next, d);
                inPath.emplace(prev, d);
            }

            for(int i = 0 ; i<m_imax; i++)
            {
                for(int j = 0; j<m_jmax; j++)
                {
                    Pos p(i, j);
                    if(isWall(p))
                        cerr << WALL;
                    else if(p == s)
                        cerr <<"s";
                    else if(p == d)
                        cerr <<"f";
                    else if(inPath.find(p) != inPath.end())
                        cerr << inPath[p];
                    else 
                        cerr <<"_";
                }
                cerr << endl;
            }
        }

    private:

        template <class T, class V>
        void initMatrix(T &mat, V v)
        {
            mat.resize(m_imax);
            for(int i = 0; i < m_imax; i++)
            {   
                mat[i].assign(m_jmax, v);
            }
        }

        int m_imax{0};
        int m_jmax{0};
        vector<string> m_map;
        vector<vector<int>> m_tempDists;
        vector<vector<bool>> m_tempVisited;
        unordered_map<int, unordered_map<int, vector<Dir>>> m_shortestPaths;
};


int main()
{

    int width, height;
    cin >> width >> height; cin.ignore();
    Board b (height, width);
    b.readMap();
    b.preCalcDist();

    int x1, y1, x2, y2;
    cin >> x1 >> y1 >> x2 >> y2;
    
    Pos s (y1, x1);
    Pos d (y2, x2);
    
    b.printShortestPath(s, d);

    return 0;
}