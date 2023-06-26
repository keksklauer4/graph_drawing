#include "placement_visualizer.hpp"
#include "gd_types.hpp"
#include "placement/placement_metrics.hpp"

#include <common/misc.hpp>
#include <common/assignment.hpp>

#include <cassert>
#include <cstddef>
#include <fstream>
#include <filesystem>

#include <math.h>
#include <numeric>
#include <stdexcept>
#include <utility>
#include <iostream>

#include <placement/divide/point_clustering.hpp>

using namespace gd;
namespace fs = std::filesystem;

const static std::string colors[] = {
  "Cyan", "purple", "green", "blue", "yellow", "magenta", "Navy","Aquamarine","Lawngreen","Violet","Darkslateblue","Darkblue","Sienna","Crimson","Tomato","Orangered","Mediumorchid","Mediumslateblue","Rosybrown","Skyblue","Cadetblue","Lightgreen","Springgreen","Paleturquoise","Firebrick","Gold","Blue",
  "Darkblue","Sienna","Crimson","Tomato","Orangered","Mediumorchid","Mediumslateblue","Rosybrown","Skyblue","Cadetblue","Lightgreen","Springgreen","Paleturquoise","Firebrick","Gold","Blue","Darkturquoise","Midnightblue","Lightslategray","Red","Darkviolet",
  "Mediumorchid","Mediumslateblue","Rosybrown","Skyblue","Cadetblue","Lightgreen","Springgreen","Paleturquoise","Firebrick","Gold","Blue","Darkturquoise","Midnightblue","Lightslategray","Red","Darkviolet","Royalblue","Lime","Plum","Darkorchid","Orange",
  "Lightgreen","Springgreen","Paleturquoise","Firebrick","Gold","Blue","Darkturquoise","Midnightblue","Lightslategray","Red","Darkviolet","Royalblue","Lime","Plum","Darkorchid","Orange","Darksalmon","Salmon","Lightsalmon","Forestgreen","Darkseagreen",
  "Blue","Darkturquoise","Midnightblue","Lightslategray","Red","Darkviolet","Royalblue","Lime","Plum","Darkorchid","Orange","Darksalmon","Salmon","Lightsalmon","Forestgreen","Darkseagreen","Mediumspringgreen","Turquoise","Coral","Mediumvioletred","Deeppink",
  "Darkviolet","Royalblue","Lime","Plum","Darkorchid","Orange","Darksalmon","Salmon","Lightsalmon","Forestgreen","Darkseagreen","Mediumspringgreen","Turquoise","Coral","Mediumvioletred","Deeppink","Teal","Darkgoldenrod","Limegreen","Darkslategray","Darkcyan",
  "Orange","Darksalmon","Salmon","Lightsalmon","Forestgreen","Darkseagreen","Mediumspringgreen","Turquoise","Coral","Mediumvioletred","Deeppink","Teal","Darkgoldenrod","Limegreen","Darkslategray","Darkcyan","Tan","Deepskyblue","Goldenrod","Gray","Yellow",
  "Darkseagreen","Mediumspringgreen","Turquoise","Coral","Mediumvioletred","Deeppink","Teal","Darkgoldenrod","Limegreen","Darkslategray","Darkcyan","Tan","Deepskyblue","Goldenrod","Gray","Yellow","Khaki","Darkgreen","Dimgray","Mediumaquamarine","Cornflowerblue",
  "Deeppink","Teal","Darkgoldenrod","Limegreen","Darkslategray","Darkcyan","Tan","Deepskyblue","Goldenrod","Gray","Yellow","Khaki","Darkgreen","Dimgray","Mediumaquamarine","Cornflowerblue","Darkorange","Lightgray","Palegoldenrod","Indianred","Slategray",
  "Darkcyan","Tan","Deepskyblue","Goldenrod","Gray","Yellow","Khaki","Darkgreen","Dimgray","Mediumaquamarine","Cornflowerblue","Darkorange","Lightgray","Palegoldenrod","Indianred","Slategray","Silver","Seagreen","Lightblue","Hotpink","Dodgerblue",
  "Yellow","Khaki","Dimgray","Mediumaquamarine","Cornflowerblue","Darkorange","Lightgray","Palegoldenrod","Indianred","Slategray","Silver","Seagreen","Lightblue","Hotpink","Dodgerblue","Indigo","Burlywood","Lightsteelblue","Yellowgreen","Peru",
  "Cornflowerblue","Darkorange","Lightgray","Palegoldenrod","Indianred","Slategray","Silver","Seagreen","Lightblue","Hotpink","Dodgerblue","Indigo","Burlywood","Lightsteelblue","Yellowgreen","Peru","Greenyellow","Palegreen","Palevioletred","Mediumpurple","Darkmagenta",
  "Slategray","Silver","Seagreen","Lightblue","Hotpink","Dodgerblue","Indigo","Burlywood","Lightsteelblue","Yellowgreen","Peru","Greenyellow","Palegreen","Palevioletred","Mediumpurple","Darkmagenta","Mediumblue","Mediumturquoise","Darkgray","Lightseagreen","Maroon",
  "Dodgerblue","Indigo","Burlywood","Lightsteelblue","Yellowgreen","Peru","Greenyellow","Palegreen","Palevioletred","Mediumpurple","Darkmagenta","Mediumblue","Mediumturquoise","Darkgray","Lightseagreen","Maroon","Darkolivegreen","Lightskyblue","Lightcoral","Aqua","Mediumseagreen",
  "Peru","Greenyellow","Palegreen","Palevioletred","Mediumpurple","Darkmagenta","Mediumblue","Mediumturquoise","Darkgray","Lightseagreen","Maroon","Darkolivegreen","Lightskyblue","Lightcoral","Aqua","Mediumseagreen","Darkred","Brown","Powderblue","Blueviolet","Steelblue",
  "Darkmagenta","Mediumblue","Mediumturquoise","Darkgray","Lightseagreen","Maroon","Darkolivegreen","Lightskyblue","Lightcoral","Aqua","Mediumseagreen","Darkred","Brown","Powderblue","Blueviolet","Steelblue","Chartreuse","Olivedrab","Chocolate","Thistle","Green",
  "Maroon","Darkolivegreen","Lightskyblue","Lightcoral","Aqua","Mediumseagreen","Darkred","Brown","Powderblue","Blueviolet","Steelblue","Chartreuse","Olivedrab","Chocolate","Thistle","Green","Darkkhaki","Wheat","Slateblue","Saddlebrown","Sandybrown",
  "Mediumseagreen","Darkred","Brown","Powderblue","Blueviolet","Steelblue","Chartreuse","Olivedrab","Chocolate","Thistle","Green","Darkkhaki","Wheat","Slateblue","Saddlebrown","Sandybrown","Fuchsia","Purple","Olive","Orchid",
  "Steelblue","Chartreuse","Olivedrab","Chocolate","Thistle","Green","Darkkhaki","Wheat","Slateblue","Saddlebrown","Sandybrown","Fuchsia","Purple","Olive","Orchid",
  "Green","Darkkhaki","Wheat","Slateblue","Saddlebrown","Sandybrown","Fuchsia","Purple","Olive","Orchid",
  "Sandybrown","Fuchsia","Purple","Olive","Orchid"
};
const static size_t nbColors = sizeof(colors)/sizeof(colors[0]);

