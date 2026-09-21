#include "windmill.hpp"

#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


using namespace gl;
using namespace glm;

    
Windmill::Windmill()
: windSpeed(0.f)
, windAngle(0.f)
, angularSpeed(0.0f)
, rotorAngle(0.f)
, roofAngle(0.0f)
{}

void Windmill::sendMatrixToShaders(const glm::mat4& mvp)
{
    glUniformMatrix4fv(
        mvpUniformLocation,
        1,
        GL_FALSE,
        glm::value_ptr(mvp)
    );
}

void Windmill::drawCompleteBlade(
    const glm::mat4& projView,
    const glm::mat4& rotorCenterMatrix
)
{
    // La pale complete est fixee a 0.13 unite du centre du mat du rotor.
    mat4 attachmentMatrix = translate(
        rotorCenterMatrix,
        vec3(0.0f, 0.0f, 0.13f)
    );

    // La pale et son cadre doivent etre deux fois plus petits.
    attachmentMatrix = scale(
        attachmentMatrix,
        vec3(0.5f)
    );

    // Le cadre est centre a z+2.38 unités du point d'attache.
    mat4 frameMatrix = translate(
        attachmentMatrix,
        vec3(0.0f, 0.0f, 2.38f)
    );
    sendMatrixToShaders(projView * frameMatrix);
    bladeframe_.draw();

    // La pale est plus loin
    mat4 bladeMatrix = translate(
        attachmentMatrix,
        vec3(1.23f, 0.0f, 2.75f)
    );
    sendMatrixToShaders(projView * bladeMatrix);
    blade_.draw();
}

void Windmill::loadModels()
{
    blade_.load("../models/windmill-blade.ply");
    bladeframe_.load("../models/windmill-bladeframe.ply");
    bladebeam_.load("../models/windmill-bladebeam.ply");
    mainbeam_.load("../models/windmill-mainbeam.ply");
    millstone_.load("../models/windmill-millstone.ply");
    roof_.load("../models/windmill-roof.ply");
    walls_.load("../models/windmill-walls.ply");
}

void Windmill::update(float deltaTime)
{
    if (deltaTime < 0.001)
        return;
    
    const float RADIUS = 12.5f;
    const float TSR = 6.0f;
    float angularAccel = TSR / RADIUS * windSpeed * deltaTime;

    const float FRICTION_FACTOR = 0.2f;
    float friction = angularSpeed * FRICTION_FACTOR;
    angularAccel -= friction;
    angularSpeed += angularAccel * deltaTime;
    rotorAngle += angularSpeed * deltaTime;
    
    roofAngle += TSR / RADIUS * (windAngle - roofAngle) * angularSpeed * deltaTime;
}

void Windmill::draw(glm::mat4& projView)
{
    
    
    // Le moulin doit être devant la vue -10, scale x5 (matrice de départ)
    static const mat4 windmillMatrix = scale(
        translate(mat4(1.0f), vec3(0.0f, 0.3f, -10.0f)),
        vec3(5.0f)
    );

    sendMatrixToShaders(projView * windmillMatrix);
    walls_.draw();

    mat4 roofMatrix = windmillMatrix;
    // Le toit du moulin est à y + 3.03
    roofMatrix = translate(roofMatrix, vec3(0.0f,3.03f,0.0f));
    // rotation du toit, vec3 est l'axe de rotation
    roofMatrix = rotate(roofMatrix, roofAngle,vec3(0.0f,1.0f,0.0f));
    sendMatrixToShaders(projView * roofMatrix);
    roof_.draw();

    // Le mât du rotor est plus haut de 0.25 et ressort de 0.7
    mat4 bladeBeamMatrix = translate(
        roofMatrix,
        vec3(0.0f, 0.25f, 0.7f)
    );

    sendMatrixToShaders(projView * bladeBeamMatrix);
    bladebeam_.draw();

    // Centre de la roue de pales, 0.5 le long du mât
    mat4 rotorCenterMatrix = translate(
        bladeBeamMatrix,
        vec3(0.0f, 0.0f, 0.5f)
    );

    // Animation de la roue autour de l'axe Z
    rotorCenterMatrix = rotate(
        rotorCenterMatrix,
        rotorAngle,
        vec3(0.0f, 0.0f, 1.0f)
    );

    for (int i = 0; i < 4; i++){
    
        mat4 bladeRootMatrix = rotorCenterMatrix;

        // Une pale à chaque 90degrés autour du centre de l'axe z
        bladeRootMatrix = rotate(
            bladeRootMatrix,
            radians(90.0f * i),
            vec3(0.0f, 0.0f, 1.0f)
        );

        // Placer la blade à la verticale 
        bladeRootMatrix = rotate(
            bladeRootMatrix,
            radians(90.0f),
            vec3(1.0f, 0.0f, 0.0f)
        );

        drawCompleteBlade(projView, bladeRootMatrix);
    }

    // Le mat tourne autour de l'axe Y a cinq fois la vitesse des pales.
    const float mainBeamAngle = 5.0f * rotorAngle;
    mat4 mainBeamMatrix = rotate(
        windmillMatrix,
        mainBeamAngle,
        vec3(0.0f, 1.0f, 0.0f)
    );
    sendMatrixToShaders(projView * mainBeamMatrix);
    mainbeam_.draw();

    // La meule suit le bas du mat principal. Le deplacement lateral est
    // negatif pour la placer du cote de l'extension du modele du mat.
    mat4 millstoneMatrix = translate(
        mainBeamMatrix,
        vec3(-0.48f, 0.15f, 0.0f)
    );

    // La meule roule autour de x et 2.27 fois plus vite que le mat.
    millstoneMatrix = rotate(
        millstoneMatrix,
        2.27f * mainBeamAngle,
        vec3(1.0f, 0.0f, 0.0f)
    );
    sendMatrixToShaders(projView * millstoneMatrix);
    millstone_.draw();



}
    
