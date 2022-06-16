#include "Game.h"
#include "Board.h"
#include "Player.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <cctype>
#include<vector>
#include<algorithm>

using namespace std;

class GameImpl
{
  public:
    GameImpl(int nRows, int nCols);
    int rows() const;
    int cols() const;
    bool isValid(Point p) const;
    Point randomPoint() const;
    bool addShip(int length, char symbol, string name);
    int nShips() const;
    int shipLength(int shipId) const;
    char shipSymbol(int shipId) const;
    string shipName(int shipId) const;
    //auxiliary function to change the order in which the ships are stored in the vectors so that it's in the order of biggest ship size to lowest
    void changeOrder();
    Player* play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause);
private:
    int mrows;
    int mcols;
    int numShips = 0;
    bool changed = false;
    //these vectors store the length, symbol, and name of the 5 ships for each player
    vector <int> lengths;
    vector <char> symbols;
    vector <string> names;
    vector <int> newLengths;
    vector <char> newSymbols;
    vector <string> newNames;

};

void waitForEnter()
{
    cout << "Press enter to continue: ";
    cin.ignore(10000, '\n');
}

//Construct a game with the indicated number of rows and columns
GameImpl::GameImpl(int nRows, int nCols)
{
    mrows = nRows;
    mcols = nCols;
}

int GameImpl::rows() const
{
    return mrows;
}

int GameImpl::cols() const
{
    return mcols;
}

//Return true if and only if the point denotes a position on the game board.
bool GameImpl::isValid(Point p) const
{
    return p.r >= 0  &&  p.r < rows()  &&  p.c >= 0  &&  p.c < cols();
}

Point GameImpl::randomPoint() const
{
    return Point(randInt(rows()), randInt(cols()));
}

bool GameImpl::addShip(int length, char symbol, string name)
{
    if (length <= 0)
    {
        return false;
    }
    if (symbol == 'X' || symbol == 'o' || symbol == '.')
    {
        return false;
    }
    //now check if the symbol has already been used
    for (int i=0; i<(int)symbols.size(); i++)
    {
        if (symbols[i]==symbol)
        {
            return false;
        }
    }
    
    lengths.push_back(length);
    symbols.push_back(symbol);
    names.push_back(name);
    numShips++;
    return true;
}

int GameImpl::nShips() const
{
    return (int)lengths.size();
}

int GameImpl::shipLength(int shipId) const
{
    if (!changed)
    {
        return lengths.at(shipId);
    }
    else
    {
        return newLengths.at(shipId);
    }
}

char GameImpl::shipSymbol(int shipId) const
{
    if (!changed)
    {
        return symbols.at(shipId);
    }
    else
    {
        return newSymbols.at(shipId);
    }
}

string GameImpl::shipName(int shipId) const
{
    if (!changed)
    {
        return names.at(shipId);
    }
    else
    {
        return newNames.at(shipId);
    }
}

//auxiliary function to change the order in which the ships are stored in the vectors so that it's in the order of biggest ship size to lowest
void GameImpl::changeOrder()
{
    //temporary vector that will store length's original values
    vector <int> temp;
    for (int j=0; j<lengths.size(); j++)
    {
        temp.push_back(lengths[j]);
    }
    
    //now, newLengths will get the values from lengths after it's sorted in descending order
    sort(lengths.begin(),lengths.end(), greater<int>()); //Sorting the vector using greater<int>() function
    for (int j=0; j<lengths.size(); j++)
    {
        newLengths.push_back(lengths[j]);
    }
    
    vector <int> newIndex;
    
    for (int i=0; i<newLengths.size(); i++)
    {
        int a = newLengths[i];
        
        for (int j=0; j<temp.size(); j++)
        {
            if (a == temp[j]) //find the index where the element from newLengths is in the original vector
            {
                temp[j] = -1;
                newIndex.push_back(j);
                break; //should break out of innter for loop
            }
        }
    }
    //now sort the vectors of names, symbols, and shipdestroyed based on how lengths was sorted
    for (int k=0; k<newIndex.size(); k++)
    {
        newNames.push_back(names[newIndex[k]]);
        newSymbols.push_back(symbols[newIndex[k]]);
    }
    
    changed = true;
}

