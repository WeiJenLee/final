/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cmath>
#include <sstream>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
  size_t simNum = 0, simSuccessNum = 0, maxFail = 0;
  RandomNumGen gen;
  vector<CirGate*> fec;
  FecGrp.clear();
  simulateCalled = true;
  for(size_t i=0; i <= gates_num[0]; ++i)
    if(_gates[i])
      if(_gates[i]->getTypeStr() == "CONST" || _gates[i]->getTypeStr() == "AIG")
        fec.push_back(_gates[i]);
  FecGrp.push_back(fec);

  for(size_t i=0; i<gates_num[1]; ++i)
  {
    _gates[_PIs[i]]->addValue(gen(2), simNum);
    _gates[_PIs[i]]->_visit = true;
  }
  if(_gates[0] && _gates[0]->getTypeStr() == "CONST")
    _gates[0]->_visit = true;
  for(size_t i=0; i<_POs.size(); ++i)
  {
    _gates[_POs[i]]->_visit = true;
    _gates[_POs[i]]->simulate(simNum);
  }
  for(size_t i=gates_num[1]; i<=gates_num[0]; ++i)
    if(_gates[i] && ((_gates[i]->value & 1) == 1))
    {
      _gates[i]->valueInv = true;
      _gates[i]->value = -1;
    }
  simNum++;
  resetVisit();
  while(simSuccessNum < 5 && maxFail < gates_num[4])
  {
    for(size_t i=0; i<gates_num[1]; ++i)
    {
      _gates[_PIs[i]]->addValue(gen(2), simNum);
      _gates[_PIs[i]]->_visit = true;
    }
    for(size_t i=0; i<_POs.size(); ++i)
    {
      _gates[_POs[i]]->_visit = true;
      _gates[_POs[i]]->simulate(simNum);
    }
    if(checkgrp())
      ++simSuccessNum;
    else
      ++maxFail;
    ++simNum;
    resetVisit();
  }
  cout << "MAX_FAIL = " << gates_num[4] << endl;
  cout << simNum << " pattern simulated.\n";
  return;
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  size_t simNum = 0;
  string data;
  vector<CirGate*> fec;
  FecGrp.clear();
  simulateCalled = true;
  for(size_t i=0; i <= gates_num[0]; ++i)
    if(_gates[i])
      if(_gates[i]->getTypeStr() == "CONST" || _gates[i]->getTypeStr() == "AIG")
        fec.push_back(_gates[i]);
  FecGrp.push_back(fec);
  patternFile >> data;
  if(data.length() != gates_num[1])
  {
    cout << "Error: Pattern(" << data << ") length(" << data.length()
         << ") does not match the number of inputs(" << gates_num[1]
         << ") in a circuit!!\n0 patterns simulated.\n";
    return;
  }
  for(size_t i=0; i<gates_num[1]; ++i)
  {
    if(data[i] != '0' && data[i] != '1')
    {
      cerr << "Error: Pattern(" << data << ") contains a non-0/1 character(\'"
           << data[i] << "\').\n0 patterns simulated.\n";
      return;
    }
    else
    {
      if(data[i] == '0')
        _gates[_PIs[i]]->addValue(0, simNum);
      else
        _gates[_PIs[i]]->addValue(1, simNum);
      _gates[_PIs[i]]->_visit = true;
    }
  }
  if(_gates[0] && _gates[0]->getTypeStr() == "CONST")
    _gates[0]->_visit = true;
  for(size_t i=0; i<_POs.size(); ++i)
  {
    _gates[_POs[i]]->_visit = true;
    _gates[_POs[i]]->simulate(simNum);
  }
  for(size_t i=gates_num[1]; i<=gates_num[0]; ++i)
    if(_gates[i] && ((_gates[i]->value & 1) == 1))
    {
      _gates[i]->valueInv = true;
      _gates[i]->value = -1;
    }
  simNum++;
  resetVisit();
  while(patternFile >> data)
  {
    if(data.length() != gates_num[1])
    {
      cerr << "Error: Pattern(" << data << ") length(" << data.length()
           << ") does not match the number of inputs(" << gates_num[1]
           << ") in a circuit!!\n0 patterns simulated.\n";
      return;
    }
    for(size_t i=0; i<gates_num[1]; ++i)
    {
      if(data[i] != '0' && data[i] != '1')
      {
        cerr << "Error: Pattern(" << data << ") contains a non-0/1 character(\'"
             << data[i] << "\').\n0 patterns simulated.\n";
        return;
      }
      else
      {
        if(data[i] == '0')
          _gates[_PIs[i]]->addValue(0, simNum);
        else
          _gates[_PIs[i]]->addValue(1, simNum);
        _gates[_PIs[i]]->_visit = true;
      }
    }
    for(size_t i=0; i<_POs.size(); ++i)
    {
      _gates[_POs[i]]->_visit = true;
      _gates[_POs[i]]->simulate(simNum);
    }
    ++simNum;
    checkgrp();
    resetVisit();
  }
  cout << simNum << " pattern simulated.\n";
  return;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
bool
CirMgr::checkgrp()
{
  size_t row = FecGrp.size();
  vector<CirGate*> newGrp;
  for(size_t i=0; i<row; ++i)
  {
    newGrp.clear();
    for(size_t j=1; j<FecGrp[i].size(); ++j)
      if(FecGrp[i][j]->value != FecGrp[i][0]->value && FecGrp[i][j]->value != ~(FecGrp[i][0]->value))
      {
        newGrp.push_back(FecGrp[i][j]);
        FecGrp[i][j]->Grp = FecGrp.size();
        FecGrp[i].erase(FecGrp[i].begin()+j);
        --j;
      }
    if(!newGrp.empty())
      FecGrp.push_back(newGrp);
  }
  if(row == FecGrp.size())
    return true;
  else
    return false;
}
