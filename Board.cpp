#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include<vector>

using namespace std;

class BoardImpl
{
  public:
    BoardImpl(const Game& g);
    void clear();
    void block();
    void unblock();
    bool placeShip(Point topOrLeft, int shipId, Direction dir);
    bool unplaceShip(Point topOrLeft, int shipId, Direction dir);
    void display(bool shotsOnly) const;
    bool attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId);
    bool allShipsDestroyed() const;

  private:
    const Game& m_game;
    char board[MAXROWS][MAXCOLS]; //the actual 10x10 board game
    //2d array of bools
    vector <char> placed; //this keeps track of whether the ship has been placed yet
    vector <int> destroyed; //this keeps track of the ships completely destroyed
};

BoardImpl::BoardImpl(const Game& g)
 : m_game(g)
{
    for (int i=0; i<g.rows(); i++)
    {
        for (int j=0; j<g.cols(); j++)
        {
            board[i][j] = '.';
        }
    }
}

void BoardImpl::clear()
{
    for (int i=0; i<m_game.rows(); i++)
    {
        for (int j=0; j<m_game.cols(); j++)
        {
            board[i][j] = '.';
        }
    }
}

void BoardImpl::block()
{
      // Block cells with 50% probability
    int numCells = m_game.rows()*m_game.cols()/2;
    for (int i=0; i<numCells; i++)
    {
        Point p = m_game.randomPoint();
        if (board[p.r][p.c] != 'b')
        {
            board[p.r][p.c] = 'b'; //blocks cell at Point p
        }
        else{
            i--;
        }
    }
}

void BoardImpl::unblock()
{
    for (int r = 0; r < m_game.rows(); r++)
        for (int c = 0; c < m_game.cols(); c++)
        {
            if (board[r][c] == 'b')
            {
                board[r][c] = '.'; //unblocks cell (r,c) if blocked
            }
        }
}

bool BoardImpl::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    if (shipId < 0 || shipId >= m_game.nShips()) //validating shipId
    {
        return false;
    }
    
    int length = m_game.shipLength(shipId);
    char symbol = m_game.shipSymbol(shipId);
    
    //first check whether that ship has already been placed
    for (int i=0; i<placed.size(); i++)
    {
        if (placed[i] == symbol)
        {
            return false;
        }
    }
    
    int r1 =0;
    int c1 =0;
    
    if (dir == VERTICAL)
    {
        if (topOrLeft.r+length <= m_game.rows()) // ship is fully inside the board
        {
            for (int r=topOrLeft.r; r < topOrLeft.r+length; r++)
            {
                if (board[r][topOrLeft.c] != '.')
                {
                    return false; //overlaps already placed ship or is blocked
                }
                r1 = r;
            }
        }
        else{
            return false; //doesn't fit inside the board
        }
    }
    else // dir is horizontal
    {
        if (topOrLeft.c+length <= m_game.cols()) // ship is fully inside the board
        {
            for (int c=topOrLeft.c; c < topOrLeft.c+length; c++)
            {
                if (board[topOrLeft.r][c] != '.')
                {
                    return false; //overlaps already placed ship or is blocked
                }
                c1 = c;
            }
        }
        else{
            return false; //doesn't fit inside the board
        }
    }
    
    if (r1 == topOrLeft.r+length-1) //if every condition was satisfied for placing the ship vertically, this statement would be true
    {
        for (int k=topOrLeft.r; k < topOrLeft.r+length; k++)
        {
            board[k][topOrLeft.c] = symbol;
        }
    }
    else if (c1 == topOrLeft.c+length-1) //if every condition was satisfied for placing the ship horizontally, this statement would be true
    {
        for (int k=topOrLeft.c; k<topOrLeft.c+length; k++)
        {
            board[topOrLeft.r][k] = symbol;
        }
    }
    
    placed.push_back(symbol);
    return true;
}

