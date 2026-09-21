#version 330 core

// TODO: Définir les entrées et sorties pour donner une position
//       et couleur à chaque vertex.

// VS veut dire vertex shader
// A pour role de calculer la position d'un sommet à l'écran
// Basic signifie shader simple qui n'a pas de matrice

// Déclaration de la position du sommet
layout(location = 0) in vec2 position;

// Déclaration de la couleur de sommet
layout(location = 1) in vec3 color;

// Déclare la sortie du sommet pour l'envoyer au fragment shader
out vec3 vertexColor;


void main()
{
    // Indique ou positionner le sommet (variable obligatoire)
    gl_Position = vec4(position, 0.0, 1.0);

    // Copie la variable couleur créée vers la sortie du sommet
    vertexColor = color;
}
