#include "model.hpp"

#include "happly.h"

using namespace gl;

struct ModelPosition
{
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct ModelColor
{
    GLubyte r;
    GLubyte g;
    GLubyte b;
};

struct ModelVertex
{
    ModelPosition position;
    ModelColor color;
};


void Model::load(const char* path)
{
    // Chargement des données du fichier .ply.
    // Ne modifier pas cette partie.
    happly::PLYData plyIn(path);

    happly::Element& vertex = plyIn.getElement("vertex");
    std::vector<float> positionX = vertex.getProperty<float>("x");
    std::vector<float> positionY = vertex.getProperty<float>("y");
    std::vector<float> positionZ = vertex.getProperty<float>("z");
    
    std::vector<unsigned char> colorRed   = vertex.getProperty<unsigned char>("red");
    std::vector<unsigned char> colorGreen = vertex.getProperty<unsigned char>("green");
    std::vector<unsigned char> colorBlue  = vertex.getProperty<unsigned char>("blue");

    // Tableau de faces, une face est un tableau d'indices.
    // Les faces sont toutes des triangles dans nos modèles (donc 3 indices par face).
    std::vector<std::vector<unsigned int>> facesIndices = plyIn.getFaceIndices<unsigned int>();
    
    // TODO: Rassemblez les propriétés du fichier .ply pour correspondre au
    //       format de donnée souhaité (celui que vous avez défini dans la struct).
    std::vector<ModelVertex> modelData;

    for(size_t i = 0; i < positionX.size(); i++){

        ModelPosition coors  = {positionX[i], positionY[i], positionZ[i]};
        ModelColor    colors = {colorRed[i], colorGreen[i], colorBlue[i]};

        ModelVertex   vertex = {coors,colors};

        modelData.push_back(vertex);

    }
    
    // TODO: Rassemblez les indices dans un seul tableau contigu.
    std::vector<GLuint> modelFacesIndices;

    for(size_t i = 0 ; i < facesIndices.size() ; i++){
        for(size_t j = 0 ; j < 3 ; j++){
            modelFacesIndices.push_back(facesIndices[i][j]);
        }
    }
    
    // TODO: Allocation des ressources sur la carte graphique et envoyer les
    //       données traitées dans le vbo et ebo sur la carte graphique.
    
    // Création des ressources (objets membres de la classe Model)
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glGenBuffers(1, &ebo_);
    
    // TODO: Créez un vao et spécifiez le format des données dans celui-ci.
    //       N'oubliez pas de lier le ebo avec le vao et de délier le vao
    //       du contexte pour empêcher des modifications sur celui-ci.

    // Spéciefie le vao à utiliser
    glBindVertexArray(vao_);

    // Spécifie le vbo à utiliseer
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);

    // Copie les données du ModelVertex array en RAM vers la VRAM GPU
    glBufferData(   GL_ARRAY_BUFFER, // le type du VBO
                    (modelData.size() * sizeof(ModelVertex)), // la taille totale des data à copier
                    modelData.data(), // Les data à copier
                    GL_STATIC_DRAW // default value du cours ?
                );

    // Spécifie l'ebo à utiliser 
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

    // Copie les données dans l'ebo
    glBufferData(
                    GL_ELEMENT_ARRAY_BUFFER,
                    modelFacesIndices.size() * sizeof(GLuint),
                    modelFacesIndices.data(),
                    GL_STATIC_DRAW
                ); 

    // On explique au vao comment lire l'attribut position dans le vbo
    glVertexAttribPointer(
        0,  //lieu #0 dans le shader (.glsl)
        3,  //3 attributs diff (x,y,z)
        GL_FLOAT, //types des attributs
        GL_FALSE, //ne pas normaliser
        sizeof(ModelVertex), // Le nombre de bytes à sauter pour atteindre le sommet suivant
        reinterpret_cast<void*>(offsetof(ModelVertex, position))
        );
    // Apres avoir expliqué comment lire position, on active ce dernier
    glEnableVertexAttribArray(0);

    // On explique au vao comment lire l'attribut color dans le vbo
    glVertexAttribPointer(
        1, //lieu #1 dans le shader (.glsl)
        3, //3 attributs diff (r,g,b)
        GL_UNSIGNED_BYTE, // unsigned char == unsigned byte type
        GL_TRUE, // normaliser (valeurs entre 0 et 255)
        sizeof(ModelVertex), // Le nombre de bytes à sauter pour atteindre le sommet suivant
        reinterpret_cast<const void*>(offsetof(ModelVertex, color))
        );
    // Apres avoir expliqué comment lire color, on active ce dernier
    glEnableVertexAttribArray(1);

        
    // TODO: Initialisez count_, qui correspond au nombre d'indices à dessiner.
    count_ = static_cast<GLsizei>(modelFacesIndices.size());

    // unbind le vao
    glBindVertexArray(0);
}

Model::~Model()
{
    glDeleteBuffers(1, &vbo_);
    glDeleteBuffers(1, &ebo_);
    glDeleteVertexArrays(1, &vao_);
}

void Model::draw()
{
    // Sélectionner le VAO du modèle
    glBindVertexArray(vao_);

    // Dessiner les triangles à partir des indices de l'EBO
    glDrawElements(
        GL_TRIANGLES,    // type de primitive à dessiner
        count_,          // nombre total d'indices à lire dans l'ebo
        GL_UNSIGNED_INT, // le type contenu dans le vecteur modelFacesIndices
        0          // l'index ou démarrer la lecture
    );

    // délier le VAO
    glBindVertexArray(0);
}

