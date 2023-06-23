#ifndef __GD_GD_TYPES_HPP__
#define __GD_GD_TYPES_HPP__

#include <cinttypes>
#include <iterator>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <stack>
#include <queue>

#include <cstring>

namespace gd
{
  typedef std::pair<size_t, size_t> fuint32_pair_t;
  typedef size_t vertex_t;

  const static size_t UINT_UNDEF = (size_t)-1;
  const static vertex_t VERTEX_UNDEF = (size_t)-1;

  template<typename T, typename Allocator = std::allocator<T>>
  using Vector = std::vector<T, Allocator>;

  template<class T, typename HashFunc = std::hash<T>, typename Allocator = std::allocator<T>>
  using UnorderedSet = std::unordered_set<T, HashFunc, std::equal_to<T>, Allocator>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMap = std::unordered_map<K, V, HashFunc, std::equal_to<K>, Allocator>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMultiMap = std::unordered_multimap<K, V, HashFunc, std::equal_to<K>, Allocator>;

  template<typename T, typename Comparator = std::less<T>>
  using PriorityQueue = std::priority_queue<T, Vector<T>, Comparator>;

  template<typename T>
  using Queue = std::queue<T>;

  template<typename T>
  using Stack = std::stack<T, std::vector<T>>;

  typedef struct Edge
  {
    Edge(): u(VERTEX_UNDEF), v(VERTEX_UNDEF) {}
    Edge(vertex_t u_vertex, vertex_t v_vertex)
    { // directly order edges (s. t. u <= v)
      if (u_vertex <= v_vertex) { u = u_vertex; v = v_vertex; }
      else { u = v_vertex; v = u_vertex; }
    }

    vertex_t u;
    vertex_t v;
  } edge_t;

  template<typename K, typename V = K>
  struct PairHashFunc
  {
    size_t operator()(const std::pair<K,V>& pair) const
    {
      size_t first = std::hash<K>()(pair.first);
      size_t second = std::hash<V>()(pair.second);
      return first ^ (second << 32) ^ (second >> 32);
    }
  };


}



#endif
