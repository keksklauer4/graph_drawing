import networkx as nx
import os, json, math, itertools

os.chdir("test/gdata/temp/rome")

files = os.listdir(".")
number_to_dump = 400

def dump_graph_on_grid(graph, grid_mult=20):
    n = len(graph.nodes)
    if (n <= 2):
        return
    grid_size = math.ceil(math.sqrt(n*grid_mult))
    node_dict = {node: i for i, node in enumerate(graph.nodes)}
    
    points = [
      {"id": i, "x": coord[0], "y": coord[1]}
          for i, coord in enumerate(itertools.product(range(grid_size), range(grid_size)))
    ]
    edges = [
      {"source": node_dict[edge[0]], "target": node_dict[edge[1]]}
          for edge in graph.edges
    ]
    return {"edges": edges, "points": points}


correct_files = list(filter(lambda f: f.endswith(".graphml"), files))
length = min(len(correct_files), number_to_dump)
for i, file in enumerate(correct_files):
    if i >= number_to_dump:
        break
    graph = nx.read_graphml(file)
    file_name = f'{file.split(".graph_ml")[0]}.json'
    with open(f"../../rome/{file_name}", 'w+') as out_file:
        print(f"[{round((i+1)/length*100,1)}%]\t Dumping {file_name}\t ({i+1} of {len(correct_files)})")
        json.dump(dump_graph_on_grid(graph), out_file)

print("Deleting .graphml files")
for file in correct_files:
    os.remove(file)