bool BoardImpl::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    if (shipId < 0 || shipId >= m_game.nShips()) //validating shipId
    {
        return false;
    }
    
    int length = m_game.shipLength(shipId);
    char symbol = m_game.shipSymbol(shipId);
    
    if (dir == VERTICAL)
    {
        for (int k=topOrLeft.r; k < topOrLeft.r+length; k++)
        {
            if (board[k][topOrLeft.c] != symbol) //if the board does not contain the entire ship at the indicated locations
            {
                return false;
            }
        }
        for (int k=topOrLeft.r; k < topOrLeft.r+length; k++)
        {
            //if conditions are valid, update the board
            board[k][topOrLeft.c] = '.';
        }
    }
    else // dir is horizontal
    {
        for (int k=topOrLeft.c; k < topOrLeft.c+length; k++)
        {
            if (board[topOrLeft.r][k] != symbol) //if the board does not contain the entire ship at the indicated locations
            {
                return false;
            }
        }
        for (int k=topOrLeft.c; k < topOrLeft.c+length; k++)
        {
            //if conditions are valid, update the board
            board[topOrLeft.r][k] = '.';
        }
    }
    
    //remove the symbol from the vector placed because that ship was unplaced
    int i;
    for (i=0; i<placed.size(); i++)
    {
        if (placed[i] == symbol)
        {
            break;
        }
    }
    
    placed.erase(placed.begin()+i);
    
    return true;
}

void BoardImpl::display(bool shotsOnly) const
{
    cout << "  ";
    
    for (int k=0; k<m_game.cols(); k++)
    {
        cout << k;
    }
    cout << endl;
    
    for (int i=0; i<m_game.rows(); i++)
    {
        cout << i << " ";
        
        for (int j=0; j<m_game.cols(); j++)
        {
            if (board[i][j]=='.' || board[i][j]=='o'|| board[i][j]=='X')
            {
                cout << board[i][j];
            }
            else // if the char is a symbol
            {
                if (shotsOnly)
                {
                    cout << '.';
                }
                else{
                    cout << board[i][j];
                }
            }
        }
        
        cout << endl;
    }
}

bool BoardImpl::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    if (!m_game.isValid(p) || board[p.r][p.c] == 'o' || board[p.r][p.c] == 'X') //checks validity of Point p
    {
        return false;
    }
    
    shotHit = false; //shotHit will be false unless the attack is validated later
    
    char symbol = board[p.r][p.c]; // the symbol point to be attacked
    if (board[p.r][p.c] != '.') //if Point p has a ship
    {
        shotHit = true;
        board[p.r][p.c] = 'X';
    }
    else // if there is no ship there
    {
        board[p.r][p.c] = 'o';
    }
    
    //checks if this specific attack destroyed the last undamaged segment of a ship
    bool check = false;
    for (int i=0; i<m_game.rows(); i++)
    {
        for (int j=0; j<m_game.cols(); j++)
        {
            if (board[i][j] == symbol)
            {
                check = true; //check becomes true if segment(s) still remain
            }
        }
    }
    bool remains = false;
    if (check) //if segments still remain, variable remain becomes true
    {
        remains = true;
    }
    
    if (remains) //attack was not on the last undamaged segment of a ship
    {
        shipDestroyed = false;
    }
    else //attack destroyed the last undamaged segment of a ship
    {
        shipDestroyed = true;
        //now determine the shipId of the ship the was destroyed
        for (int k=0; k<m_game.nShips(); k++)
        {
            if (m_game.shipSymbol(k) == symbol) //iterate thru the vector of ship symbols until we hit the element with the matching symbol. When that is found, set the shipId to the index & break out of the loop.
            {
                shipId = k;
                destroyed.push_back(shipId);
                break;
            }
        }
    }
    
    return true;
}

bool BoardImpl::allShipsDestroyed() const
{
    if (destroyed.size() == m_game.nShips()) //the vector destroyed will be filled with nShips if all nShips are destroyed
    {
        return true;
    }
    return false;
}

//******************** Board functions ********************************

// These functions simply delegate to BoardImpl's functions.
// You probably don't want to change any of this code.

Board::Board(const Game& g)
{
    m_impl = new BoardImpl(g);
}

Board::~Board()
{
    delete m_impl;
}

void Board::clear()
{
    m_impl->clear();
}

void Board::block()
{
    return m_impl->block();
}

void Board::unblock()
{
    return m_impl->unblock();
}

bool Board::placeShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->placeShip(topOrLeft, shipId, dir);
}

bool Board::unplaceShip(Point topOrLeft, int shipId, Direction dir)
{
    return m_impl->unplaceShip(topOrLeft, shipId, dir);
}

void Board::display(bool shotsOnly) const
{
    m_impl->display(shotsOnly);
}

bool Board::attack(Point p, bool& shotHit, bool& shipDestroyed, int& shipId)
{
    return m_impl->attack(p, shotHit, shipDestroyed, shipId);
}

bool Board::allShipsDestroyed() const
{
    return m_impl->allShipsDestroyed();
}
