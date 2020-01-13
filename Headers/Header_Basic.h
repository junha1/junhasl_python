#pragma once


#include <vector>
#include <list>
#include <queue>
#include <set>
#include <map>
#include <array>
#include <unordered_map> 
#include <unordered_set> 
using std::vector;
using std::array;
using std::list;
using std::queue;
using std::set;
using std::map;
template <class K, class V>
using umap = std::unordered_map<K, V>;
template <class K>
using uset = std::unordered_set<K>;

#include <tuple>
using std::pair;
using std::tuple;
using std::make_pair;
using std::make_tuple;
using std::get;

#include <string>
#include <iostream>
using std::wstring;
using std::string;
using std::endl;

#include <thread>
#include <mutex>
using std::thread;
using std::mutex;
typedef std::recursive_mutex rmutex;
using std::unique_lock;
typedef unique_lock<mutex> ulock;
typedef unique_lock<rmutex> rlock;

#include <future>
#include <atomic>
using std::async;
using std::future;
using std::promise;
using std::atomic;

#include <memory>
using std::shared_ptr;
using std::unique_ptr;
using std::weak_ptr;
using std::dynamic_pointer_cast;
using std::make_shared;
using std::make_unique;

/*
#include <experimental/propagate_const>
template <class K>
using pconst = std::experimental::propagate_const<K>*/

#include <algorithm>
#include <fstream>
#include <sstream>
using std::ostringstream;
using std::istringstream;
using std::ofstream;
using std::ifstream;

#include <functional>
using std::function;
using std::ref; using std::cref;

#include <optional>
#include <any>
#include <variant>
using std::optional;
using std::any;
using std::variant;

#include <type_traits>
using std::enable_if_t;
using std::is_same_v;

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef long long int bint;
typedef unsigned long long int ubint;

typedef vector<uchar> bytes;

#pragma execution_character_set("utf-8")

#pragma warning (disable : 4267)
#pragma warning (disable : 4702) // not accesible code
#pragma warning (disable : 4505) // not used function


#define GLOBAL_DEC(t, x) const t & x(void)
#define GLOBAL_DEF(t, x, y) static const t & x(void) {static t x = y; return x;}

template <typename t>
static vector<t> LIST(std::initializer_list<t> a)
{
	return vector<t>(a);
}



#define SINGLETON(x) public: x(x const&) = delete; void operator=(x const&) = delete; static x* getInstance(){static x instance; return &instance;} private: 
#define MULTITON(x) public:  x(x const&) = delete; void operator=(x const&) = delete;\
static x* getInstance(int key) {static umap<int, shared_ptr<x>> instances;\
if (instances.count(key)) return instances.at(key).get();\
else {auto p = new x(); instances.emplace(key, p); return p;}} private: 
#define NO_COPY(x) public: x(x const&) = delete; void operator=(x const&) = delete; private:
#define MEM_HASH(x) public : bool operator<(x const& p) const {return memcmp(this, &p, sizeof(x)) < 0;} private:



