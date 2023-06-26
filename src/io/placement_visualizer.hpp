#ifndef __GD_PLACEMENT_VISUALIZER_HPP__
#define __GD_PLACEMENT_VISUALIZER_HPP__

#include <cassert>
#include <gd_types.hpp>
#include <common/instance.hpp>
#include <sstream>

namespace gd
{
  class PlacementVisualizer
  {
    enum DrawingMode : int
    {
      NORMAL_DRAWING = 0,
      COLOR_VERTICES_ACCORDING_TO_HIERARCHY = 1,
      DRAW_ONLY_CLUSTERING = 2
    };
    public:
      PlacementVisualizer(const instance_t& instance, std::string filename)
        : m_instance(instance), m_hierarchy(nullptr), m_pointClustering(nullptr),
          m_filename(filename), m_iteration(0), m_mode(NORMAL_DRAWING) { }

      void setAssignment(const VertexAssignment& assignment)
      { m_assignment = &assignment; }

      void setClustering(const PointClustering& pointClustering) { m_pointClustering = &pointClustering; }
      void setHierarchy(const HierarchicalGraph& hierarchy) { m_hierarchy = &hierarchy; }

    private:
      double getWidth() const { return m_width; }
      double getHeight() const { return m_height; }

      double getNodeSize() const { return 5; }
      double getRadius() const { return getNodeSize() / 2; }
      double getPointRadius() const { return getRadius() * 0.5; }
      double getStrokeWidth() const { return 0.6; }

      void initialize();
      void writeToFile();
      void setupDrawing();
      void finishDrawing();
      void drawPoint(Point p, double radius, const std::string& color);
      void drawEdge(Point p1, Point p2, const std::string& color, double stroke = 1);
      void drawNodes();
      void drawEdges();
      const std::string& getColor(size_t node);
      Point translatePoint(const Point& p) const;
      void calculateGCDScaling();
      void drawClusteredNodes();

      double minDistance() const;

    public:
      void draw(const char* title = nullptr);

      template<typename Functor>
      void draw(Functor textGen)
      {
        setupDrawing();
        textGen(m_svg);
        finishDrawing();
      }

      void drawClustering();

    private:
      bool m_initialized = false;
      const instance_t& m_instance;
      const VertexAssignment* m_assignment;
      const HierarchicalGraph* m_hierarchy;
      const PointClustering* m_pointClustering;
      std::string m_filename;
      size_t m_iteration;
      std::stringstream m_svg;
      std::string m_prepared;

      UnorderedMap<size_t, std::string> m_sourceNodeColors;

      coordinate_2d_t m_pointTranslation;
      size_t m_width;
      size_t m_height;
      coordinate_t m_scaling;
      coordinate_t m_gcdScaling;

      DrawingMode m_mode;
  };
}


#endif
