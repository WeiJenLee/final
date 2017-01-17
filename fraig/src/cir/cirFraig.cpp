/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

class faninpin
{
public:
  faninpin(CirGate* gate)
  {
    this->_fanin[0] = gate->getfanin(0);
    this->_fanin[1] = gate->getfanin(1);
  }
  ~faninpin() {}

  faninpin& operator=(const faninpin& pin)
  {
    this->_fanin[0] = pin._fanin[0];
    this->_fanin[1] = pin._fanin[1];
    return *this;
  }
  size_t operator() () const
  {
    return _fanin[0].getID()+_fanin[1].getID()+(_fanin[0].getID()%256)*(_fanin[1].getID()%256);
  }
  bool operator==(const faninpin& refpin) const
  {
    return (_fanin[0] == refpin._fanin[0] && _fanin[1] == refpin._fanin[1]) ||
           (_fanin[0] == refpin._fanin[1] && _fanin[1] == refpin._fanin[0]);
  }
private:
  pin _fanin[2];
};

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  if(strashCalled)
  {
    cerr << "Error: strash operation has been performed!!\n";
    return;
  }
  HashMap<faninpin, CirGate*>* hash = new HashMap<faninpin, CirGate*>(gates_num[0] + gates_num[3]);
  for(size_t i=0; i<dfsorder.size(); ++i)
    if(dfsorder[i]->getTypeStr() == "AIG")
    {
      faninpin key(dfsorder[i]);
      CirGate* tmp = dfsorder[i];
      if(hash->query(key, tmp))
      {
        cout << "Strashing: " << tmp->getID() << " merging " << dfsorder[i]->getID() << "...\n";
        for(size_t j=0; j<dfsorder[i]->_fanin.size(); ++j)
          dfsorder[i]->_fanin[j].getGate()->removed_fanout(dfsorder[i]->getID());
        for(size_t j=0; j<dfsorder[i]->_fanout.size(); ++j)
        {
          dfsorder[i]->_fanout[j].getGate()->replace_fanin(dfsorder[i]->getID(), tmp, dfsorder[i]->_fanout[j].isinv());
          tmp->add_fanout(dfsorder[i]->_fanout[j]);
        }
        unsigned int deletenum = dfsorder[i]->getID();
        _AIGs.erase(std::find(_AIGs.begin(), _AIGs.end(), deletenum));
        delete _gates[deletenum];
        _gates[deletenum] = NULL;
      }
      else
        hash->insert(key, tmp);
    }
  delete hash;
  DFSsort();
  strashCalled = true;
}

void
CirMgr::fraig()
{
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
