from tiovx import *

context = Context("uc_sample_03")
graph = Graph()

in1 = Image(640, 480, DfImage.NV12)
in2 = Image(640, 480, DfImage.NV12)
out = Image(640, 480, DfImage.NV12)

graph.add ( NodeAbsDiff(in1, in2, out) )

context.add ( graph )

usecase = UsecaseCode(context)

usecase.generate_code()