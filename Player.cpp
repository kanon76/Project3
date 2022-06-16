#include "Player.h"
#include "Board.h"
#include "Game.h"
#include "globals.h"
#include <iostream>
#include <string>
#include <stack>

using namespace std;

//*********************************************************************
//  AwfulPlayer
//*********************************************************************

class AwfulPlayer : public Player
{
  public:
    AwfulPlayer(string nm, const Game& g);
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit,
                                                bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
  private:
    Point m_lastCellAttacked;
};

AwfulPlayer::AwfulPlayer(string nm, const Game& g)
 : Player(nm, g), m_lastCellAttacked(0, 0)
{}

bool AwfulPlayer::placeShips(Board& b)
{
      // Clustering ships is bad strategy
    for (int k = 0; k < game().nShips(); k++)
        if ( ! b.placeShip(Point(k,0), k, HORIZONTAL))
            return false;
    return true;
}

Point AwfulPlayer::recommendAttack()
{
    if (m_lastCellAttacked.c > 0)
        m_lastCellAttacked.c--;
    else
    {
        m_lastCellAttacked.c = game().cols() - 1;
        if (m_lastCellAttacked.r > 0)
            m_lastCellAttacked.r--;
        else
            m_lastCellAttacked.r = game().rows() - 1;
    }
    return m_lastCellAttacked;
}

void AwfulPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
                                     bool /* shotHit */, bool /* shipDestroyed */,
                                     int /* shipId */)
{
      // AwfulPlayer completely ignores the result of any attack
}

void AwfulPlayer::recordAttackByOpponent(Point /* p */)
{
      // AwfulPlayer completely ignores what the opponent does
}

//*********************************************************************
//  HumanPlayer
//*********************************************************************

bool getLineWithTwoIntegers(int& r, int& c)
{
    bool result(cin >> r >> c);
    if (!result)
        cin.clear();  // clear error state so can do more input operations
    cin.ignore(10000, '\n');
    return result;
}

class HumanPlayer : public Player
{
  public:
    HumanPlayer(string nm, const Game& g);
    virtual bool isHuman() const { return true; }
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
};

HumanPlayer::HumanPlayer(string nm, const Game& g)
 : Player(nm, g){}

/*
 It must implement the logic required to place the ships on the board before game play begins. It returns true if all ships could be placed, and false otherwise (e.g., because the game parameters are such that there is no configuration of ships that will fit, or because a MediocrePlayer is unable to place all of the ships after 50 tries).
 */
bool HumanPlayer::placeShips(Board& b)
{
    for (int i=0; i<game().nShips(); i++)
    {
        int length = game().shipLength(i);
        string named = game().shipName(i);
        
        cout << name() << " must place " << game().nShips()-i << " ships." << endl; //first line to print
        b.display(false); //displays board with ships placed
        
        Direction dir = HORIZONTAL; // to store the direction
        bool continingLoop = false;
        
        while (continingLoop == false)
        {
            cout << "Enter h or v for direction of " << named << " (length " << length << "): "; //2nd line
            char direction;
            cin >> direction;
            cin.ignore(10000, '\n');

            if (direction == 'h')
            {
                dir = HORIZONTAL;
                continingLoop = true;
            }
            else if (direction == 'v')
            {
                dir = VERTICAL;
                continingLoop = true;
            }
            else{
                cout << "Direction must be h or v." << endl;
            }
        }
        
        bool continueLoop = false;
        
        while (continueLoop == false)
        {
            int r = -1;
            int c = -1;
            cout << "Enter row and column of topmost cell (e.g., 3 5): "; //3rd line
            if (getLineWithTwoIntegers(r, c))
            {
                Point p(r, c);
                continueLoop = b.placeShip(p, i, dir); //if ship placement is valid, continueLoop becomes true and we get out of the loop
                
                if (!continueLoop) //if the ship placement is not valid
                {
                    cout << "The ship can not be placed there." << endl;
                }
            }
            else
            {
                cout << "You must enter two integers." << endl;
            }
        }
    }
    
    return true; //when all ships are placed, return true
}

/*
It returns a point that indicates the position on the opponent's board that the player would like to attack next.
 */