Player* GameImpl::play(Player* p1, Player* p2, Board& b1, Board& b2, bool shouldPause)
{
    changeOrder();
    
    //calls the placeShips function of each player to place the ships on their respective board
    if (!p1->placeShips(b1) || !p2->placeShips(b2))
    {
        return nullptr;
    }
    
    int n=0;
    int p1destroyed = 0;
    int p2destroyed = 0;
    
    bool keepLoop = true;
    while (keepLoop)
    {
        if (n%2==0) //player 1
        {
            bool humanity = p1->isHuman();
            if(humanity)
            {
                cout << p1->name() << "'s turn. Board for " << p2->name() << ":" << endl;
                b2.display(true);
            }
            else
            {
                cout << p1->name() << "'s turn. Board for " << p2->name() << ":" << endl;
                b2.display(false);
            }
            //now we will attack
            Point p = p1->recommendAttack();
            bool shotHit = false;
            bool shipDestroyed = false;
            int shipId = 100;
            if (b2.attack(p, shotHit, shipDestroyed, shipId))
            {
                p1->recordAttackResult(p, true, shotHit, shipDestroyed, shipId);
                
                p1->recordAttackByOpponent(p);
                
                if (!shotHit)
                {
                    cout << p1->name() << " attacked (" << p.r << "," << p.c << ") and missed, resulting in:" << endl;
                }
                else if(shotHit && shipDestroyed)
                {
                    cout << p1->name() << " attacked (" << p.r << "," << p.c << ") and destroyed the " << shipName(shipId) << ", resulting in:" << endl;
                }
                else //shot was hit but no ship destroyed
                {
                    cout << p1->name() << " attacked (" << p.r << "," << p.c << ") and hit something, resulting in:" << endl;
                }
                                
                //Display the result of the attack
                if(humanity)
                {
                    b2.display(true);
                }
                else
                {
                    b2.display(false);
                }
                
                //If all ships are destroyed, this person loses and we end the game
//.                if (shipDestroyed)
//                {
//                    p2destroyed++;
//                }

                if (b2.allShipsDestroyed())
                {
                    keepLoop = false;
                    cout << p1->name() << " wins!" << endl;
                    //If the losing player is human, display the winner's board, showing everything
                    //. here, are we supposed to show the board before it got altered from X and o?
                    if(p2->isHuman())
                    {
                        b1.display(false);
                    }
                    return p1;
                }
                else if(shouldPause)
                {
                    waitForEnter();
                }
            }
            else //comes here if the attack was invalid
            {
                cout << p1->name() << " wasted a shot at (" << p.r << "," << p.c << ")." << endl;
            }
        }
        else //player 2
        {
            bool humanity = p2->isHuman();
            if(humanity)
            {
                cout << p2->name() << "'s turn. Board for " << p1->name() << ":" << endl;
                b1.display(true);
            }
            else
            {
                cout << p2->name() << "'s turn. Board for " << p1->name() << ":" << endl;
                b1.display(false);
            }
            Point p = p2->recommendAttack();
            bool shotHit = false;
            bool shipDestroyed = false;
            int shipId = 100;
            if (b1.attack(p, shotHit, shipDestroyed, shipId))
            {
                p2->recordAttackResult(p, true, shotHit, shipDestroyed, shipId);
                p2->recordAttackByOpponent(p);
                
                if (!shotHit)
                {
                    cout << p2->name() << " attacked (" << p.r << "," << p.c << ") and missed, resulting in:" << endl;
                }
                else if(shotHit && shipDestroyed)
                {
                    cout << p2->name() << " attacked (" << p.r << "," << p.c << ") and destroyed the " << shipName(shipId) << ", resulting in:" << endl;
                }
                else
                {
                    cout << p2->name() << " attacked (" << p.r << "," << p.c << ") and hit something, resulting in:" << endl;
                }
                
                //Display the result of the attack
                if(humanity)
                {
                    b1.display(true);
                }
                else
                {
                    b1.display(false);
                }
                
                //If all ships are destroyed, this person loses and we end the game
//                if (shipDestroyed)
//                {
//                    p1destroyed++;
//                }

                if (b1.allShipsDestroyed())
                {
                    keepLoop = false;
                    cout << p2->name() << " wins!" << endl;
                    //If the losing player is human, display the winner's board, showing everything
                    //. here, are we supposed to show the board before it got altered from X and o?
                    if(p1->isHuman())
                    {
                        b2.display(false);
                    }
                    return p2;
                }
                else if(shouldPause)
                {
                    waitForEnter();
                }
            }
            else //comes here if the attack was invalid
            {
                cout << p2->name() << " wasted a shot at (" << p.r << "," << p.c << ")." << endl;
            }
        }
        n++;
    }
    return nullptr;
}