#define Y_OFFSET 100

void PlacementVisualizer::setupDrawing()
{
  std::cout << "Iteration " << (m_iteration + 1) << "\n==================" << std::endl;
  if (!m_initialized) initialize();

  m_svg << m_prepared;

  double fontSize = std::min(getWidth() / 40, 40.0);
  double y = 50 + fontSize / 2.0;

  std::stringstream titleText {};
  titleText << "Iteration " << (m_iteration + 1) << ": ";
  m_svg << "<text x=\"" << getRadius() << "\" y=\"" << y
        << "\" font-size=\"" << fontSize
        << "\">" << titleText.str();
}

void PlacementVisualizer::draw(const char* title)
{
  setupDrawing();
  if (title != nullptr) m_svg << title;
  finishDrawing();
}

void PlacementVisualizer::drawClustering()
{
  assert(m_pointClustering != nullptr && "Point clustering not set!");
  DrawingMode mode = m_mode;
  m_mode = DRAW_ONLY_CLUSTERING;
  setupDrawing();
  const char* title = "Clustering of the point set";
  m_svg << title;
  finishDrawing();
  m_mode = mode;
}

void PlacementVisualizer::finishDrawing()
{
  m_svg << "</text>";
  if(m_mode != DRAW_ONLY_CLUSTERING)
  {
    drawEdges();
    drawNodes();
  }
  else drawClusteredNodes();

  m_svg << "</svg>";
  writeToFile();
}

void PlacementVisualizer::drawClusteredNodes()
{
  for (const auto& point : m_instance.m_points)
  {
    drawPoint(translatePoint(point), getRadius(),
        getColor(m_pointClustering->getCluster(point.id)));
  }
}

const std::string& PlacementVisualizer::getColor(size_t node)
{
  return colors[node % nbColors];
}