Point HumanPlayer::recommendAttack()
{
    Point p;
    
    bool continueLoop = false;

    while (continueLoop == false)
    {
        int r = -1;
        int c = -1;
        
        cout << "Enter the row and column to attack (e.g., 3 5): ";
        if (getLineWithTwoIntegers(r, c))
        {
            p.r = r;
            p.c = c;
            continueLoop = true;
        }
        else
        {
            cout << "You must enter two integers." << endl;
        }
    }
    
    return p;
}

void HumanPlayer::recordAttackResult(Point /* p */, bool /* validShot */,
                                     bool /* shotHit */, bool /* shipDestroyed */,
                                     int /* shipId */)
{
      // presumably need not do anything
}

void HumanPlayer::recordAttackByOpponent(Point /* p */)
{
      // presumably need not do anything
}

//*********************************************************************
//  MediocrePlayer
//*********************************************************************

class MediocrePlayer : public Player
{
  public:
    MediocrePlayer(string nm, const Game& g);
    virtual bool isHuman() const { return false; }
    bool place (vector<Point> vect, int shipId, Board& b); //Auxiliary function that will be recursive in placeShips
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
private:
    bool recs = false;
    Point justAttacked;
    stack <Point> pointToCheck;
    vector <Point> alreadyAttacked;
};

MediocrePlayer::MediocrePlayer(string nm, const Game& g)
 : Player(nm, g){}

//recursive auxiliary function to place ships
bool MediocrePlayer::place (vector<Point> vect, int shipId, Board& b)
{
    if (shipId >= game().nShips()) //base case when we ran out of ships to place
    {
        return true;
    }
    
    for (int i=0; i<vect.size(); i++) //look thru every point on the board until a placement is successful
    {
        Direction dir = HORIZONTAL;
        Direction dir2 = VERTICAL;
        if (b.placeShip(vect[i], shipId, dir)) //try placing the ship horizontally
        {
            if (place(vect, shipId+1, b)) //if that was successful, try the next ship
            {
                return true;
            }
            else{
                b.unplaceShip(vect[i], shipId, dir); //if that was unsuccessful, unplace the ship
            }
        }
        if (b.placeShip(vect[i], shipId, dir2)) //try placing the ship vertically
        {
            if (place(vect, shipId+1, b)) //if that was successful, try the next ship
            {
                return true;
            }
            else{
                b.unplaceShip(vect[i], shipId, dir2); //if that was unsuccessful, unplace the ship
            }
        }
    }
    
    return false; //if the ships could not be placed, return false
}

bool MediocrePlayer::placeShips(Board& b)
{
    for (int i=0; i<50; i++)
    {
        b.block(); // first block out the points
        
        //create a vector of points on the board
        vector <Point> allPoints;
        
        for (int i=0; i<game().rows(); i++)
        {
            for (int j=0; j<game().cols(); j++)
            {
                Point p;
                p.r = i;
                p.c = j;
                allPoints.push_back(p);
            }
        }
        
        //Auxiliary function that will be recursive
        int shipId = 0;
        bool havePlaced = place(allPoints, shipId, b); //this will place all the ships
        
        b.unblock(); //now unblock
        
        if (havePlaced) // if successful, end here
        {
            return true;
        }
        
        //if not, go through another iteration of the for loop until it does 50 tries
//        allPoints.erase(allPoints.begin(), allPoints.end());
    }

    return false; //after 50 tries and no way to place all of the ships, return false
}

void MediocrePlayer::recordAttackByOpponent(Point /* p */)
{
      // presumably need not do anything
}

Point MediocrePlayer::recommendAttack()
{
    Point a;
    
    if (!recs) //State 1
    {
        bool continuing = true;
        
        while (continuing)
        {
            a = game().randomPoint();
            
            bool notFound = true;
            for (int i=0; i<alreadyAttacked.size(); i++)
            {
                if(alreadyAttacked[i].r == a.r && alreadyAttacked[i].c == a.c)
                {
                    notFound = false;
                }
            }
            if (notFound)
            {
                continuing = false;
            }
        }
    }
    else //State 2
    {
        bool continuing = true;
        
        while (continuing)
        {
            if (!pointToCheck.empty() && game().isValid(pointToCheck.top())) //checks if the next point on stack is a valid point
            {
                a = pointToCheck.top();
                pointToCheck.pop();
                
                bool notFound = true;
                for (int i=0; i<alreadyAttacked.size(); i++)
                {
                    if(alreadyAttacked[i].r == a.r && alreadyAttacked[i].c == a.c)
                    {
                        notFound = false;
                    }
                }
                if (notFound)
                {
                    continuing = false;
                }
            }
            else if (!pointToCheck.empty() && !game().isValid(pointToCheck.top()))
            {
                pointToCheck.pop();
            }
            else
            {
                recs = false; //reset
                a = game().randomPoint();
                
                bool notFound = true;
                for (int i=0; i<alreadyAttacked.size(); i++)
                {
                    if(alreadyAttacked[i].r == a.r && alreadyAttacked[i].c == a.c)
                    {
                        notFound = false;
                    }
                }
                if (notFound)
                {
                    continuing = false;
                }
            }
        }
    }

    alreadyAttacked.push_back(a);
    return a;
}

void MediocrePlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    if (!recs && shotHit && !shipDestroyed)
    {
        justAttacked = p;
        for (int i=1; i<=4; i++)
        {
            Point q;
            Point k;
            int r = justAttacked.r + i;
            int c = justAttacked.c + i;
            q.r =r;
            q.c =justAttacked.c;
            k.r = justAttacked.r;
            k.c = c;
            pointToCheck.push(q);
            pointToCheck.push(k);
        }
        for (int i=1; i<=4; i++)
        {
            Point q;
            Point k;
            int r = justAttacked.r - i;
            int c = justAttacked.c - i;
            q.r =r;
            q.c =justAttacked.c;
            k.r = justAttacked.r;
            k.c = c;
            pointToCheck.push(q);
            pointToCheck.push(k);
        }
        recs = true; // this makes recommendAttack go into State 2 next time it's called
    }
    else if(recs && shotHit && shipDestroyed)
    {
        for (int i=0; i<pointToCheck.size(); i++)
        {
            pointToCheck.pop();
        }
        recs = false;
    }
}

//*********************************************************************
//  GoodPlayer
//*********************************************************************

class GoodPlayer : public Player
{
  public:
    GoodPlayer(string nm, const Game& g);
    virtual bool isHuman() const { return false; }
    virtual bool placeShips(Board& b);
    virtual Point recommendAttack();
    virtual void recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId);
    virtual void recordAttackByOpponent(Point p);
private:
    int recs = 1;
    stack <Point> pointStack;
    Point justAttacked;
    vector <Point> alreadyAttacked;
};

GoodPlayer::GoodPlayer(string nm, const Game& g)
 : Player(nm, g){}

bool GoodPlayer::placeShips(Board& b)
{
    //place ships randomly
    for (int i=0; i<game().nShips(); i++)
    {
        Point p = game().randomPoint();
        
        int num = randInt(2);
        if (num==0)
        {
            b.placeShip(p, i, HORIZONTAL);
        }
        else{
            b.placeShip(p, i, VERTICAL);
        }
    }
    
    return true;
}

void GoodPlayer::recordAttackByOpponent(Point /* p */)
{
      // presumably need not do anything
}

Point GoodPlayer::recommendAttack()
{
    Point a;
    
    switch (recs)
    {
        case 1:
        {
            bool continuing = true;
            
            while (continuing)
            {
                a = game().randomPoint();
                bool notFound = true;
                for (int i=0; i<alreadyAttacked.size(); i++)
                {
                    if(alreadyAttacked[i].r == a.r && alreadyAttacked[i].c == a.c)
                    {
                        notFound = false;
                    }
                }
                if (notFound)
                {
                    continuing = false;
                }
            }
            
            break;
        }
        case 2:
        {
            bool continuing = true;
            
            while (continuing)
            {
                if (!pointStack.empty())
                {
                    a = pointStack.top();
                    pointStack.pop();
                }
                else
                {
                    recs = 1; //reset
                    a = game().randomPoint();
                }
                bool notFound = true;
                for (int i=0; i<alreadyAttacked.size(); i++)
                {
                    if(alreadyAttacked[i].r == a.r && alreadyAttacked[i].c == a.c)
                    {
                        notFound = false;
                    }
                }
                if (notFound)
                {
                    continuing = false;
                }
            }
            
            break;
        }
        case 3:
        {
            if (!pointStack.empty())
            {
                a = pointStack.top();
                pointStack.pop();
            }
            else
            {
                recs = 1; //reset
                
                bool continuing = true;
                
                while (continuing)
                {
                    a = game().randomPoint();
                    bool notFound = true;
                    for (int i=0; i<alreadyAttacked.size(); i++)
                    {
                        if(alreadyAttacked[i].r == a.r && alreadyAttacked[i].c == a.c)
                        {
                            notFound = false;
                        }
                    }
                    if (notFound)
                    {
                        continuing = false;
                    }
                }
                
                break;
            }
            
            break;
        }
    }
    
    alreadyAttacked.push_back(a);
    return a;
}

