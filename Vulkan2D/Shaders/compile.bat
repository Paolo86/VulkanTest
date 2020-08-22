glslc.exe basic.vert -o basic_vertex.spv
glslc.exe basic.frag -o basic_fragment.spv

glslc.exe pbr.vert -o pbr_vertex.spv
glslc.exe pbr.frag -o pbr_fragment.spv

glslc.exe pbr_instanced.vert -o pbr_instanced_vertex.spv
glslc.exe pbr_instanced.frag -o pbr_instanced_fragment.spv

glslc.exe phong.vert -o phong_vertex.spv
glslc.exe phong.frag -o phong_fragment.spv

glslc.exe phong_instanced.vert -o phong_instanced_vertex.spv
glslc.exe phong_instanced.frag -o phong_instanced_fragment.spv
pause