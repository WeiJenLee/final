/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

static bool isinvert(unsigned int number)
{
  if(number == 0)
    return false;
  if(number == 1)
    return true;
  return number/2 != (number+1)/2;
}
static bool isUndef(CirGate* tmp){return tmp->getTypeStr() == "UNDEF";}

unsigned int
pin::getID() const {return connectGate->ID;}

const string
pin::getTypeStr() const
{
  switch(connectGate->_gateType){
    case UNDEF_GATE:
      return "UNDEF";
    case PI_GATE:
      return "PI";
    case PO_GATE:
      return "PO";
    case AIG_GATE:
      return "AIG";
    case CONST_GATE:
      return "CONST";
  }
}

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
  cout << "==================================================\n= "
       << getTypeStr() << "(" << ID << "), line " << lineNo;
  if(symbol[0])
    cout << " \"" << symbol << "\"";
  cout << "\n==================================================\n";
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   reportFaninhelp(cirMgr->getGate(this->ID), level, 1);
   cirMgr->resetVisit();
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   reportFanouthelp(cirMgr->getGate(this->ID), level, 1);
   cirMgr->resetVisit();
}

void
CirGate::add_fanin(unsigned int id)
{
   pin pinin;
   pinin.setGate(cirMgr->getGate(id/2));
   pinin.setinv(isinvert(id));
   _fanin.push_back(pinin);
}

void
CirGate::removed_fanin(unsigned int id)
{
   for(size_t i=0; i<_fanin.size(); ++i)
     if(_fanin[i].getID() == id)
       _fanin.erase(_fanout.begin()+i);
}

void
CirGate::replace_fanin(unsigned int _id, CirGate* tmp, bool invert)
{
  pin pinin;
  pinin.setGate(tmp);
  pinin.setinv(invert);
  for(size_t i=0; i<_fanin.size(); ++i)
    if(_fanin[i].getID() == _id)
      _fanin[i] = pinin;
}

void
CirGate::add_fanout(unsigned int id, bool invert)
{
   pin pinout;
   pinout.setGate(cirMgr->getGate(id));
   pinout.setinv(invert);
   _fanout.push_back(pinout);
}

void
CirGate::add_fanout(pin newfan) { _fanout.push_back(newfan); }

void
CirGate::replace_fanout(unsigned int _id, CirGate* tmp, bool invert)
{
  pin pinin;
  pinin.setGate(tmp);
  pinin.setinv(invert);
  for(size_t i=0; i<_fanout.size(); ++i)
    if(_fanout[i].getID() == _id)
      _fanout[i] = pinin;
}

void
CirGate::removed_fanout(unsigned int id)
{
   for(size_t i=0; i<_fanout.size(); ++i)
     if(_fanout[i].getID() == id)
       _fanout.erase(_fanout.begin()+i);
}

CirAndGate::CirAndGate(unsigned int _id, unsigned int _line):CirGate(AIG_GATE, _line) { ID = _id/2; }

void
CirAndGate::printGate() const
{
   cout << this->getTypeStr() << " " << ID << "   "
        << ( isUndef(_fanin[0].getGate()) ? "*" : "")
        << (_fanin[0].isinv()? "!" : "") << _fanin[0].getID() << " "
        << ( isUndef(_fanin[1].getGate()) ? "*" : "")
        << (_fanin[1].isinv()? "!" : "") << _fanin[1].getID() << endl;
}

CirInGate::CirInGate(unsigned int _id, unsigned int _line):CirGate(PI_GATE, _line){ ID = _id/2; needSweep = false;}

void
CirInGate::printGate() const
{
   cout << this->getTypeStr() << "\t" << ID;
   if(symbol[0])
     cout << "(" << symbol << ")";
   cout << endl;
}

CirOutGate::CirOutGate(unsigned int _id, unsigned int _line):CirGate(PO_GATE, _line)
{
   ID = _id;
   _Flt = false;
   needSweep = false;
}

void
CirOutGate::printGate() const
{
   cout << this->getTypeStr() << "\t" << ID << "   "
        << ( isUndef(_fanin[0].getGate()) ? "*" : "")
        << (_fanin[0].isinv() ? "!" : "") << _fanin[0].getID();
   if(symbol[0])
     cout << " (" << symbol << ")";
   cout << endl;
}

CirConstGate::CirConstGate():CirGate(CONST_GATE, 0)
{
   ID = 0;
   _Flt = false;
}

CirUndefGate::CirUndefGate(unsigned int _id):CirGate(UNDEF_GATE, 0) { ID = _id; }

string
CirGate::getTypeStr() const
{
  switch(_gateType){
    case UNDEF_GATE:
      return "UNDEF";
    case PI_GATE:
      return "PI";
    case PO_GATE:
      return "PO";
    case AIG_GATE:
      return "AIG";
    case CONST_GATE:
      return "CONST";
  }
}

void
CirGate::reportFaninhelp(CirGate* tmp, int level, int space) const
{
   cout << tmp->getTypeStr() << " " << tmp->getID() << (tmp->_visit ? "*\n" : "\n");
   if(level == 0 || tmp->_visit)
     return;
   tmp->_visit = true;

   for(size_t i=0; i<tmp->_fanin.size(); ++i)
   {
     for(int j=0; j<2*space; ++j)
       cout << " ";
     cout << (tmp->_fanin[i].isinv() ? "!":"");
     reportFaninhelp(tmp->_fanin[i].getGate(), level-1, space+1);
   }
}

void
CirGate::reportFanouthelp(CirGate* tmp, int level, int space) const
{
   cout << tmp->getTypeStr() << " " << tmp->getID() << (tmp->_visit ? "*\n" : "\n");
   if(level == 0 || tmp->_visit)
     return;
   tmp->_visit = true;

   for(size_t i=0; i<tmp->_fanout.size(); ++i)
   {
     for(int j=0; j<2*space; ++j)
       cout << " ";
     cout << (tmp->_fanout[i].isinv() ? "!":"");
     reportFanouthelp(tmp->_fanout[i].getGate(), level-1, space+1);
   }
}
