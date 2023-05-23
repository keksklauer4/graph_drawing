import json
import numpy as np
import graph
import point
from datetime import datetime

from pysat.solvers import Solver
from pysat.card import CardEnc
from pysat.card import EncType

class SatSolver():
    
    def load_graph(self, filename):
        f = open(filename)
        data = json.load(f)
        f.close()
        nodes = {v['id']: (v['x'], v['y']) for v in data['nodes']}
        points = {p['id']: (p['x'], p['y']) for p in data['points']}
        edges = [(e['source'], e['target']) for e in data['edges']]
        return graph.Graph(nodes, points, edges)
        
    def __init__(self, filename):
        g = self.load_graph(filename)
        solver = Solver()
        
        m = np.array([[(j + i * g.number_of_points + 1) for j in range(g.number_of_points)] for i in range(g.number_of_nodes)])
        u = np.array([(g.number_of_nodes * g.number_of_points + j + 1) for j in range(g.number_of_points)])
        
        #make hard clauses
        
        for i in range(g.number_of_nodes):
            solver.append_formula(CardEnc.equals([int(m[i][j]) for j in range(g.number_of_points)], encoding=EncType.pairwise))
            
        for j in range(g.number_of_points):
            solver.append_formula(CardEnc.atmost([int(m[i][j]) for i in range(g.number_of_nodes)], encoding=EncType.pairwise))
            
            impl_left = [int(m[i][j]) for i in range(g.number_of_nodes)]
            impl_left.append(-int(u[j]))
            solver.add_clause(impl_left)
            for i in range(g.number_of_nodes):
                solver.add_clause([int(u[j]), -int(m[i][j])])
        
        
        for i, k in g.edges:
            for j in range(g.number_of_points):
                for l in range(g.number_of_points):
                    if j != l:
                        colin_temp = []
                        if j < l:
                            colin_temp = g.colinear_points[(j, l)]
                        else:
                            colin_temp = g.colinear_points[(l, j)]
                        if len(colin_temp) != 0:
                            for r in colin_temp:
                                solver.add_clause([-int(m[i][j]), -int(m[k][l]), -int(u[r])])
                                
        print('hard clauses are generated\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        
        for i, k in g.edges:
            for r, t in g.edges:
                if(i != r and i != t and k != r and k != t):
                    for j in range(g.number_of_points):
                        for l in range(g.number_of_points):
                            if j != l:
                                for s in g.clockwise_points[(j, l)]:
                                    for u in g.counterclockwise_points[(j, l)]:
                                        if(point.do_intersect(g.points[j], g.points[l], g.points[s], g.points[u])):
                                            solver.add_clause([-int(m[i][j]), -int(m[k][l]), -int(m[r][s]), -int(m[t][u])])
                            

        print('soft clauses are generated\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        
        solver.solve()
        model = solver.get_model()
        
        for i in range(g.number_of_nodes):
            model_i = [model[j + i * g.number_of_points] for j in range(g.number_of_points)]
            for k in model_i:
                if (k > 0):
                    point_j = (k - 1) % g.number_of_points
                    g.map_node_to_point(i, point_j)
        print('model is computed\t\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        print(g.mapping_node_to_point, g.is_valid(), g.count_crossings())
        g.to_svg('result.svg')