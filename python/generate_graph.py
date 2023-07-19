from graph import Graph
def generate_4K4():
    points = {i: ((2 * (i % 5)), (2 * (i - (i % 5)) // 5)) for i in range(35)}
    nodes = {i: (((i % 3) + 3), ((i - (i % 3)) // 3 + 5)) for i in range(9)}
    edges = []
    for i in range(9):
        x = i % 3
        y = (i - x) // 3

        if(x != 2):
            edges.append((i, i + 1))
            if(y != 2):
                edges.append((i, i + 4))
        if(y != 2):
            edges.append((i, i + 3))
            if(x != 0):
                edges.append((i, i + 2))
    return Graph(nodes, points, edges)

def generate_4x4K4():
    width = 10
    height = 14
    num_points = width * height
    points = {i: ((2 * (i % width)), (2 * (i - (i % width)) // width)) for i in range(num_points)}
    nodes = {(i+x * 9 + y * 18): (((i % 3) + 3) + x * width, ((i - (i % 3)) // 3 + 5)+ y * height) for i in range(9) for x in range(2) for y in range(2)}

    edges = []
    for x in range(2):
        for y in range(2):
            for i in range(9):
                index = (i+x * 9 + y * 18)
                x1 = i % 3
                y1 = (i - x1) // 3
                if(x1 != 2):
                    edges.append((index, index + 1))
                    if(y1 != 2):
                        edges.append((index, index + 4))
                else:
                    if(y1 == 1 and x == 0):
                        edges.append((index, index - 2 + 9))
                if(y1 != 2):
                    edges.append((index, index + 3))
                    if(x1 != 0):
                        edges.append((index, index + 2))
                else:
                    if(x1 == 1 and y == 0):
                        edges.append((index, index - 6 + 18))


    return Graph(nodes, points, edges)

def generate_16x4K4():
    width = 20
    height = 28
    num_points = width * height
    points = {i: ((2 * (i % width)), (2 * (i - (i % width)) // width)) for i in range(num_points)}
    nodes = {(i+x * 9 + y * 18 + x1 * 36 + y1 * 72): (((i % 3) + 3) + x * width//2 + x1 * width, ((i - (i % 3)) // 3 + 5)+ y * height//2 + y1 * height) for i in range(9) for x in range(2) for y in range(2) for x1 in range(2) for y1 in range(2)}

    edges = []
    for x2 in range(2):
        for y2 in range(2):
            for x in range(2):
                for y in range(2):
                    for i in range(9):
                        index = (i+x * 9 + y * 18 + x2 * 36 + y2 * 72)
                        x1 = i % 3
                        y1 = (i - x1) // 3
                        if(x1 != 2):
                            edges.append((index, index + 1))
                            if(y1 != 2):
                                edges.append((index, index + 4))
                        else:
                            if(y1 == 1 and x == 0):
                                edges.append((index, index - 2 + 9))
                            elif(y1 == 1 and x == 1 and x2 == 0):
                                edges.append((index, index - 11 + 36))                                
                        if(y1 != 2):
                            edges.append((index, index + 3))
                            if(x1 != 0):
                                edges.append((index, index + 2))
                        else:
                            if(x1 == 1 and y == 0):
                                edges.append((index, index - 6 + 18))
                            elif(x1 == 1 and y == 1 and y2 == 0):
                                edges.append((index, index - 24 + 72)) 
                                    
            
    return Graph(nodes, points, edges)
