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
  if(FecGrp.size() == 0)
    return;
  SatSolver solver;
  solver.initialize();

  for(size_t i=0; i<=gates_num[0]; ++i)
    _gates[i]->satID = solver.newVar();
  for(size_t i=gates_num[0]+1; i<_gates.size(); ++i)
    _gates[i]->satID = _gates[i]->_fanin[0].getSATID();

  for(size_t i=0; i<_AIGs.size(); ++i)
    if(!(_gates[_AIGs[i]]->_visit))
    {
      _gates[_AIGs[i]]->_visit = true;
      genProofModel(solver, _gates[_AIGs[i]]);
    }
  resetVisit();
  bool result;
  for(size_t i=0; i<_gates.size(); ++i)
    if(_gates[i] && FecGrp[_gates[i]->Grp].size() > 1)
      trymerge(solver, _gates[i]);
  size_t count = 0;
  for(size_t i=0; i<FecGrp.size(); ++i)
    if(FecGrp[i].size() > 1)
      ++count;
  cout << "Updating by UNSAT... Total #FEC Group = " << count << endl;
  strashCalled = simulateCalled = false;
  DFSsort();
}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void
CirMgr::genProofModel(SatSolver& s, CirGate* tmp)
{
  if(tmp->getTypeStr() == "PI" || tmp->getTypeStr() == "CONST")
    return;
  for(size_t i=0; i<tmp->_fanin.size(); ++i)
  {
    CirGate* next = (tmp->_fanin[i]).getGate();
    if(next && !(next->_visit))
    {
        next->_visit = true;
        genProofModel(s, next);
    }
  }
  if(tmp->getTypeStr() == "AIG")
    s.addAigCNF(tmp->satID, tmp->_fanin[0].getSATID(), tmp->_fanin[0].isinv(),
                tmp->_fanin[1].getSATID(), tmp->_fanin[1].isinv());

}

void
CirMgr::FecReplace(CirGate* fir, CirGate* sec)
{
  bool inv = (fir->value != sec->value);
  if(sec->getTypeStr() == "CONST")
  {
    CirGate* tmp = fir;
    fir = sec;
    sec = tmp;
  }
  cout << "Fraig: " << fir->ID << " merging " << (inv ? "!" : "") << sec->ID << "...\n";
  for(size_t i=0; i<sec->_fanin.size(); ++i)
    sec->_fanin[i].getGate()->replace_fanout(sec->getID(), fir);
  for(size_t i=0; i<sec->_fanout.size(); ++i)
  {
    sec->_fanout[i].getGate()->replace_fanin(sec->getID(), fir, inv != sec->_fanout[i].isinv());
    fir->add_fanout(sec->_fanout[i].getID(), inv != sec->_fanout[i].isinv());
  }
  unsigned int deletenum = sec->ID;
  _AIGs.erase(std::find(_AIGs.begin(), _AIGs.end(), deletenum));
  delete _gates[deletenum];
  _gates[deletenum] = NULL;
}

void
CirMgr::trymerge(SatSolver& solver, CirGate* g)
{
  for(size_t i=0; i<FecGrp[g->Grp].size(); ++i)
    if(FecGrp[g->Grp][i] != g)
    {
      Var newV = solver.newVar();
      solver.addXorCNF(newV, g->satID, false, FecGrp[g->Grp][i]->satID,
                       g->value == FecGrp[g->Grp][i]->value);
      solver.assumeRelease();
      solver.assumeProperty(newV, true);
      if(solver.assumpSolve())
      {
        FecReplace(g, FecGrp[g->Grp][i]);
        FecGrp[g->Grp].erase(FecGrp[g->Grp].begin()+i);
        --i;
      }
    }
}
