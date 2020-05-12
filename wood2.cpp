#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <map>


using namespace std;

constexpr int INF = 2'000'000'000;

vector<string> gameMap;
vector<vector<int>> scoreMap;
vector<vector<int>> dists; //Temporary dist


int IMAX = 0;
int JMAX = 0;

enum class Dir {UP, RIGHT, DOWN, LEFT};
constexpr int NumDir = 4;
constexpr char WALL = '#';
constexpr char FREE = ' ';

int minIdx (const vector<int> &v)
{
    assert(!v.empty());

    int min_idx = 0;
    int min_el = v[0];
    for(int i = 1; i<v.size(); ++i)
    {
        if(v[i] < min_el)
        {
            min_idx = i;
            min_el = v[i];
        }
    }

    return min_idx;
}

int maxIdx (const vector<int> &v)
{
    assert(!v.empty());

    int max_idx = 0;
    int max_el = v[0];
    for(int i = 1; i<v.size(); ++i)
    {
        if(v[i] > max_el)
        {
            max_idx = i;
            max_el = v[i];
        }
    }

    return max_idx;
}


class Pos
{
    public:
        Pos(int ii, int jj) : i(ii), j(jj) {}
        Pos(const Pos &) = default;
        ~Pos() = default;

        int i,j;
};

bool operator==(const Pos &a, const Pos &b)
{
    return a.i == b.i && a.j == b.j;
}

bool operator!=(const Pos &a, const Pos &b)
{
    return a.i != b.i || a.j != b.j;
}

Pos operator+(const Pos & p, Dir d)
{
    int i = p.i;
    int j = p.j;
    switch(d)
    {
        case Dir::UP:
        {
            i--;
            break;
        }

        case Dir::DOWN:
        {
            i++;
            break;
        }

        case Dir::LEFT:
        {
            j--;
            break;    
        }

        case Dir::RIGHT:
        {
            j++;
            break;
        }
    }

    
    i = (i + IMAX)%IMAX;
    j = (j + JMAX)%JMAX;

    return Pos(i, j);
}

bool isWall(const Pos &p)
{
    return gameMap[p.i][p.j] == WALL;
}

bool isFree(const Pos &p)
{
    return gameMap[p.i][p.j] == FREE;
}

void printDist(const vector<vector<int>> &dists)
{
  for(int i=0; i < dists.size(); i++)
    {
        for(int j = 0; j < dists[i].size(); j++)
        {
            if(isWall(Pos(i, j)))
                cout << WALL << " ";
            else if(dists[i][j] >= INF)
                cout << "X ";
            else
            {
                cout << dists[i][j];
                if(dists[i][j] < 10)
                    cout << " ";
            }
        }
        cout <<endl;
    }
}
template <typename T>
void printSize(const T& arr)
{
    assert(!arr.empty());
    cout << arr.size() <<"x"<<arr[0].size() <<endl;
}

int getIdxFromPos(const Pos &p)
{
    return JMAX*p.i + p.j;
}

Pos getPosFromIdx(int idx)
{
    return Pos(idx/JMAX, idx%JMAX);
}

map<pair<int, int>, int> allDists;

void bfs(const Pos &source)
{
    vector<vector<bool>> visited;
    visited.resize(IMAX);
    for(int i = 0; i < IMAX; i++)
        visited[i].resize(JMAX, false);
    
    
    dists[source.i][source.j] = 0;
    visited[source.i][source.j] = true;

    queue<Pos> q;
    q.push(source);


    while(!q.empty())
    {
        Pos f = q.front();
        q.pop();
       
        int curDist = dists[f.i][f.j];
        
        for(int i = 0; i < NumDir; i++)
        {
            Pos n = f + static_cast<Dir>(i);
            
            if(isFree(n) && !visited[n.i][n.j])
            {
                dists[n.i][n.j] = curDist + 1;
                q.push(n);
                visited[n.i][n.j] = true;
            }
        }
    }
}

void preCalcDists()
{
    for(int i = 0; i<IMAX; i++)
        for(int j = 0; j<JMAX; j++)
            {
                bfs(Pos(i, j));
                for(int ii = 0; ii< IMAX; ii++)
                {
                    for(int jj = 0; jj<JMAX; jj++)
                    {
                        Pos source(i, j);
                        Pos dest(ii, jj);
                        auto pointPair1 = make_pair(getIdxFromPos(source), getIdxFromPos(dest));
                        allDists.emplace(pointPair1, dists[ii][jj]);
                        auto pointPair2 = make_pair(getIdxFromPos(dest), getIdxFromPos(source));
                        allDists.emplace(pointPair2, dists[ii][jj]);
                    }
                }
            }
}

int getDist(const Pos &a, const Pos&b)
{
    int idx1 = getIdxFromPos(a);
    int idx2 = getIdxFromPos(b);
    auto it = allDists.find(make_pair(idx1, idx2)); 
    if( it != allDists.end())
        return it->second;
    else
        return INF;
}

