/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include <queue>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include "cmdParser.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;
static bool isinvert(unsigned int number) {return number/2 != (number+1)/2;}

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
   unsigned int pin[3] = {0};
   ifstream cir_file(fileName, ifstream::in);
   if(cir_file.fail())
   {
     cerr << "File: " << fileName << " can not be opened!\n";
     return false;
   }

   if(!cir_file.getline(buf, 1024, '\n'))
     return parseError(ILLEGAL_IDENTIFIER);

   colNo = myStrGetTok(buf, errMsg, colNo);

   for(size_t i=0; i<5; ++i)
   {
     colNo = myStrGetTok(buf, errMsg, colNo);
     if(!myStr2Int(errMsg, gates_num[i]))
       return parseError(ILLEGAL_NUM);
   }
   _gates.resize(gates_num[0]+gates_num[3]+1);
   colNo = 0;
   lineNo++;

   for(size_t i=0; i<gates_num[1]; ++i)
   {
     if(!cir_file.getline(buf, 1024, '\n'))
       return parseError(MISSING_NEWLINE);
     if(!myStr2Int(buf, errInt))
     {
       errMsg = buf;
       return parseError(ILLEGAL_NUM);
     }

     _gates[errInt/2] = new CirInGate(errInt, lineNo);
     _PIs.push_back(errInt/2);
     lineNo++;
   }

   for(size_t count = gates_num[0]+1; count <= gates_num[0]+gates_num[3]; ++count)
   {
     if(!cir_file.getline(buf, 1024, '\n'))
       return parseError(MISSING_NEWLINE);
     if(!myStr2Int(buf, errInt))
     {
       errMsg = buf;
       return parseError(ILLEGAL_NUM);
     }
     _gates[count] = new CirOutGate(count, lineNo);
     _POs.push_back(count);
     ++lineNo;
   }
   for(size_t i=0; i<gates_num[4]; ++i)
   {
     if(!cir_file.getline(buf, 1024, '\n'))
       return parseError(MISSING_NEWLINE);

     for(size_t j=0; j<3; ++j)
     {
       colNo = myStrGetTok(buf, errMsg, colNo);
       if(!myStr2Int(errMsg, errInt))
         return parseError(ILLEGAL_NUM);
       pin[j] = errInt;
       if(errInt/2 == 0)
         if(!_gates[0])
           _gates[0] = new CirConstGate();
     }
     colNo = 0;
     _gates[pin[0]/2] = new CirAndGate(pin[0], lineNo);
     _AIGs.push_back(pin[0]/2);
     lineNo++;
   }
   int id;
   while(cir_file.getline(buf, 1024, '\n'))
   {
     if(myStrNCmp("c", buf, 1) == 0)
     {
       printCmd = true;
       break;
     }
     colNo = myStrGetTok(buf, errMsg, colNo);
     if(errMsg.length()>1)
       errMsg = errMsg.substr(1);
     if(myStrNCmp(buf, "i", 1) == 0)
     {
       if(!myStr2Int(errMsg, id))
         return parseError(ILLEGAL_SYMBOL_NAME);
       errMsg.assign(buf);
       errMsg.erase(errMsg.begin(), errMsg.begin()+colNo+1);
       _gates[_PIs[id]]->symbol = errMsg;
     }
     else if(myStrNCmp(buf, "o", 1) == 0)
     {
       if(!myStr2Int(errMsg, id))
         return parseError(ILLEGAL_SYMBOL_NAME);
       errMsg.assign(buf);
       errMsg.erase(errMsg.begin(), errMsg.begin()+colNo+1);
       _gates[_POs[id]]->symbol = errMsg;
     }
     lineNo++;
     colNo = 0;
   }

   cir_file.clear();
   cir_file.seekg(0, ios::beg);
   for(size_t i=0; i<=gates_num[1]; ++i)
     cir_file.getline(buf, 1024, '\n');

   for(size_t count = gates_num[0]+1; count <= gates_num[0]+gates_num[3]; ++count)
   {
     cir_file.getline(buf, 1024, '\n');

     if(!myStr2Int(buf, errInt))
     {
       errMsg = buf;
       return parseError(ILLEGAL_NUM);
     }
     if(_gates[errInt/2])
       _gates[errInt/2]->_Flt = false;
     else
     {
       if(errInt == 0)
         _gates[0] = new CirConstGate();
       else
         _gates[errInt/2] = new CirUndefGate(errInt);
     }
     _gates[count]->add_fanin(errInt);
     _gates[errInt/2]->add_fanout(count, isinvert(errInt));
   }

   for(size_t i=0; i<_AIGs.size(); ++i)
   {
     cir_file.getline(buf, 1024, '\n');
     for(size_t j=0; j<3; ++j)
     {
       colNo = myStrGetTok(buf, errMsg, colNo);
       if(!myStr2Int(errMsg, errInt))
         return parseError(ILLEGAL_NUM);
       pin[j] = errInt;
     }
     colNo = 0;
     if(!_gates[pin[1]/2])
       _gates[pin[1]/2] = new CirUndefGate(pin[1]/2);
     else
       _gates[pin[1]/2]->_Flt = false;
     if(!_gates[pin[2]/2])
       _gates[pin[2]/2] = new CirUndefGate(pin[2]/2);
     else
       _gates[pin[2]/2]->_Flt = false;
     _gates[pin[0]/2]->add_fanin(pin[1]);
     _gates[pin[0]/2]->add_fanin(pin[2]);
     _gates[pin[1]/2]->add_fanout(pin[0]/2, isinvert(pin[1]));
     _gates[pin[2]/2]->add_fanout(pin[0]/2, isinvert(pin[2]));
   }

   DFSsort();
   strashCalled = false;
   return true;
}

