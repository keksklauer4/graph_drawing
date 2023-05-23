import solver
import point
import numpy as np
from pysat.formula import CNFPlus
from pysat.formula import WCNFPlus
if __name__ == '__main__':
    s = solver.SatSolver('4x4K4.json')
    