void GoodPlayer::recordAttackResult(Point p, bool validShot, bool shotHit, bool shipDestroyed, int shipId)
{
    if (recs==1 && shotHit && !shipDestroyed)
    {
        justAttacked = p;
        //add all 4 directions from the attacked point
        Point n (p.r,p.c-1);
        if (game().isValid(n)) //1 left
        {
            pointStack.push(n);
        }
        Point s (p.r,p.c+1);
        if (game().isValid(s)) //1 right
        {
            pointStack.push(s);
        }
        Point g (p.r-1,p.c);
        if (game().isValid(g)) //1 above
        {
            pointStack.push(g);
        }
        Point b (p.r+1,p.c);
        if (game().isValid(b)) //1 below
        {
            pointStack.push(b);
        }
        
        recs = 2; // this makes recommendAttack go into State 2 next time it's called
    }
    else if(recs==2 && shotHit && shipDestroyed)
    {
        //pop off everything from the stack
        for (int i=0; i<pointStack.size(); i++)
        {
            pointStack.pop();
        }
        recs = 1; //return recs to 1 so that it starts at state 1 for the next round
    }
    else if(recs==2 && shotHit && !shipDestroyed) //size 3 or more; add point right above/below or right/left to increase chance of hitting the segment right next to the previously hit segment
    {
        Point o(p.r+1, p.c);
        Point o1(p.r-1, p.c);
        Point o2(p.r, p.c+1);
        Point o3(p.r, p.c-1);
        if (p.r > justAttacked.r && game().isValid(o)) //if last attack was below the original attack and the next attack can still fit in the board
        {
            pointStack.push(o);
        }
        else if (p.r < justAttacked.r && game().isValid(o1))
        {
            pointStack.push(o1);
        }
        else if (p.c > justAttacked.c && game().isValid(o2))
        {
            pointStack.push(o2);
        }
        else if (p.c < justAttacked.c && game().isValid(o3))
        {
            pointStack.push(o3);
        }
        
        recs = 3;
    }
    else if (recs==3 && !shotHit) //tried the spot right next to it but there was no segment
    {
        recs = 2;
    }
    else if(recs==3 && shotHit && !shipDestroyed) //if there was a segment right next to the previously hit segment
    {
        Point o(p.r+1, p.c);
        Point o1(p.r-1, p.c);
        Point o2(p.r, p.c+1);
        Point o3(p.r, p.c-1);
        if (p.r > justAttacked.r && game().isValid(o)) //if last attack was below the original attack and the next attack can still fit in the board
        {
            pointStack.push(o);
        }
        else if (p.r < justAttacked.r && game().isValid(o1))
        {
            pointStack.push(o1);
        }
        else if (p.c > justAttacked.c && game().isValid(o2))
        {
            pointStack.push(o2);
        }
        else if (p.c < justAttacked.c && game().isValid(o3))
        {
            pointStack.push(o3);
        }
    }
    else if(recs==3 && shotHit && shipDestroyed)
    {
        //pop off everything from the stack
        for (int i=0; i<pointStack.size(); i++)
        {
            pointStack.pop();
        }
        recs = 1; //return recs to 1 so that it starts at state 1 for the next round
    }
}

//*********************************************************************
//  createPlayer
//*********************************************************************

Player* createPlayer(string type, string nm, const Game& g)
{
    static string types[] = {
        "human", "awful", "mediocre", "good"
    };
    
    int pos;
    for (pos = 0; pos != sizeof(types)/sizeof(types[0])  &&
                                                     type != types[pos]; pos++)
        ;
    switch (pos)
    {
      case 0:  return new HumanPlayer(nm, g);
      case 1:  return new AwfulPlayer(nm, g);
      case 2:  return new MediocrePlayer(nm, g);
      case 3:  return new GoodPlayer(nm, g);
      default: return nullptr;
    }
}