void PlacementVisualizer::initialize()
{
  fs::path path = m_filename;
  auto parent = path.parent_path();
  fs::create_directories(parent);

  const auto& pset = m_instance.m_points;
  if (pset.getNumPoints() == 0) throw std::runtime_error("No points");
  coordinate_2d_t x_range {1e10, -1e10};
  coordinate_2d_t y_range {1e10, -1e10};
  for (const auto& p : pset)
  {
    lowerTo(x_range.first, p.x);
    lowerTo(y_range.first, p.y);
    higherTo(x_range.second, p.x);
    higherTo(y_range.second, p.y);
  }

  calculateGCDScaling();
  double min_dist = 1;//minDistance();

  std::cout << "Min dist " << min_dist << std::endl;
  m_scaling = getRadius() * 4 / min_dist;
  m_width = ((x_range.second - x_range.first) / m_gcdScaling + 2) * m_scaling;
  higherTo(m_width, 1000ul);
  m_height = ((y_range.second - y_range.first) / m_gcdScaling + 2) * m_scaling;
  higherTo(m_height, 1000ul);

  m_pointTranslation = std::make_pair(-x_range.first / m_gcdScaling + 1, -y_range.first  / m_gcdScaling + 1);


  auto width = getWidth();
  auto height = getHeight() + Y_OFFSET;
  m_svg << "<svg height=\"" << height
     << "\" width=\"" << width << "\" xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 "
     << width << " " << height << "\">"
     << "<rect x=\"0\" y=\"0\" width=\"" << width
     << "\" height=\"" << height << "\" fill=\"white\"/>";

  double radius = getPointRadius();
  std::string black = "black";

  for (const auto& point : m_instance.m_points)
  {
    drawPoint(translatePoint(point), radius, black);
  }


  m_prepared = m_svg.str();
  m_svg.str(std::string());

  m_initialized = true;
}

double PlacementVisualizer::minDistance() const
{
  double min_dist = DOUBLE_MAX;
  const auto& pset = m_instance.m_points;
  size_t num_points = pset.getNumPoints();
  for (point_id_t p1 = 0; p1 < num_points; ++p1)
  {
    for (point_id_t p2 = p1 + 1; p2 < num_points; ++p2)
    {
      lowerTo(min_dist, squared_distance(pset.getPoint(p1), pset.getPoint(p2)));
    }
  }

  return sqrt(min_dist);
}

void PlacementVisualizer::calculateGCDScaling()
{
  const auto& pset = m_instance.m_points;
  m_gcdScaling = UINT_UNDEF;
  for (const auto& p : pset)
  {
    if (!isDefined(m_gcdScaling))
    {
      if (p.x != 0) m_gcdScaling = p.x;
      else if (p.y != 0) m_gcdScaling = p.y;
      else continue;
    }
    if (p.x != 0) lowerTo(m_gcdScaling, std::gcd(m_gcdScaling, p.x));
    if (p.y != 0) lowerTo(m_gcdScaling, std::gcd(m_gcdScaling, p.y));
  }
  if (m_gcdScaling == 0 || !isDefined(m_gcdScaling)) m_gcdScaling = 1;
}


void PlacementVisualizer::writeToFile()
{
  std::stringstream name;
  name << m_filename << "_" << (++m_iteration) << ".svg";
  std::ofstream f(name.str());
  if (!f.is_open()) throw std::runtime_error("Could not open file for visualization of embedding.");

  f << m_svg.str();

  f.close();
  m_svg.str(std::string());
}

void PlacementVisualizer::drawNodes()
{
  for (vertex_t v = 0; v != m_instance.m_graph.getNbVertices(); ++v)
  {
    if (m_assignment->isAssigned(v))
    {
      const Point& p = m_instance.m_points.getPoint(m_assignment->getAssigned(v));
      drawPoint(translatePoint(p), getRadius(), getColor(v));
    }
  }
}

void PlacementVisualizer::drawEdges()
{
  std::string color = "black";
  const auto& pset = m_instance.m_points;
  for (const auto& edge : m_instance.m_graph)
  {
    if (edge.first < edge.second && m_assignment->isAssigned(edge.first)
      && m_assignment->isAssigned(edge.second))
    {
      const auto& p1 = pset.getPoint(m_assignment->getAssigned(edge.first));
      const auto& p2 = pset.getPoint(m_assignment->getAssigned(edge.second));
      drawEdge(translatePoint(p1), translatePoint(p2), color);
    }
  }
}

void PlacementVisualizer::drawPoint(Point p, double radius, const std::string& color)
{
  m_svg << "<circle id=\"point_" << p.id << "\" r=\"" << radius << "\" cx=\"" << p.x
         << "\" cy=\"" << (p.y + Y_OFFSET) << "\" fill=\""
         << color << "\" stroke=\"black\" />";
}

void PlacementVisualizer::drawEdge(Point p1, Point p2, const std::string& color, double stroke)
{
  #define COORD(c) c.x << "," << (c.y + Y_OFFSET) << " "
  m_svg << "<polyline points=\"" <<  COORD(p1);
  m_svg << COORD(p2) << "\" fill=\"none\" stroke=\""
        << color << "\" stroke-width=\"" << stroke << "\"/>";
}

Point PlacementVisualizer::translatePoint(const Point& p) const
{
  return Point{ p.id, (p.x  / m_gcdScaling + m_pointTranslation.first) * m_scaling,
    (p.y  / m_gcdScaling + m_pointTranslation.second) * m_scaling};
}

// TODOs:
// add tool tips
// add scaling depending on nearest points
// scale multiplicative
