#version 330 core

// Couleur reçue du vertex shader
in vec3 vertexColor;

// Couleur finale envoyée à l'écran
out vec4 fragmentColor;

void main()
{
    fragmentColor = vec4(vertexColor, 1.0);
}