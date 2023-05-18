import json
import numpy as np
import drawsvg as draw
import point


class Graph():
    def __init__(self, nodes, points, edges):
        self.nodes = nodes
        self.number_of_nodes = len(self.nodes.keys())
        self.points = points
        self.number_of_points = len(self.points.keys())
        self.reversed_points = {self.points[id]: id for id in self.points.keys()}
        self.list_of_points = [points[p_id] for p_id in points.keys()]
        self.list_of_points.sort()
        self.mapping = np.array([-1 for i in range(self.number_of_points)], dtype=int)
        self.mapped_to = np.array([-1 for i in range(self.number_of_nodes)], dtype=int)
        for id in self.nodes.keys():
            if self.nodes[id] in self.reversed_points.keys():
                self.map_node_to_point(id, self.reversed_points[self.nodes[id]])
        self.edges = edges
        self.number_of_edges = len(self.edges)
        self.neighbors = {id: [] for id in nodes.keys()}
        for e in self.edges:
            self.neighbors[e[0]].append(e[1])
            self.neighbors[e[1]].append(e[0])
        self.smallest_x = 0
        self.smallest_y = 0
        self.biggest_x = 0
        self.biggest_y = 0
        for id in self.points.keys():
            x = self.points[id][0]
            y = self.points[id][1]
            if (self.smallest_x > x):
                self.smallest_x = x
            if (self.smallest_y > y):
                self.smallest_y = y
            if (self.biggest_x < x):
                self.biggest_x = x
            if (self.biggest_y < y):
                self.biggest_y = y
        self.colinear_points = {}
        self.calculate_colinear_points()

    def from_json(filename='test.json'):
        f = open(filename)
        data = json.load(f)
        f.close()
        nodes = {v['id']: (v['x'], v['y']) for v in data['nodes']}
        points = {p['id']: (p['x'], p['y']) for p in data['points']}
        edges = [(e['source'], e['target']) for e in data['edges']]
        return Graph(nodes, points, edges)

    def to_json(self, filename='test.json'):
        data = {}
        data['nodes'] = [{'id': id, 'x': self.nodes[id][0], 'y': self.nodes[id][1]} for id in self.nodes.keys()]
        data['edges'] = [{'source': e[0], 'target': e[1]} for e in self.edges]
        data['points'] = [{'id': id, 'x': self.points[id][0], 'y': self.points[id][1]} for id in self.points.keys()]
        js = json.dumps(data)
        fp = open(filename, 'a')
        fp.write(js)
        fp.close()

    def to_svg(self, filename='test.svg'):
        width = self.biggest_x - self.smallest_x + 2
        height = self.biggest_y - self.smallest_y + 2
        pixel_per_int = 10
        res_x = width * pixel_per_int
        res_y = height * pixel_per_int
        d = draw.Drawing(res_x, res_y)
        # draw background
        d.append(draw.Rectangle(0, 0, res_x, res_y, fill='#777'))

        for id in self.points.keys():
            x_off = self.points[id][0] * pixel_per_int + pixel_per_int - 4
            y_off = res_y - (self.points[id][1] * pixel_per_int + pixel_per_int + 4)
            d.append(draw.Rectangle(x_off, y_off, 8, 8, fill='#1248ff', fill_opacity=0.5))
        for id in self.nodes.keys():
            x_off = self.nodes[id][0] * pixel_per_int + pixel_per_int
            y_off = res_y - (self.nodes[id][1] * pixel_per_int + pixel_per_int)
            d.append(draw.Circle(x_off, y_off, 4, fill='black'))

        for e in self.edges:
            x1 = self.nodes[e[0]][0] * pixel_per_int + pixel_per_int
            y1 = res_y - (self.nodes[e[0]][1] * pixel_per_int + pixel_per_int)
            x2 = self.nodes[e[1]][0] * pixel_per_int + pixel_per_int
            y2 = res_y - (self.nodes[e[1]][1] * pixel_per_int + pixel_per_int)
            d.append(draw.Line(x1, y1, x2, y2, fill='#eeee00', stroke='black'))
        d.set_pixel_scale(2)
        d.save_svg(filename)

    def map_node_to_point(self, node_id, point_id):
        if (self.mapped_to[node_id] != -1):
            self.mapping[self.mapped_to[node_id]] = -1
        self.mapping[point_id] = node_id
        self.mapped_to[node_id] = point_id
        self.nodes[node_id] = self.points[point_id]

    def mapping_contains_node(self, node_id):
        for i in range(len(self.mapping)):
            if (self.mapping[i] == node_id):
                return True
        return False

    def mapping_contains_node_at(self, node_id):
        for i in range(len(self.mapping)):
            if (self.mapping[i] == node_id):
                return i
        return -1

    def all_points_are_mapped(self):
        for id in range(0, self.number_of_nodes):
            if not self.mapping_contains_node(id):
                return False
        return True

    def calculate_colinear_points(self):
        for i in range(0, self.number_of_points):
            for j in range(i + 1, self.number_of_points):
                self.colinear_points[(i, j)] = []
                for id in self.points.keys():
                    if (id != i and id != j):
                        if (point.is_colinear(self.points[i], self.points[j], self.points[id]) and point.is_between(
                                self.points[i], self.points[j], self.points[id])):
                            self.colinear_points[(i, j)].append(id)

    def is_valid(self):
        if (not self.all_points_are_mapped()):
            return False

        for e in self.edges:
            point_id_p = self.mapped_to[e[0]]
            point_id_q = self.mapped_to[e[1]]
            if (point_id_p < point_id_q):
                colin_points = self.colinear_points[(point_id_p, point_id_q)]
            else:
                colin_points = self.colinear_points[(point_id_q, point_id_p)]
            for r in colin_points:
                if (self.mapping[r] != -1):
                    return False
        return True

    def check_if_mapping_is_valid(self):
        for e in self.edges:
            p_temp = self.nodes[e[0]]
            q_temp = self.nodes[e[1]]
            p = (-1, -1)
            q = (-1, -1)
            if ((p_temp[0] < q_temp[0]) or (p_temp[0] == q_temp[0] and p_temp[1] < q_temp[1])):
                p = p_temp
                q = q_temp
            else:
                p = q_temp
                q = p_temp
            # print(p,'->',q)

            starting_index = self.list_of_points.index(p)
            final_index = self.list_of_points.index(q)
            for i in range(starting_index, final_index):
                current_point = self.list_of_points[i]
                if (current_point != p and current_point != q):
                    if ((point.is_between(p, q, current_point) and point.is_colinear(p, q, current_point))):
                        # print(current_point)
                        if (self.mapping[self.reversed_points[current_point]] != -1):
                            print(current_point, 'is used')
                            return False
        return True

    def count_crossings(self):
        crossings = 0
        for i in range(0, self.number_of_edges):
            p = self.nodes[self.edges[i][0]]
            q = self.nodes[self.edges[i][1]]
            for j in range(i + 1, self.number_of_edges):
                r = self.nodes[self.edges[j][0]]
                s = self.nodes[self.edges[j][1]]
                if (not (p == r or p == s or q == r or q == s)):
                    o1 = point.orientation(p, q, r)
                    o2 = point.orientation(p, q, s)
                    o3 = point.orientation(r, s, p)
                    o4 = point.orientation(r, s, q)
                    if ((o1 != o2) and (o3 != o4)):
                        crossings += 1
        return crossings

    def generate_clauses(self):
        # each node has to get a point
        # each
        pass