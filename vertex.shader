#version 450

in vec4 vPosition;
in vec4 vColor;


out vec4 color;

uniform mat4 uMVPMatrix;

void main(void)
{
    gl_Position = uMVPMatrix * vPosition;
    color = vColor;
}
