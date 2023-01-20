#version 330 core
layout (triangles) in;  //输入图元类型
layout (triangle_strip, max_vertices=18) out; //输出图元类型

uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // 指定要向哪个立方体贴图面发射primitive图元 仅当fbo附着cubemap的时候有用
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 

//调用EmitVertex()后，gl_Position中的数据会被添加到输出的图元中
//调用EndPrimitive()后，所有发射出的顶点会合成指定输出的图元
//重复调用EndPrimitive()能够生成多个图元