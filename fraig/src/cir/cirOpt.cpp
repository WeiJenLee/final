/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep()
{
   for(size_t i=gates_num[1]+1; i<=gates_num[0]; ++i)
     if(_gates[i] && _gates[i]->needSweep)
     {
       cout << "Sweeping: " << _gates[i]->getTypeStr() << "(" << _gates[i]->getID()
            << ") removed...\n";
       if(_gates[i]->isAig())
         _AIGs.erase(std::find(_AIGs.begin(), _AIGs.end(), _gates[i]->getID()));
       for(size_t j=0; j<_gates[i]->_fanin.size(); ++j)
         (_gates[i]->_fanin)[j].getGate()->removed_fanout(i);
       delete _gates[i];
       _gates[i] = NULL;
     }
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  for(size_t i=0; i<_POs.size(); ++i)
    if(!(_gates[_POs[i]]->_visit))
    {
      _gates[_POs[i]]->_visit = true;
      DFSopt(_gates[_POs[i]]);
    }
  resetVisit();
  DFSsort();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void
CirMgr::DFSopt(CirGate* tmp)
{
  for(size_t i=0; i<tmp->_fanin.size(); ++i)
  {
    CirGate* next = (tmp->_fanin[i]).getGate();
    if(next && !(next->_visit))
    {
        next->_visit = true;
        DFSopt(next);
    }
  }
  if(tmp->getTypeStr() == "AIG")
  {
    if(tmp->_fanin[0].getTypeStr() == "CONST")
      if(tmp->_fanin[0].isinv())
        replacegate(tmp, tmp->_fanin[1].getGate());
      else
        replacegate(tmp, _gates[0]);
    else if(tmp->_fanin[1].getTypeStr() == "CONST")
      if(tmp->_fanin[1].isinv())
        replacegate(tmp, tmp->_fanin[0].getGate());
      else
        replacegate(tmp, _gates[0]);
    else if(tmp->_fanin[0].getID() == tmp->_fanin[1].getID())
      if(tmp->_fanin[0].isinv() == tmp->_fanin[1].isinv())
        replacegate(tmp, tmp->_fanin[0].getGate());
      else
        replacegate(tmp, _gates[0]);
  }
}

void
CirMgr::replacegate(CirGate* tmp, CirGate* next)
{
  bool inv = (tmp->_fanin[0].isinv() && tmp->_fanin[1].isinv());
  if(!next)
    next = new CirConstGate();
  cout << "Simplifying: " << next->ID << " merging " << (inv ? "!" : "") << tmp->ID << "...\n";
  for(size_t i=0; i<tmp->_fanin.size(); ++i)
    tmp->_fanin[i].getGate()->removed_fanout(tmp->getID());
  for(size_t i=0; i<tmp->_fanout.size(); ++i)
  {
    tmp->_fanout[i].getGate()->replace_fanin(tmp->getID(), next, inv ^ tmp->_fanout[i].isinv());
    next->add_fanout(tmp->_fanout[i].getID(), inv ^ tmp->_fanout[i].isinv());
  }
  unsigned int deletenum = tmp->ID;
  _AIGs.erase(std::find(_AIGs.begin(), _AIGs.end(), deletenum));
  delete _gates[deletenum];
  _gates[deletenum] = NULL;
}
