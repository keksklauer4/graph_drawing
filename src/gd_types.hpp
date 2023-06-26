#ifndef __GD_GD_TYPES_HPP__
#define __GD_GD_TYPES_HPP__

#include <cinttypes>
#include <cstddef>
#include <iterator>
#include <limits>
#include <ostream>
#include <utility>
#include <vector>
#include <list>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <stack>
#include <queue>
#include <cassert>

#include <cstring>

namespace gd
{
  typedef int64_t coordinate_t;
  typedef std::pair<coordinate_t,coordinate_t> coordinate_2d_t;
  typedef std::pair<coordinate_2d_t,coordinate_2d_t> line_2d_t;
  typedef std::pair<size_t, size_t> size_pair_t;
  typedef size_pair_t vertex_pair_t;
  typedef size_t vertex_t;
  typedef size_t point_id_t;

  const static size_t UINT_UNDEF = (size_t)-1;
  const static vertex_t VERTEX_UNDEF = (size_t)-1;
  const static vertex_t POINT_UNDEF = (size_t)-1;
  const static double DOUBLE_MAX = std::numeric_limits<double>::max();
  const static double DOUBLE_MIN = std::numeric_limits<double>::min();


  template<typename T, typename Allocator = std::allocator<T>>
  using Vector = std::vector<T, Allocator>;

  template<typename T, typename Allocator = std::allocator<T>>
  using List = std::list<T, Allocator>;

  template<typename T, typename Compare = std::less<T>, typename Allocator = std::allocator<T>>
  using Set = std::set<T, Compare, Allocator>;

  template<typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using Map = std::map<K, V, Compare, Allocator>;

  template<typename K, typename V, typename Compare = std::less<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using MultiMap = std::multimap<K, V, Compare, Allocator>;


  template<class T, typename HashFunc = std::hash<T>, typename Allocator = std::allocator<T>>
  using UnorderedSet = std::unordered_set<T, HashFunc, std::equal_to<T>, Allocator>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMap = std::unordered_map<K, V, HashFunc, std::equal_to<K>, Allocator>;

  template<typename K, typename V, typename HashFunc = std::hash<K>, typename Allocator = std::allocator<std::pair<const K, V>>>
  using UnorderedMultiMap = std::unordered_multimap<K, V, HashFunc, std::equal_to<K>, Allocator>;

  template<typename T, typename Comparator = std::less<T>> // less -> max prio queue
  using PriorityQueue = std::priority_queue<T, Vector<T>, Comparator>;


  template<typename T>
  using Queue = std::queue<T>;

  template<typename T>
  using Stack = std::stack<T, std::vector<T>>;

  template<typename Iterator>
  using RangeIterator = std::pair<Iterator, Iterator>;

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

  typedef struct Point
  {
    Point(): Point(UINT_UNDEF, 0,0) {}
    Point(size_t id_val, coordinate_t x_val, coordinate_t y_val): id(id_val), x(x_val), y(y_val) {}

    std::pair<coordinate_2d_t, point_id_t> getCoordToId() const
    { return std::make_pair(coordinate_2d_t{x, y}, id); }
    coordinate_2d_t getCoordPair() const { return std::make_pair(x,y); }
    friend bool operator<(const Point& p1, const Point& p2)
    { return p1.id < p2.id; }

    size_t id;
    coordinate_t x;
    coordinate_t y;
  } point_t;

  typedef std::pair<point_id_t, point_id_t> pointid_pair_t;
  typedef std::pair<Point, Point> point_pair_t;



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

  template<typename T>
  struct PairLexicographicOrdering
  {
    using pair_t = std::pair<T,T>;
    bool operator()(const pair_t& p1, const pair_t& p2) const
    { return p1.first < p2.first || (p1.first == p2.first && p1.second < p2.second); }
  };

  inline vertex_pair_t getOrderedPair(vertex_t from, vertex_t to)
  {
    return from <= to ? std::make_pair(from, to) : std::make_pair(to, from);
  }

  template<typename S, typename T>
  inline std::pair<T,S> reversePair(const std::pair<S,T>& p)
  {
    return std::make_pair(p.second, p.first);
  }


  // for better readability
  #define NAMED_SIZE_PAIR(struct_name, type1, name1, type2, name2)  \
    struct struct_name {                                            \
      struct_name(): name1(UINT_UNDEF), name2(UINT_UNDEF) {}        \
      struct_name(type1 v1, type2 v2): name1(v1), name2(v2) {}      \
      type1 name1;                                                  \
      type2 name2;                                                  \
    };

  NAMED_SIZE_PAIR(VertexPointPair, vertex_t, vertex, point_id_t, point)
  NAMED_SIZE_PAIR(VertexDegreePair, vertex_t, vertex, size_t, degree)
  inline bool operator<(const VertexDegreePair& p1, const VertexDegreePair& p2)
  { return p1.degree < p2.degree; }

  typedef Map<coordinate_2d_t, point_id_t, PairLexicographicOrdering<coordinate_t>> CoordinateToIdMap;
  typedef Set<vertex_t> VertexSet;
  typedef Vector<vertex_t> VertexVector;
  typedef Vector<point_id_t> PointIdVector;

  class VertexAssignment;
  class Graph;
  class Verifier;
  class PointSet;
  struct Instance;
  class GreedyPlacement;
  class PlacementVisualizer;
  class KdTree;
  class PointClustering;
  class HierarchicalGraph;
  class IncrementalCollinear;
  struct LocalImprovementNN;
  class LocalGurobi;
}



#endif
