# Test the new exported coefficientfunction
from netgen.geom2d import unit_square
from ngsolve import *
sys.path.append(r'..\build\x64\Release')
from myngspy import *

mesh = Mesh(unit_square.GenerateMesh(maxh=0.2))
cf = MyCoefficient() 
Draw(cf,mesh,"myCoefficient")
