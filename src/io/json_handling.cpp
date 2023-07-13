#include "json_handling.hpp"

#include "common/instance.hpp"
#include "common/pointset.hpp"
#include "gd_types.hpp"

#include <fstream>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <sstream>
#include <stdexcept>
#include <sys/types.h>
#include <iostream>

#include <common/assignment.hpp>
#include <io/printing.hpp>

using namespace gd;
using namespace rapidjson;

#define ParseList()

namespace
{

  void expectTrue(bool value, const char* msg)
  {
    if (!value) throw std::runtime_error(msg);
  }

  void assertDocumentStructure(const Document& doc)
  {
    expectTrue(doc.IsObject(), "Input file is invalid (1: not an object)");
    // expectTrue(doc.HasMember("nodes"), "Input file is invalid (2: no nodes)");
    expectTrue(doc.HasMember("edges"), "Input file is invalid (3: no edges)");
    expectTrue(doc.HasMember("points"), "Input file is invalid (4: no points))");
    // expectTrue(doc.HasMember("width"), "Input file is invalid (5: no width)");
    // expectTrue(doc.HasMember("height"), "Input file is invalid (6: no height)");

    expectTrue(doc["edges"].IsArray(), "Input file is invalid (7: edges not array))");
    expectTrue(doc["points"].IsArray(), "Input file is invalid (8: points not array)");
  }

  void parseEdges(Document& doc, Vector<vertex_pair_t>& edges)
  {
    const Value& edgeArr = doc["edges"];
    edges.resize(2 * edgeArr.Size());
    for (SizeType idx = 0; idx < edgeArr.Size(); ++idx)
    {
      expectTrue(edgeArr[idx].IsObject(), "Invalid edge");
      expectTrue(edgeArr[idx].HasMember("source")
        && edgeArr[idx].HasMember("target"), "Edge invalid properties.");
      expectTrue(edgeArr[idx]["source"].IsInt()
        && edgeArr[idx]["target"].IsInt(), "Edge properties not int");
      auto edge = vertex_pair_t(edgeArr[idx]["source"].GetInt(),
        edgeArr[idx]["target"].GetInt());
      edges[2*idx] = edge;
      edges[2*idx + 1] = reversePair(edge);
    }
  }

  void parsePoints(Document& doc, Vector<point_t>& points)
  {
    const Value& pointsArr = doc["points"];
    points.resize(pointsArr.Size());
    for (SizeType idx = 0; idx < pointsArr.Size(); ++idx)
    {
      expectTrue(pointsArr[idx].IsObject(), "Invalid point");
      expectTrue(pointsArr[idx].HasMember("x")
        && pointsArr[idx].HasMember("y") && pointsArr[idx].HasMember("id"), "Point invalid properties.");
      expectTrue(pointsArr[idx]["x"].IsInt()
        && pointsArr[idx]["y"].IsInt() && pointsArr[idx]["id"].IsInt(), "Point properties not int");
      points[idx] = Point{ (size_t)pointsArr[idx]["id"].GetInt64(), pointsArr[idx]["x"].GetInt(),
        pointsArr[idx]["y"].GetInt() };
    }
  }

}

instance_t gd::parseInstanceFromFile(const std::string& filename)
{
  std::ifstream file(filename);
  expectTrue(file.is_open(), "Could not open file.");
  IStreamWrapper wrapper(file);
  Document document;
  document.ParseStream(wrapper);
  assertDocumentStructure(document);
  Vector<vertex_pair_t> edges {};
  Vector<point_t> points {};

  parseEdges(document, edges);
  parsePoints(document, points);
  file.close();
  return Instance{
    Graph(edges),
    PointSet(std::move(points))
  };
}

void gd::dump_assignment(std::ostream& out, const Instance& instance,
                         const VertexAssignment& assignment)
{
  const auto& graph = instance.m_graph;
  const auto& pset = instance.m_points;
  // dumping nodes
  out << "{\"nodes\": [\n";
  for (vertex_t v = 0; v < graph.getNbVertices(); ++v)
  {
    Point point{};
    if (assignment.isAssigned(v)) point = pset.getPoint(assignment.getAssigned(v));
    out << "\t{\"id\": " << v << ", \"x\": " << point.x << ", \"y\": " << point.y << "}";
    if (v + 1 < graph.getNbVertices()) out << ",\n";
    else out << "]," << std::endl;
  }

  // dumping edges
  out << "\"edges\": [\n";
  bool first = true;
  for (const auto& edge : graph)
  {
    if (edge.first >= edge.second) continue;
    if (first) first = false;
    else
    {
      out << ",\n";
    }

    out << "\t{\"source\": " << edge.first << ", \"target\": " << edge.second << "}";
  }
  out << "],\n";

  // dumping points
  out << "\"points\": [\n";
  for (point_id_t p = 0; p < pset.getNumPoints(); ++p)
  {
    const auto& point = pset.getPoint(p);
    out << "\t{\"id\": " << point.id << ", \"x\": " << point.x << ", \"y\": " << point.y << "}";
    if (p + 1 < pset.getNumPoints()) out << ",\n";
    else out << "]" << std::endl;
  }

  out << "}";
}

void gd::dump_statistics(std::ostream& out, const RunStatistics& statistics)
{
  out << "{\"TODO\": 1}";
}
