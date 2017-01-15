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
  size_t simNum = 0;
  size_t simSuccessNum = 0;
  vector<CirGate*> fec;
  FecGrp.clear();
  for(size_t i=0; i<gates_num[0]; ++i)
    if(_gates[i])
      if(_gates[i]->getTypeStr() == "CONST" || _gates[i]->getTypeStr() == "AIG")
        fec.push_back(_gates[i]);
  FecGrp.push_back(fec);
  while(simSuccessNum < 5 && simNum < gates_num[0]+gates_num[3])
  {
    ++simNum;
    resetSim();
    RandomNumGen gen;
    for(size_t i=0; i<gates_num[1]; ++i)
      _gates[_PIs[i]]->value = gen(2);
    for(size_t i=0; i<_POs.size(); ++i)
      _gates[_POs[i]]->simulate();
    if(checkgrp())
      ++simSuccessNum;
  }
  cout << FecGrp.size();
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
  for(size_t i=0; i<gates_num[0]; ++i)
    if(_gates[i])
      if(_gates[i]->getTypeStr() == "CONST" || _gates[i]->getTypeStr() == "AIG")
        fec.push_back(_gates[i]);
  FecGrp.push_back(fec);
  while(patternFile >> data)
  {
    resetSim();
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
        _gates[_PIs[i]]->value = (int)data[i];
    }
    ++simNum;
    for(size_t i=0; i<_POs.size(); ++i)
      _gates[_POs[i]]->simulate();
    checkgrp();
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
  for(size_t i=0; i<row; ++i)
  {
    vector<CirGate*> newGrp;
    for(size_t j=1; j<FecGrp[i].size(); ++j)
      if(FecGrp[i][j]->value != FecGrp[i][0]->value)
      {
        newGrp.push_back(FecGrp[i][j]);
        FecGrp[i].erase(FecGrp[i].begin()+j);
        --j;
      }
    if(newGrp.size() > 0)
      FecGrp.push_back(newGrp);
  }
  if(row == FecGrp.size())
    return true;
  else
    return false;
}
