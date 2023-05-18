def is_between(a, b, c):
    return (a[0] <= c[0] and b[0] >= c[0] and a[1] <= c[1] and b[1] >= c[1])

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