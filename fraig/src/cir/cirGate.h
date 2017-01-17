/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include "cirDef.h"
#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class pin
{

public:
   pin() : inv(true), connectGate(NULL) {};
   pin(bool invert, CirGate* tmp) : inv(invert), connectGate(tmp) {}
   ~pin() {}

   pin& operator = (const pin& refpin)
   {
     this->inv = refpin.inv;
     this->connectGate = refpin.connectGate;
     return *this;
   }

   bool operator == (const pin& n) const
   {
     if(n.inv == this->inv && n.connectGate == this->connectGate)
       return true;
     else
       return false;
   }

   bool isinv() const { return inv; }
   CirGate* getGate() const { return connectGate; }
   void setinv(bool invert) {inv = invert;}
   void setGate(CirGate* tmp) {connectGate = tmp;}
   const unsigned int getID() const;
   const string getTypeStr() const;
   const Var getSATID() const;

 private:
   bool inv;
   CirGate* connectGate;

};

class CirGate;
//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
class CirGate
{
public:
   friend class CirMgr;
   friend class pin;
   CirGate(GateType _type, unsigned int _line)
   {
     _gateType = _type;
     lineNo = _line;
     _Flt = true;
     _visit = false;
     needSweep = true;
     Grp = 0;
     value = 0;
     valueInv = false;
   }
   virtual ~CirGate() {};

   // Basic access methods
   string getTypeStr() const;
   unsigned getLineNo() const { return lineNo; }
   unsigned int getID() const {return ID;}
   pin getfanin(unsigned int i) const {return _fanin[i];}
   pin getfanout(unsigned int i) const {return _fanout[i];}
   virtual bool isAig() const {};

   // Printing functions
   virtual void printGate() const {};
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;
   void add_fanin(unsigned int);
   void add_fanout(unsigned int, bool);
   void add_fanout(pin);
   void removed_fanin(unsigned int);
   void removed_fanout(unsigned int);
   void replace_fanin(unsigned int, CirGate*, bool);
   void replace_fanout(unsigned int, CirGate*, bool);
   void simulate(size_t const);
   void addValue(unsigned int i, size_t const num)
   {
     if(num > 31)
     {
       if(valueInv)
         value = -1;
       else
         value = 0;
     }
     if(valueInv)
       value -= (1-i)*(1 << (num%32));
     else
       value += i*(1 << (num%32));
   }

private:
  void reportFaninhelp(CirGate*, int, int) const;
  void reportFanouthelp(CirGate*, int, int) const;

protected:
   bool _Flt, _visit, needSweep, valueInv;
   unsigned int lineNo, ID, Grp, value;
   GateType _gateType;
   vector<pin> _fanin, _fanout;
   string symbol;
   Var satID;
};

class CirAndGate: public CirGate
{
 public:
   friend class CirMgr;
   friend class CirGate;
   CirAndGate(unsigned int, unsigned int);
   ~CirAndGate(){ _fanout.clear(); _fanin.clear();}
   void printGate() const;
   bool isAig() const { return true; }

private:
};

class CirInGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirInGate(unsigned int, unsigned int);
   ~CirInGate(){ _fanout.clear(); }
   void printGate() const;
   bool isAig() const { return false; }

private:
};

class CirOutGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirOutGate(unsigned int, unsigned int);
   ~CirOutGate(){_fanin.clear();}
   void printGate() const;
   bool isAig() const { return false; }

private:
};


class CirConstGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirConstGate();
   ~CirConstGate(){}

   void printGate() const
   {
     if(!symbol.empty())
       cout << this->getTypeStr() << " " << 0 << endl;
   }
   bool isAig() const { return false; }

private:
  bool print;
};

class CirUndefGate: public CirGate
{
public:
   friend class CirMgr;
   friend class CirGate;
   CirUndefGate(unsigned int);
   ~CirUndefGate(){}
   void printGate() const { cout << this->getTypeStr() << " " << ID << endl; }
   bool isAig() const { return false; }

private:
};

#endif // CIR_GATE_H