vector<int> getDists(const vector<Pos> &targets, const Pos &s)
{
    vector<int> res;
    res.reserve(targets.size());
    for(const auto &t : targets)
        res.push_back(getDist(s, t));
    
    return res;
}

Pos getTargetPellet(const vector<Pos> &supers, const vector<Pos> &pellets, 
                    const vector<Pos> &mine, const vector<Pos> &theirs, 
                    size_t pacId, vector<bool> &superTaken)
{
    Pos myPac = mine[pacId];
    auto superDists = getDists(supers, myPac);
    int minIdxSup = -1;
    int minDist = INF;
    for(int i = 0; i < supers.size(); ++i)
    {
        cerr << "taking super..." <<endl;
        if(!superTaken[i] && superDists[i] < minDist)
        {
            minIdxSup = i;
            minDist = superDists[i];
        }
    }
    
    if(minIdxSup >= 0)
    {
        superTaken[minIdxSup] = true;
        return supers[minIdxSup];
    }

    auto pelletDists = getDists(pellets, myPac);
    if(!pelletDists.empty())
    {
        cerr <<"taking normal..." <<endl;
        int idx = minIdx(pelletDists);
        return pellets[idx];   
    }

    // Park there if no more pellets
    return Pos(IMAX/2, JMAX/2);
}


int main()
{
    int width; // size of the grid
    int height; // top left corner is (x=0, y=0)
    cin >> width >> height; cin.ignore();
    
    IMAX = height;
    JMAX = width;

    gameMap.resize(IMAX);
    
    dists.resize(IMAX);
    
    for (int i = 0; i < height; i++) {
        string row;
        getline(cin, row); // one line of the grid: space " " is floor, pound "#" is wall
        gameMap[i] = row;
        dists[i].resize(JMAX, INF);
    }

    
    vector<Pos> direction;
    bool firstStep = true;
    // game loop
    while (1) {
        vector<Pos> myPacs;
        vector<int> pacIds;
        vector<Pos> theirPacs;
        vector<Pos> pellets;
        vector<Pos> supers;
        vector<bool> superTaken;

        scoreMap.resize(IMAX);
        for(int i = 0; i < IMAX; i++)
            scoreMap[i].resize(JMAX, 0);
        

        int myScore;
        int opponentScore;
        cin >> myScore >> opponentScore; cin.ignore();
        int visiblePacCount; // all your pacs and enemy pacs in sight
        cin >> visiblePacCount; cin.ignore();
        

        for (int i = 0; i < visiblePacCount; i++) {
            int pacId; // pac number (unique within a team)
            bool mine; // true if this pac is yours
            int x; // position in the grid
            int y; // position in the grid
            string typeId; // unused in wood leagues
            int speedTurnsLeft; // unused in wood leagues
            int abilityCooldown; // unused in wood leagues
            cin >> pacId >> mine >> x >> y >> typeId >> speedTurnsLeft >> abilityCooldown; cin.ignore();

            if(mine)
            {
                pacIds.push_back(pacId);
                myPacs.push_back(Pos(y, x));
            }
            else
                theirPacs.push_back(Pos(y, x));
        }
        
        int visiblePelletCount; // all pellets in sight
        cin >> visiblePelletCount; cin.ignore();
        for (int i = 0; i < visiblePelletCount; i++) {
            int x;
            int y;
            int value; // amount of points this pellet is worth
            cin >> x >> y >> value; cin.ignore();

            if(value > 1)
            {
                supers.push_back(Pos(y, x));
                superTaken.push_back(false);
            }
            else
                pellets.push_back(Pos(y, x));

            scoreMap[y][x] = value;
        }

        auto start = chrono::system_clock::now();

        if(firstStep)
        {
            direction.resize(IMAX, Pos(IMAX/2, JMAX/2));
        }

        for(size_t idxPac = 0; idxPac < myPacs.size(); idxPac++)
        {
            int y = direction[idxPac].i;
            int x = direction[idxPac].j; 

            if(!firstStep && scoreMap[y][x] > 0)
            {
                cout <<"MOVE "<< pacIds[idxPac] << " " << x << " " << y;
            }
            else
            {
                const auto &pelletPos = getTargetPellet(supers, pellets, myPacs, theirPacs, idxPac, superTaken);
                direction[idxPac] = pelletPos;
                cout <<"MOVE "<< pacIds[idxPac] << " " << pelletPos.i << " " << pelletPos.j;
            }

            if(idxPac == myPacs.size() - 1)
                cout <<endl;
            else
                cout <<" | ";
        }  
        firstStep = false;

        auto end = chrono::system_clock::now();
        auto duration = end - start;
        auto duration_ms = chrono::duration_cast<chrono::milliseconds>(duration).count();
        cerr << duration_ms << " ms" <<endl;
    }
}