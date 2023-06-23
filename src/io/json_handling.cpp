#include "json_handling.hpp"
#include "common/pointset.hpp"
#include "gd_types.hpp"

#include <fstream>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <stdexcept>
#include <sys/types.h>
#include <iostream>

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

  void parsePoints(Document& doc, Vector<coordinate_t>& points)
  {
    const Value& pointsArr = doc["points"];
    points.resize(pointsArr.Size());
    for (SizeType idx = 0; idx < pointsArr.Size(); ++idx)
    {
      expectTrue(pointsArr[idx].IsObject(), "Invalid point");
      expectTrue(pointsArr[idx].HasMember("x")
        && pointsArr[idx].HasMember("y"), "Point invalid properties.");
      expectTrue(pointsArr[idx]["x"].IsInt()
        && pointsArr[idx]["y"].IsInt(), "Point properties not int");
      points[idx] = Coordinate{ pointsArr[idx]["x"].GetInt(), 
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
  Vector<coordinate_t> points {};

  parseEdges(document, edges);
  parsePoints(document, points);
  return Instance{
    Graph(edges),
    PointSet(std::move(points))
  };
}