//******************** Game functions *******************************

// These functions for the most part simply delegate to GameImpl's functions.
// You probably don't want to change any of the code from this point down.

Game::Game(int nRows, int nCols)
{
    if (nRows < 1  ||  nRows > MAXROWS)
    {
        cout << "Number of rows must be >= 1 and <= " << MAXROWS << endl;
        exit(1);
    }
    if (nCols < 1  ||  nCols > MAXCOLS)
    {
        cout << "Number of columns must be >= 1 and <= " << MAXCOLS << endl;
        exit(1);
    }
    m_impl = new GameImpl(nRows, nCols);
}

Game::~Game()
{
    delete m_impl;
}

int Game::rows() const
{
    return m_impl->rows();
}

int Game::cols() const
{
    return m_impl->cols();
}

bool Game::isValid(Point p) const
{
    return m_impl->isValid(p);
}

Point Game::randomPoint() const
{
    return m_impl->randomPoint();
}

bool Game::addShip(int length, char symbol, string name)
{
    if (length < 1)
    {
        cout << "Bad ship length " << length << "; it must be >= 1" << endl;
        return false;
    }
    if (length > rows()  &&  length > cols())
    {
        cout << "Bad ship length " << length << "; it won't fit on the board"
             << endl;
        return false;
    }
    if (!isascii(symbol)  ||  !isprint(symbol))
    {
        cout << "Unprintable character with decimal value " << symbol
             << " must not be used as a ship symbol" << endl;
        return false;
    }
    if (symbol == 'X'  ||  symbol == '.'  ||  symbol == 'o')
    {
        cout << "Character " << symbol << " must not be used as a ship symbol"
             << endl;
        return false;
    }
    int totalOfLengths = 0;
    for (int s = 0; s < nShips(); s++)
    {
        totalOfLengths += shipLength(s);
        if (shipSymbol(s) == symbol)
        {
            cout << "Ship symbol " << symbol
                 << " must not be used for more than one ship" << endl;
            return false;
        }
    }
    if (totalOfLengths + length > rows() * cols())
    {
        cout << "Board is too small to fit all ships" << endl;
        return false;
    }
    return m_impl->addShip(length, symbol, name);
}

int Game::nShips() const
{
    return m_impl->nShips();
}

int Game::shipLength(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipLength(shipId);
}

char Game::shipSymbol(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipSymbol(shipId);
}

string Game::shipName(int shipId) const
{
    assert(shipId >= 0  &&  shipId < nShips());
    return m_impl->shipName(shipId);
}

Player* Game::play(Player* p1, Player* p2, bool shouldPause)
{
    if (p1 == nullptr  ||  p2 == nullptr  ||  nShips() == 0) 
        return nullptr;
    Board b1(*this);
    Board b2(*this);
    return m_impl->play(p1, p2, b1, b2, shouldPause);
}

