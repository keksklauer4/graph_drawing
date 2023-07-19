import numpy as np
import drawsvg as draw
import point
from datetime import datetime

class Graph():
    def __init__(self, nodes, points, edges):
        self.nodes = nodes
        self.number_of_nodes = len(self.nodes.keys())
        self.points = points
        self.number_of_points = len(self.points.keys())
        
        
        self.mapping_point_to_node = np.array([-1 for i in range(self.number_of_points)], dtype=int)
        self.mapping_node_to_point = np.array([-1 for i in range(self.number_of_nodes)], dtype=int)
        
        self.edges = edges
        self.number_of_edges = len(self.edges)
        
        #data structures to save points related to node-id's
        self.colinear_points = {}
        self.clockwise_points = {}
        self.counterclockwise_points = {}
        
        self.calculate_colinear_points()
        print('graph is loaded\t\t\t', datetime.now().strftime('%Y-%m-%d %H:%M:%S'))
        
    def map_node_to_point(self, node_id, point_id):
        if (self.mapping_node_to_point[node_id] != -1):
            self.mapping_point_to_node[self.mapping_node_to_point[node_id]] = -1
        self.mapping_point_to_node[point_id] = node_id
        self.mapping_node_to_point[node_id] = point_id
        self.nodes[node_id] = self.points[point_id]
    
    #doesnt check if each number is contained once..
    def all_points_are_mapped(self):
        for i in self.mapping_node_to_point:
            if i == -1:
                return False
        return True
    
    def calculate_colinear_points(self):
        for i in self.points.keys():
            for j in self.points.keys():
                if(i != j):
                    self.colinear_points[(i, j)] = []
                    self.clockwise_points[(i, j)] = []
                    self.counterclockwise_points[(i, j)] = []
                    
                    for k in self.points.keys():
                        if (k != i and k != j):
                            if (point.is_colinear(self.points[i], self.points[j], self.points[k]) and point.is_between(
                                    self.points[i], self.points[j], self.points[k])):
                                self.colinear_points[(i, j)].append(k)
                            elif(point.is_clockwise(self.points[i], self.points[j], self.points[k])):
                                self.clockwise_points[(i, j)].append(k)
                            elif(point.is_counterclockwise(self.points[i], self.points[j], self.points[k])):
                                self.counterclockwise_points[(i, j)].append(k)

    def is_valid(self):
        if (not self.all_points_are_mapped()):
            print('not all points are mapped')
            return False

        for e in self.edges:
            point_id_p = self.mapping_node_to_point[e[0]]
            point_id_q = self.mapping_node_to_point[e[1]]
            if (point_id_p < point_id_q):
                colin_points = self.colinear_points[(point_id_p, point_id_q)]
            else:
                colin_points = self.colinear_points[(point_id_q, point_id_p)]
            for r in colin_points:
                if (self.mapping_point_to_node[r] != -1):
                    return False
        return True
    
    def count_crossings(self):
        crossings = 0
        for i, j in self.edges:
            for k, l in self.edges:
                if(i != k and i != l and j != k and j != l):
                    if(point.do_intersect(self.nodes[i], self.nodes[j], self.nodes[k], self.nodes[l])):
                        crossings += 1
        return crossings//2
    
    
    def to_svg(self, filename='test.svg'):
        smallest_x = 0
        smallest_y = 0
        biggest_x = 0
        biggest_y = 0
        for pid in self.points.keys():
            x, y = self.points[pid]
            if (smallest_x > x):
                smallest_x = x
            if (smallest_y > y):
                smallest_y = y
            if (biggest_x < x):
                biggest_x = x
            if (biggest_y < y):
                biggest_y = y
        width = biggest_x - smallest_x + 2
        height = biggest_y - smallest_y + 2
        pixel_per_int = 10
        res_x = width * pixel_per_int
        res_y = height * pixel_per_int
        d = draw.Drawing(res_x, res_y)
        # draw background
        d.append(draw.Rectangle(0, 0, res_x, res_y, fill='#777'))

        for pid in self.points.keys():
            x_off = self.points[pid][0] * pixel_per_int + pixel_per_int - 4
            y_off = res_y - (self.points[pid][1] * pixel_per_int + pixel_per_int + 4)
            d.append(draw.Rectangle(x_off, y_off, 8, 8, fill='#1248ff', fill_opacity=0.5))
        for pid in self.nodes.keys():
            x_off = self.nodes[pid][0] * pixel_per_int + pixel_per_int
            y_off = res_y - (self.nodes[pid][1] * pixel_per_int + pixel_per_int)
            d.append(draw.Circle(x_off, y_off, 4, fill='black'))

        for e in self.edges:
            x1 = self.nodes[e[0]][0] * pixel_per_int + pixel_per_int
            y1 = res_y - (self.nodes[e[0]][1] * pixel_per_int + pixel_per_int)
            x2 = self.nodes[e[1]][0] * pixel_per_int + pixel_per_int
            y2 = res_y - (self.nodes[e[1]][1] * pixel_per_int + pixel_per_int)
            d.append(draw.Line(x1, y1, x2, y2, fill='#eeee00', stroke='black'))
        d.set_pixel_scale(2)
        d.save_svg(filename)