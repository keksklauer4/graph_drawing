import generate_graph
from graph import Graph
from pysat.solvers import Solver
from pysat.card import *


def mij(i, j, number_of_points):
    return j + i * number_of_points + 1


def compute_mapping(graph, graphname):
    s = Solver()
    # using literals from 1 to num_nodes * num_points
    for i in range(graph.number_of_nodes):
        exactly_mapped_to_one = [mij(i, j, graph.number_of_points) for j in range(graph.number_of_points)]
        #print(exactly_mapped_to_one)
        s.append_formula(CardEnc.equals(exactly_mapped_to_one, encoding=EncType.pairwise))
    # no new literals
    for j in range(graph.number_of_points):
        at_most_mapped_by_one = [mij(i, j, graph.number_of_points) for i in range(graph.number_of_nodes)]
        # print(at_most_mapped_by_one)
        s.append_formula(CardEnc.atmost(at_most_mapped_by_one, encoding=EncType.pairwise))

    for j in range(graph.number_of_points):
        used_j = graph.number_of_nodes * graph.number_of_points + j + 1
        #print(used_j)
        impl_left = [mij(i, j, graph.number_of_points) for i in range(graph.number_of_nodes)]
        impl_left.append(-used_j)
        s.add_clause(impl_left)
        #print(impl_left)
        for i in range(graph.number_of_nodes):
            impl_right_i = [used_j, -mij(i, j, graph.number_of_points)]
            s.add_clause(impl_right_i)

    colin_points = graph.colinear_points
    for e in graph.edges:
        i, k = e
        for j in range(graph.number_of_points):
            m_ij = mij(i, j, graph.number_of_points)
            for l in range(graph.number_of_points):
                if j != l:
                    m_kl = mij(k, l, graph.number_of_points)
                    colin_temp = []
                    if j < l:
                        colin_temp = colin_points[(j, l)]
                    else:
                        colin_temp = colin_points[(l, j)]
                    if len(colin_temp) != 0:
                        #print(j, l)
                        #print([used_id for used_id in colin_temp])
                        #print([graph.number_of_nodes * graph.number_of_points + used_id + 1 for used_id in colin_temp])
                        list_temp = [graph.number_of_nodes * graph.number_of_points + used_id + 1 for used_id in colin_temp]
                        for u_j in list_temp:
                            #(m_ij and m_kl) -> -u_j
                            #if(j == 2 and l == 0):
                                #print([-m_ij, -m_kl, -u_j])
                            s.add_clause([-m_ij, -m_kl, -u_j])


    s.solve()
    model = s.get_model()
    # print(model)
    for i in range(graph.number_of_nodes):
        model_i = [model[j + i * graph.number_of_points] for j in range(graph.number_of_points)]
        #print(model_i)
        for k in model_i:
            if (k > 0):
                point_j = (k - 1) % graph.number_of_points
                graph.map_node_to_point(i, point_j)

    #used = [model[j + graph.number_of_nodes * graph.number_of_points] for j in range(graph.number_of_points)]
    #print(used)
    filetype = '.svg'
    mapped = '_mapped'

    failed = '_fail'
    if (graph.is_valid()):
        filename = graphname + mapped + filetype
        print('computed a valid mapping with ', graph.count_crossings(),' crossings')
        graph.to_svg(filename)
    else:
        filename = graphname + failed + filetype
        print('mapping is not valid')
        graph.to_svg(filename)
    #print(graph.mapped_to)


if __name__ == '__main__':
    graph1 = generate_graph.generate_4K4()
    #graph1.to_json('4K4_test.json')
    graph1.to_svg('4K4_initial.svg')
    #graph1 = Graph.from_json('4K4.json')
    #compute_mapping(graph1, '4K4')

    graph2 = generate_graph.generate_4x4K4()
    #graph2.to_json('4x4K4.json')
    graph2.to_svg('4x4K4_initial.svg')
    #graph2 = Graph.from_json('4x4K4.json')
    #compute_mapping(graph2, '4x4K4')
    
    #graph3 = generate_graph.generate_16x4K4()
    #graph3.to_svg('16x4K4_initial.svg')
    #graph3.to_json('16x4K4.json')
    #graph3 = Graph.from_json('16x4K4.json')
    #compute_mapping(graph3, '16x4K4')

