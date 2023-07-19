#include "json_handling.hpp"

#include "common/instance.hpp"
#include "common/pointset.hpp"
#include "gd_types.hpp"

#include <cstddef>
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

#include <common/run_statistics.hpp>

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
  out << "{\n";//start dict
  out << "\"num_nodes\" : " << statistics.get_num_nodes() << ",\n";
  out << "\"num_edges\" : " << statistics.get_num_edges() << ",\n";
  out << "\"num_points\" : " << statistics.get_num_points() << ",\n";
  out << "\"time_cr\" : " << statistics.get_time_CR() << ",\n";
  out << "\"time_coll\" : " << statistics.get_time_COLL() << ",\n";
  out << "\"time_local_sat\" : " << statistics.get_time_LOCAL_SAT() << ",\n";
  out << "\"time_move_op\" : " << statistics.get_time_MOVE_OP() << ",\n";
  out << "\"time_init\" : " << statistics.get_time_INIT() << ",\n";
  out << "\"time_coll_re\" : " << statistics.get_time_COLL_RE() << ",\n";
  out << "\"time_gurobi\" : " << statistics.get_time_GUROBI() << ",\n";
  out << "\"time_gurobi_build\" : " << statistics.get_time_GUROBI_BUILD() << ",\n";
  auto num_initial = statistics.get_m_num_runs();
  out << "\"num_initial\" : " << num_initial << ",\n";
  out << "\"initial_placements\" :\t[\n";//start list of initial placements
  for(size_t i = 0; i < num_initial; i++){
    out << "\t\t\t{ \"valid\" : " << statistics.get_m_init_placement_results_i(i).first << ", \"cr\" : " << statistics.get_m_init_placement_results_i(i).second << "}";
    if(i < (num_initial - 1)) out << ",";
    out << "\n";
  }
  out << "\t\t\t],\n";
  auto num_reopts = statistics.get_num_m_reopt_results();
  out << "\"num_reopts\" : " << num_reopts << ",\n";
  out << "\"reopts\" :\t\t[\n";
  for (size_t i = 0; i < num_reopts; i++) {
    out << "\t\t\t{ \"method\" : " << statistics.get_m_reopt_results_i(i).first << ", \"cr\" : " << statistics.get_m_reopt_results_i(i).second << "}";
    if(i < (num_reopts - 1)) out << ",";
    out << "\n";
  }
  out << "\t\t\t],\n";

  auto num_progress = statistics.get_num_m_progress();
  int current_id = -1;
  int placed_nodes = 0;
  int move_opts = 0;
  int reopts = 0;
  int coll_rebuilds = 0;
  int deaths = 0;
  int handled_placements = 0;
  out << "\"placements_detailed\" :\t[\n";
  bool started_final_placement = false;
  out << "\t\t\t{\n\t\t\t\"initial_placement\" : " << 0 << ",\n";
  out << "\t\t\t\"node_placement\" : \n\t\t\t[\n";

  size_t final_start = 0;

  for(size_t i = 0; i < num_progress; i++){
    auto current = statistics.get_m_progress_i(i);
    if(current.type == 5)
    {
      final_start = i;
      break;
    }
  }
  std::cout << "final_start " << final_start << std::endl;
  for(size_t i = 0; i < num_progress; i++){
    auto current = statistics.get_m_progress_i(i);

    if(handled_placements <= statistics.get_m_num_runs() && i < final_start)
    {
      if(current_id != current.curr_placement_idx)
      {
        current_id = current.curr_placement_idx;
        move_opts = 0;
        reopts = 0;
        coll_rebuilds = 0;
        deaths = 0;
        placed_nodes = 0;
        handled_placements++;
      }
      if(current.type == 0) 
      {
        if(placed_nodes > -1)
        {
          out << "\t\t\t{\"iteration\" : " << i << ", \"vertex_id\" : " <<  placed_nodes << ", \"move_opts\" : " << move_opts << ", \"reopts\" : " << reopts << ", \"coll_rebuilds\" : " << coll_rebuilds << ", \"deaths\" : " << deaths << "}";
          if(placed_nodes < statistics.get_num_nodes() - 1)
          {
            out << ",\n";
          }
          else 
          {
            out << "\n\t\t\t]\n\t\t\t},\n";
            if(handled_placements < statistics.get_m_num_runs())
            {
              out << "\t\t\t{\n\t\t\t\"initial_placement\" : " << (current.curr_placement_idx + 1) << ",\n";
              out << "\t\t\t\"node_placement\" :\n\t\t\t[\n";
            }
            else
            {
              out << "\t\t\t{\n\t\t\t\"final_placement\" : " << current.curr_placement_idx << ",\n";
              out << "\t\t\t\"improvements\" :\n\t\t\t[\n";
            }
          }
          move_opts = 0;
          reopts = 0;
          coll_rebuilds = 0;
          deaths = 0;
        }
        placed_nodes++;
      }
      else 
      {
        if(current.type == 1) move_opts++;
        if(current.type == 2) reopts++;
        if(current.type == 3) coll_rebuilds++;
        if(current.type == 4) deaths++;
      }
    }
    else 
    {
      out << "\t\t\t{\"iteration\" : " << i << ", \"placement_id\" : " <<  current.curr_placement_idx << ", \"type\" : " << current.type << ", \"cr\" : " << current.curr_crossings << ", \"time\" : " << current.time_stamp << "}";
      if(i < num_progress - 1) out << ",";
      out << "\n";
    }
  }
  out << "\t\t\t]\n\t\t\t}\n\t\t\t]\n}";
}
