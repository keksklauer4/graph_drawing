import json
import numpy as np
from datetime import datetime
import graph
import point
import gc

from pysat.solvers import Solver
from pysat.formula import CNFPlus
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
        cnf = CNFPlus()
        solver = Solver()
        
        
        
        o = np.array([[[point.orientation(g.points[p], g.points[q], g.points[r]) for r in g.points] for q in g.points] for p in g.points])
        m = np.array([[(j + i * g.number_of_points + 1) for j in range(g.number_of_points)] for i in range(g.number_of_nodes)])
        u = np.array([(g.number_of_nodes * g.number_of_points + j + 1) for j in range(g.number_of_points)])
        n = np.array([[False for j in g.nodes] for i in g.nodes] , dtype=bool)
        for i, k in g.edges:
            n[i][k] = True
            n[k][i] = True
        
        print('o, m, n, u build\t\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        #c = np.array([[point.do_intersect_different(g.points[i], g.points[j], g.points[k], g.points[l]) for i in range(g.number_of_points) for j in range(g.number_of_points)]for k in range(g.number_of_points) for l in range(g.number_of_points)])
        
        
        pc = np.array([[[[(o[p][q][r] != o[p][q][s] and o[p][q][r] != 0 and o[p][q][s] != 0) for s in g.points] for r in g.points] for q in g.points] for p in g.points])
        
        print('pc build\t\t\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        #c = np.array([[[[(pc[p][q][r][s] and pc[r][s][p][q]) for s in g.points] for r in g.points] for q in g.points] for p in g.points])
        
        #print('c build\t\t\t\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        mn = np.array([[(1 + (g.number_of_nodes + 1 + i) * g.number_of_points + j) for j in range(g.number_of_points)] for i in range(g.number_of_points)], dtype = int)
        
        print('data structures are build\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        #make hard clauses
        for i in range(g.number_of_nodes):
            cnf.extend(CardEnc.equals([int(m[i][j]) for j in range(g.number_of_points)], encoding=EncType.pairwise))
            
        for j in range(g.number_of_points):
            cnf.extend(CardEnc.atmost([int(m[i][j]) for i in range(g.number_of_nodes)], encoding=EncType.pairwise))
            
            impl_left = [int(m[i][j]) for i in range(g.number_of_nodes)]
            impl_left.append(-int(u[j]))
            cnf.append(impl_left)
            for i in range(g.number_of_nodes):
                cnf.append([int(u[j]), -int(m[i][j])])
        
        
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
                                cnf.append([-int(m[i][j]), -int(m[k][l]), -int(u[r])])
                                
        print('hard clauses are generated\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        for i, k in g.edges:
            for j in range(g.number_of_points):
                for l in range(g.number_of_points):
                    if j != l:
                        cnf.append([-int(m[i][j]), -int(m[k][l]), int(mn[j][l])])
        print('soft clauses1 are generated\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        
        #for j in g.points.keys():
        #    for l in g.points.keys():
        #        if j != l:
        #            for s in g.points.keys():
        #                for u in g.points.keys():
        #                    if j != s and j != u and l != s and l != u:
        #                        if(pc[j][l][s][u] and pc[s][u][j][l]):
        #                            cnf.append([-int(mn[j][l]), -int(mn[s][u])])
        for j, l in g.clockwise_points.keys():
            for s in g.clockwise_points[(j, l)]:
                for u in g.counterclockwise_points[(j, l)]:
                    if(pc[j][l][s][u] and pc[s][u][j][l]):
                        cnf.append([-int(mn[j][l]), -int(mn[s][u])])
        
        print('all soft clauses are generated\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        del o
        del m
        del u
        del n
        del mn
        del pc
        gc.collect()
        print('arrays are deleted\t\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        #cnf.to_file('test.cnf')
        #print('clauses are saved\t\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        solver.append_formula(cnf)
        print('clauses are passed to the solver', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        solver.solve()
        print('solver has finished\t\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        model = solver.get_model()
        if(model != None):
            for i in range(g.number_of_nodes):
                model_i = [model[j + i * g.number_of_points] for j in range(g.number_of_points)]
                for k in model_i:
                    if (k > 0):
                        point_j = (k - 1) % g.number_of_points
                        g.map_node_to_point(i, point_j)
            print('model is computed\t\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
            print(g.mapping_node_to_point, g.is_valid(), g.count_crossings())
            g.to_svg('result.svg')