def is_between(a, b, c):
    east = (a[0] < c[0] and c[0] < b[0] and a[1] == c[1] and c[1] == b[1])
    south_east = (a[0] < c[0] and c[0] < b[0] and a[1] > c[1] and c[1] > b[1])
    south = (a[0] == c[0] and c[0] == b[0] and a[1] > c[1] and c[1] > b[1])
    south_west = (a[0] > c[0] and c[0] > b[0] and a[1] > c[1] and c[1] > b[1])
    west = (a[0] > c[0] and c[0] > b[0] and a[1] == c[1] and c[1] == b[1])
    north_west = (a[0] > c[0] and c[0] > b[0] and a[1] < c[1] and c[1] < b[1])
    north = (a[0] == c[0] and c[0] == b[0] and a[1] < c[1] and c[1] < b[1])
    north_east = (a[0] < c[0] and c[0] < b[0] and a[1] < c[1] and c[1] < b[1])
    return east or south_east or south or south_west or west or north_west or north or north_east

def orientation(a, b, c):
    val = (b[1] - a[1]) * (c[0] - b[0]) - (b[0] - a[0]) * (c[1] - b[1])
    if(val == 0):
        return 0#colinear
    if(val > 0):
        return 1#clockwise
    if(val < 0):
        return -1#counterclockwise

def is_colinear(a, b, c):
    return orientation(a, b, c) == 0

def is_clockwise(a, b, c):
    return orientation(a, b, c) == 1

def is_counterclockwise(a, b, c):
    return orientation(a, b, c) == -1

def do_intersect(a, b, c, d):
    o1 = orientation(a, b, c)
    o2 = orientation(a, b, d)
    o3 = orientation(c, d, a)
    o4 = orientation(c, d, b)
    return (o1 != o2) and (o3 != o4) and (o1 != 0) and (o2 != 0) and (o3 != 0) and (o4 != 0)

def do_intersect_different(a, b, c, d):
    o1 = orientation(a, b, c)
    o2 = orientation(a, b, d)
    o3 = orientation(c, d, a)
    o4 = orientation(c, d, b)
    return (o1 != o2) and (o3 != o4) and (o1 != 0) and (o2 != 0) and (o3 != 0) and (o4 != 0) and (a != c) and (a != d) and (b != c) and (b != d)