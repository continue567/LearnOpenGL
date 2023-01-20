#version 330 core
layout (triangles) in;  //����ͼԪ����
layout (triangle_strip, max_vertices=18) out; //���ͼԪ����

uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{
    for(int face = 0; face < 6; ++face)
    {
        gl_Layer = face; // ָ��Ҫ���ĸ���������ͼ�淢��primitiveͼԪ ����fbo����cubemap��ʱ������
        for(int i = 0; i < 3; ++i) // for each triangle's vertices
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
    }
} 

//����EmitVertex()��gl_Position�е����ݻᱻ��ӵ������ͼԪ��
//����EndPrimitive()�����з�����Ķ����ϳ�ָ�������ͼԪ
//�ظ�����EndPrimitive()�ܹ����ɶ��ͼԪ