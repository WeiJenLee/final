/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;

class CirMgr
{
public:
   CirMgr()
   {
      for(size_t i=0; i<5; ++i)
        gates_num[i] = 0;
   }
   ~CirMgr()
   {
      if(_gates.size() == 0)
        return;
      for(size_t i=0; i<gates_num[0]+gates_num[3]+1; ++i)
        if(getGate(i)){
          delete _gates[i];
          _gates[i] = NULL;
        }
      _gates.clear();
   }

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const
   {
      if(gid <= gates_num[0]+gates_num[3] || gid > -1)
        if(_gates[gid])
          return _gates[gid];
        else
          return NULL;
      else
        return NULL;
   }

   // Member functions about circuit construction
   void DFSsort();
   bool readCircuit(const string&);

   // Member functions about circuit optimization
   void sweep();
   void optimize();

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }

   // Member functions about fraig
   void strash();
   void printFEC() const;
   void fraig();

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;
   void resetVisit() const;

private:
   ofstream           *_simLog;
   bool printCmd;
   int gates_num[5];
   GateList _gates, dfsorder;
   IdList _PIs, _POs, _AIGs;
   void DFS(CirGate*);
   void DFSopt(CirGate*);
   void replacegate(CirGate*, CirGate*, bool);
};

#endif // CIR_MGR_H