/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/

void
CirMgr::DFSsort()
{
  dfsorder.clear();
   for(size_t i=0; i<_POs.size(); ++i)
     if(!(_gates[_POs[i]]->_visit))
     {
       _gates[_POs[i]]->_visit = true;
       DFS(_gates[_POs[i]]);
     }
   resetVisit();
}

void
CirMgr::printSummary() const
{
   cout << "Circuit Statistics\n==================\n"
        << "  PI\t\t" << _PIs.size() << endl
        << "  PO\t\t" << _POs.size() << endl
        << "  AIG\t\t" << _AIGs.size() << endl
        << "------------------\n  Total\t\t" << _PIs.size() + _POs.size() + _AIGs.size() << endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for (unsigned i = 0, n = 0; i < dfsorder.size();++n,  ++i)
   {
      if(dfsorder[i]->getTypeStr() != "UNDEF")
      {
        cout << "[" << n << "] ";
        dfsorder[i]->printGate();
      }
      else
        --n;
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit: ";
   for(size_t i=0; i<_PIs.size(); ++i)
     cout << _PIs[i] << " ";
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit: ";
   for(size_t i=0; i<_POs.size(); ++i)
     cout << _POs[i] << " ";
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   cout << "Floats of the circuit: ";
   for(size_t i=0; i<gates_num[0]+gates_num[3]+1; ++i)
     if(_gates[i] && _gates[i]->_Flt)
       cout << i << " ";
   cout << endl;
}

void
CirMgr::printFECPairs() const
{
  bool firstinv = false;
  for(size_t i=0, count = 0; i<FecGrp.size(); ++i)
    if(FecGrp[i].size() > 1)
    {
      if(FecGrp[i][0]->valueInv)
        firstinv = true;
      cout << "[" << count << "] ";
      for(size_t j=0; j<FecGrp[i].size(); ++j)
        cout << (firstinv != FecGrp[i][j]->valueInv? "!":"") << FecGrp[i][j]->ID << " ";
      cout << endl;
      ++count;
      firstinv = false;
    }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   CirGate* tmp;
   outfile << "aag ";
   for(size_t i=0; i<4; ++i)
     cout << gates_num[i] << " ";
   cout << _AIGs.size() << endl;

   for(size_t i=0; i<_PIs.size(); ++i)
     outfile << 2*_PIs[i] << endl;
   for(size_t i=0; i<_POs.size(); ++i)
   {
     unsigned int output = _gates[_POs[i]]->_fanin[0].getID();
     if(_gates[_POs[i]]->_fanin[0].isinv())
       outfile << 2*output+1 << endl;
     else
       outfile << 2*output << endl;
   }
   for(size_t i=0; i<_AIGs.size(); ++i)
   {
     outfile << 2*_AIGs[i] << " ";
     tmp = _gates[_AIGs[i]];
     unsigned int input_1 = tmp->_fanin[0].getID();
     if(tmp->_fanin[0].isinv())
       outfile << 2*input_1+1 << " ";
     else
       outfile << 2*input_1 << " ";
     unsigned int input_2 = tmp->_fanin[1].getID();
     if(tmp->_fanin[1].isinv())
       outfile << 2*input_2+1 << endl;
     else
       outfile << 2*input_2 << endl;
   }
   cout << "c\nAAG is output by Wei-Jen(Wen) Lee\n";
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
  g->_visit = true;
  writeDFS(g);
  size_t in = 0, a = 0;
  for(size_t j=0; j<_PIs.size(); ++j)
    if(_gates[_PIs[j]]->_visit)
      ++in;
  for(size_t j=0; j<_AIGs.size(); ++j)
    if(_gates[_AIGs[j]]->_visit)
      ++a;
  outfile << "aag " << ((a+in) >= g->ID ? a+in:g->ID) << " " << in << " " << gates_num[2]
          << " 1 " << a << endl;

  for(size_t i=0; i<_PIs.size(); ++i)
    if(_gates[_PIs[i]]->_visit)
      outfile << 2*_PIs[i] << endl;
  outfile << 2*(g->ID) << endl;
  CirGate* tmp;
  for(size_t i=0; i<_AIGs.size(); ++i)
    if(_gates[_AIGs[i]]->_visit)
    {
      outfile << 2*_AIGs[i] << " ";
      tmp = _gates[_AIGs[i]];
      unsigned int input_1 = tmp->_fanin[0].getID();
      if(tmp->_fanin[0].isinv())
        outfile << 2*input_1+1 << " ";
      else
        outfile << 2*input_1 << " ";
      unsigned int input_2 = tmp->_fanin[1].getID();
      if(tmp->_fanin[1].isinv())
        outfile << 2*input_2+1 << endl;
      else
        outfile << 2*input_2 << endl;
      }
  for(size_t i=0; i<_PIs.size(); ++i)
    if(_gates[_PIs[i]]->_visit)
      if(_gates[_PIs[i]]->symbol.length() > 0)
        outfile << "i" << i << " " << _gates[_PIs[i]]->symbol << endl;
  outfile << "o0 " << g->ID << endl;
  if(printCmd)
    outfile << "c\nWrite gate (" << g->ID << ") by Wei-Jen (Wen) Lee\n";
  resetVisit();
}

void
CirMgr::DFS(CirGate* tmp)
{
   for(size_t i=0; i<tmp->_fanin.size(); ++i)
   {
     CirGate* next = (tmp->_fanin[i]).getGate();
     if(next && !(next->_visit))
     {
         next->_visit = true;
         DFS(next);
     }
   }
   dfsorder.push_back(tmp);
   tmp->needSweep = false;
}

void
CirMgr::resetVisit() const
{
   for(size_t i=0; i<gates_num[0]+gates_num[3]+1; ++i)
     if(_gates[i])
       _gates[i]->_visit = false;
}

void
CirMgr::writeDFS(CirGate* tmp) const
{
  for(size_t i=0; i<tmp->_fanin.size(); ++i)
  {
    CirGate* next = (tmp->_fanin[i]).getGate();
    if(next && !(next->_visit))
    {
        next->_visit = true;
        writeDFS(next);
    }
  }
}
