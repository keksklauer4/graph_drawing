#ifndef __GD_POINT_CLUSTERING_HPP__
#define __GD_POINT_CLUSTERING_HPP__

#include <cstddef>
#include <gd_types.hpp>
#include <ostream>

namespace gd
{
  typedef size_t cluster_t;

  struct Cluster
  {
    Cluster(): left(UINT_UNDEF), right(UINT_UNDEF), isLeftPoint(false), isRightPoint(false) {}
    Cluster(size_t l, size_t r, bool leftChild, bool rightChild)
      : left(l), right(r),
        isLeftPoint(leftChild), isRightPoint(rightChild) {}

    size_t left;
    size_t right;
    bool isLeftPoint;
    bool isRightPoint;
  };

  class PointClustering
  {
    public:
      PointClustering(const Instance& instance);
      void cluster();
      cluster_t getCluster(point_id_t point) const
      { return m_pointToCluster.at(point); }

      friend std::ostream& operator<<(std::ostream& os, const PointClustering& clustering)
      {
        os << "Number of point clusters: " << clustering.m_clusters.size() << std::endl;
        return os;
      }

      Cluster get_Cluster(cluster_t c){
        return m_clusters[c];
      }

      int getSize(){
        return m_clusters.size();
      }

    private:
      const Instance& m_instance;
      Vector<Cluster> m_clusters;
      Vector<cluster_t> m_pointToCluster;
  };


}


#endif
