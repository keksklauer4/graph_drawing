from tulip import tlp
import networkx as nx
import random
import json

def generate_planar_graph(identifier, num_nodes, factor_points):
    filename = 'planar'+ str(num_nodes) +'x'+ str(factor_points) + identifier+'.json'
    data = {}
    params = tlp.getDefaultPluginParameters('Planar Graph')
    params['nodes'] = num_nodes
    params['directed'] = False
    params['multiple edges'] = False
    graph = tlp.importGraph('Planar Graph', params)
    G = nx.Graph()
    nodes = []
    edges = []
    for node in graph.getNodes():
        G.add_node(node.id)
        nodes.append({'id' : node.id, 'x': 0, 'y' : 0})
    for edge in graph.getEdges():
        G.add_edge(graph.source(edge).id, graph.target(edge).id)
        edges.append({'source' : graph.source(edge).id, 'target' :graph.target(edge).id})
    data['nodes'] = nodes
    data['edges'] = edges
    is_planar, planar_embedding = nx.check_planarity(G)
    points = []
    if(is_planar):
        pos = nx.algorithms.combinatorial_embedding_to_pos(planar_embedding)
        for i in pos.keys():
            points.append(pos[i])
        
        smallest_x = 1000
        biggest_x = 0
        smallest_y = 1000
        biggest_y = 0
        
        for x, y in points:
            if (x < smallest_x):
                smallest_x = x
            if (x > biggest_x):
                biggest_x = x
            if (y < smallest_y):
                smallest_y = y
            if (y > biggest_y):
                biggest_y = y
        
        x = smallest_x
        y = smallest_y
        width = biggest_x - smallest_x + 1
        height =  biggest_y - smallest_y + 1
        num_points = width * height
        num_points_to_add = (factor_points - 1) * num_nodes
        if( num_points - num_nodes - num_points_to_add < 0):
            print('fail')
        else:
            for i in range(num_points_to_add):
                success = False
                while(not success):
                    p = (random.randint(x, width + x), random.randint(y, y + height))
                    if p not in points:
                        points.append(p)
                        success = True
            data['points'] = [{'id':point_id, 'x': points[point_id][0], 'y': points[point_id][1]} for point_id in range(len(points))]
            data['width'] = width
            data['height'] = height
            js = json.dumps(data)
            fp = open(filename, 'a')
            fp.write(js)
            fp.close()
    else:
        print('fail')
    
if __name__ == '__main__':
    number_of_nodes = [10000]
    number_of_versions = ['a', 'b']
    number_of_factors = [1, 2, 4]
    for num_nodes in number_of_nodes:
        for factor in number_of_factors:
            for version in number_of_versions:
                generate_planar_graph(version, num_nodes, factor)