/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: (Optionally) Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
/*
class HashKey
{
public:
   HashKey(CirGate* tmp):
   {
     //this->_fanin[0] = tmp->_fanin[0].connectGate->ID;
     //this->_fanin[1] = tmp->_fanin[1];
   }
   HashKey() {_fanin[0] = _fanin[1] = 0;}
   ~HashKey(){}
   HashKey& operator=(const HashKey& tmp)
   {
     this->_fanin[0] = tmp->_fanin[0];
     this->_fanin[1] = tmp->_fanin[1];
     return this;
   }
   size_t operator() () const
   {

     return 0;
   }

   bool operator == (const HashKey& k) const { return true; }

private:
  pin _fanin[2];
};
*/
template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(b), _buckets(0)
   {
     if (b != 0)
       init(b);
     _endBucket = _buckets[_numBuckets-1];
   }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey, HashData>;

   public:
     iterator(): map(NULL), _curBucket(NULL) {}
     iterator(const HashMap* newMap, vector<HashNode>* _newBucket, typename vector<HashNode>::iterator newit)
     : map(newMap), _curBucket(_newBucket), it(newit) {}
     ~iterator(){}

     const HashNode& operator*() const {return *map;}
     HashNode& operator*() {return *map;}

     iterator& operator++()
     {
       if(it+1 == _curBucket->end())
       {
         if((_curBucket = map->getNextBucket(_curBucket)) != NULL)
           it = _curBucket->begin();
         else
         {
           this->_curBucket = map->end()._curBucket;
           this->it = map->end().it;
         }
       }
       else
         it++;
       return *this;
     }
     iterator operator++(int)
     {
       iterator tmp = *this;
       this->operator++();
       return tmp;
     }

     iterator& operator--()
     {
       if(it == _curBucket->begin())
         if((_curBucket = map->getPrevBucket(_curBucket)) != NULL)
           it = _curBucket->end()-1;
         else
         {
           this->_curBucket = map->begin()._curBucket;
           this->it = map->begin().it;
         }
       else
         it++;
       return *this;
     }
     iterator operator--(int)
     {
       iterator tmp = *this;
       this->operator--();
       return tmp;
     }

     iterator& operator=(const iterator& i)
     {
       this->map = i.map;
       this->_curBucket = i._curBucket;
       this->it = i.it;
     }

     bool operator != (const iterator& i) const {return this->it != i.it;}
     bool operator == (const iterator& i) const {return this->it == i.it;}

   private:
     const HashMap<HashKey, HashData>* map;
     vector<HashNode>* _curBucket;
     typename vector<HashNode>::iterator it;
   };

   void init(size_t b) {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const
   {
     for(size_t i=0; i<_numBuckets; ++i)
       if(!(_buckets[i].empty()))
         return iterator(_buckets[i][0]);
     return iterator(this, _endBucket, _endBucket->end());
   }
   // Pass the end
   iterator end() const { return iterator(this, _endBucket, _endBucket->end()); }
   // return true if no valid data
   bool empty() const {return begin() == end();}
   // number of valid data
   size_t size() const
   {
     size_t count;
     for(iterator it = begin(); it != end(); ++it)
       ++count;
     return count;
   }

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k) const
   {
     size_t bucket = bucketNum(k);
     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i].first == k)
         return true;
     return false;
   }

   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const
   {
     size_t bucket = bucketNum(k);
     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i].first == k)
       {
         d = _buckets[bucket][i].second;
         return true;
       }
     return false;
   }

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d)
   {
     size_t bucket = bucketNum(k);
     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i]->first == k)
       {
         _buckets[bucket][i]->second = d;
         return true;
       }
     insert(d);
     return false;
   }

   // return true if inserted d successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d)
   {
     size_t bucket = bucketNum(k);
     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i].first == k)
         return false;

     HashNode tmp(k, d);
     _buckets[bucket].push_back(tmp);
     return true;
   }

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k)
   {
     size_t bucket = bucketNum(k);
     for(size_t i=0; i<_buckets[bucket].size(); ++i)
       if(_buckets[bucket][i]->first == k)
       {
         _buckets[bucket].erase(_buckets[bucket].begin()+i);
         return true;
       }
     return false;
   }

private:
   // Do not add any extra data member
   size_t                         _numBuckets;
   vector<HashNode>*              _buckets, _endBucket;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

   vector<HashNode>* getNextBucket(vector<HashNode>* tmp) const
   {
     if(tmp == _endBucket)
       return NULL;
     for(vector<HashNode>* i = tmp+1; tmp < _buckets+_numBuckets; ++i)
       if(!(i->empty()))
         return i;
     return NULL;
   }

   vector<HashNode>* getPrevBucket(vector<HashNode>* tmp) const
   {
     if(tmp == _buckets)
       return NULL;
     for(vector<HashNode>* i = tmp-1; tmp >= _buckets; --i)
       if(!(i->empty()))
         return i;
     return NULL;
   }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//
//    size_t operator() () const { return 0; }
//
//    bool operator == (const CacheKey&) const { return true; }
//
// private:
// };
//
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};


#endif // MY_HASH_H
