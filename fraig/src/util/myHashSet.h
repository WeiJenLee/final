/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data> class HashSet;
template <class Data>

class HashNode
{
  friend class HashSet<Data>;
  friend class HashSet<Data>::iterator;

  HashNode(const Data& d, HashNode* _prev = NULL, HashNode* _next = NULL)
  : _data(d), prev(_prev), next(_next) {}

  Data _data;
  HashNode* prev;
  HashNode* next;
};

template <class Data>
class HashSet
{
public:
  HashSet() : _numBuckets(0), _buckets(0)
  {
    endNode = new HashNode<Data>(Data("", 0));
    endNode->prev = endNode;
  }
  HashSet(size_t b = 0) : _numBuckets(0), _buckets(0)
  {
    if (b != 0)
      init(b);
    endNode = new HashNode<Data>(Data("", 0));
    endNode->prev = endNode;
  }
  ~HashSet() { reset(); }
   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;
    public:
      iterator(HashNode<Data>* n): _node(n) {}
      ~iterator(){}

      const Data& operator*() const{ return this->_node->_data;}
      Data operator*() {return this->_node->_data;}

      const iterator& operator++()
      {
        this->_node = this->_node->next;
        return *this;
      }
      iterator operator++(int)
      {
        iterator tmp = *this;
        this->_node = this->_node->next;
        return tmp;
      }

      const iterator& operator--()
      {
        this->_node = this->_node->prev;
        return *this;
      }
      iterator operator--(int)
      {
        iterator tmp = *this;
        this->_node = this->_node->prev;
        return tmp;
      }

      iterator& operator=(const iterator& i)
      {
        this->_node = i._node;
        return this;
      }

      bool operator!=(const iterator& i) const {return !(this->_node->_data == i._node->_data);}
      bool operator==(const iterator& i) const {return this->_node->_data == i._node->_data;}

   private:
     HashNode<Data>* _node;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<HashNode<Data>* >[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const
   {
     for(size_t i=0; i<_numBuckets; ++i)
       if(!(_buckets[i].empty()))
         return iterator(_buckets[i][0]);
     return iterator(endNode);
   }
   // Pass the end
   iterator end() const { return iterator(endNode);}
   // return true if no valid data
   bool empty() const { return endNode->prev == endNode; }
   // number of valid data
   size_t size() const
   {
     size_t count;
     for(iterator it = begin(); it != end(); ++it)
       ++count;
     return count;
   }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const
   {
     size_t bucket = bucketNum(d);
     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i]->_data == d)
         return true;
     return false;
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const
   {
     size_t bucket = bucketNum(d);
     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i]->_data == d)
       {
         d = _buckets[bucket][i]->_data;
         return true;
       }
     return false;
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d)
   {
     size_t bucket = bucketNum(d);
     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i]->_data == d)
       {
         _buckets[bucket][i]->_data = d;
         return true;
       }
     insert(d);
     return false;
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d)
   {
     size_t bucket = bucketNum(d);
     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i]->_data == d)
         return false;

     HashNode<Data>* tmp = new HashNode<Data>(d);
     if(_buckets[bucket].empty())
     {
       tmp->prev = getPrev(bucket);
       if(tmp->prev)
         tmp->prev->next = tmp;
       tmp->next = getNext(bucket);
       tmp->next->prev = tmp;
     }
     else
     {
       tmp->prev = _buckets[bucket].back();
       tmp->next = _buckets[bucket].back()->next;
       _buckets[bucket].back()->next = tmp;
       tmp->next->prev = tmp;
     }

     _buckets[bucket].push_back(tmp);
     return true;
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d)
   {
     size_t bucket = bucketNum(d);

     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i]->_data == d)
       {
         if(_buckets[bucket][i]->prev)
           _buckets[bucket][i]->prev->next = _buckets[bucket][i]->next;
         if(_buckets[bucket][i]->next)
           _buckets[bucket][i]->next->prev = _buckets[bucket][i]->prev;
         _buckets[bucket].erase(_buckets[bucket].begin()+i);
         return true;
       }
     return false;
   }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector< HashNode<Data>* >*     _buckets;
   HashNode<Data>*    endNode;

   size_t bucketNum(const Data& d) const { return (d() % _numBuckets); }

   HashNode<Data>* getPrev(size_t i) const
   {
     for(size_t count = 0; count<=i; ++count)
       if(!(_buckets[i-count].empty()))
         return _buckets[i-count].back();
     return NULL;
   }

   HashNode<Data>* getNext(size_t i) const
   {
     for(i; i<_numBuckets; ++i)
       if(!(_buckets[i].empty()))
         return _buckets[i].front();
     return endNode;
   }
};

#endif // MY_HASH_SET_H
