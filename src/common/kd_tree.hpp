#ifndef __GD_KD_TREE_HPP__
#define __GD_KD_TREE_HPP__

#include <gd_types.hpp>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Kd_tree.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Search_traits_2.h>
#include <CGAL/Search_traits_adapter.h>
#include <CGAL/property_map.h>

#include <functional>

namespace gd
{

  class KdTree
  {
    public:
      typedef CGAL::Simple_cartesian<coordinate_t> Kernel;
      typedef Kernel::Point_2 Point_2;
      typedef boost::tuple<Point_2, point_id_t> Point_and_id;
      typedef CGAL::Search_traits_2<Kernel> TreeBaseTraits;
      typedef CGAL::Search_traits_adapter<Point_and_id,
        CGAL::Nth_of_tuple_property_map<0, Point_and_id>,
        TreeBaseTraits> TreeTraits;
      typedef CGAL::Orthogonal_k_neighbor_search<TreeTraits> Neighbor_search;

      KdTree(const PointSet& pset);

      void k_nearest_neighbors(const Point& p, size_t k,
        std::function<bool(size_t)> func) const;

    private:
      const PointSet& m_pset;
      Vector<Point_2> m_points;
      Vector<point_id_t> m_pointIds;
      std::unique_ptr<Neighbor_search::Tree> m_tree;
  };

}


#endif
