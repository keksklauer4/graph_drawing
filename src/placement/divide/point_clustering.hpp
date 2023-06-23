#ifndef __GD_POINT_CLUSTERING_HPP__
#define __GD_POINT_CLUSTERING_HPP__

#include <gd_types.hpp>

namespace gd
{

  struct Cluster
  {
    Cluster(): m_offset(UINT_UNDEF), m_size(0) {}

    size_t m_offset;
    size_t m_size;
  };

  class PointClustering
  {
    public:
      PointClustering(const Instance& instance);
      void cluster();

    private:
      const Instance& m_instance;
      Vector<Cluster> m_clusters;
      Vector<point_id_t> m_points;
  };


}


#endif
