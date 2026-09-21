#include <cstddef>
#include <cstdint>

#include <array>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "happly.h"
#include <imgui/imgui.h>

#include <inf2705/OpenGLApplication.hpp>

#include "model.hpp"
#include "windmill.hpp"

#define CHECK_GL_ERROR printGLError(__FILE__, __LINE__)

using namespace gl;
using namespace glm;

// TODO: Il est fortement recommandé de définir quelques structs
//       pour représenter les attributs.
//       Faire de même pour représenter une vertex, qui est constitué d'attributs.
//       Cela facilitera l'utilisation et rendra votre code plus clair.
//       Un format entrelacé est recommandé (ordonné par vertex au lieu par attribut).
// struct ... { ... };
struct Position
{
    GLfloat x;
    GLfloat y;
};

struct Color
{
    GLfloat r;
    GLfloat g;
    GLfloat b;
};

struct Vertex
{
    Position position;
    Color color;
};

struct App : public OpenGLApplication
{
    App()
    : nSide_(5)
    , oldNSide_(0)
    , cameraPosition_(0.f, 0.f, 0.f)
    , cameraOrientation_(0.f, 0.f)
    , currentScene_(0)
    , isMouseMotionEnabled_(false)
    {
    }
	
	void init() override
	{
		// Le message expliquant les touches de clavier.
		setKeybindMessage(
			"ESC : quitter l'application." "\n"
			"T : changer de scène." "\n"
			"W : déplacer la caméra vers l'avant." "\n"
			"S : déplacer la caméra vers l'arrière." "\n"
			"A : déplacer la caméra vers la gauche." "\n"
			"D : déplacer la caméra vers la droite." "\n"
			"Q : déplacer la caméra vers le bas." "\n"
			"E : déplacer la caméra vers le haut." "\n"
			"Flèches : tourner la caméra." "\n"
			"Souris : tourner la caméra" "\n"
			"Espace : activer/désactiver la souris." "\n"
		);

		// Config de base.
		
		// TODO: Initialisez la couleur de fond.
        
        // On set la couleur qui servira à clear l'écran au moment de clear l'ancien frame
        glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
       
        // TODO: Partie 2: Activez le test de profondeur (GL_DEPTH_TEST) et
        //       l'élimination des faces arrières (GL_CULL_FACE).

        // Permet de faire en sorte que les objets soient affichés en fonction de leur profondeur et non de l'ordre dans lequel ils sont affichés
        glEnable(GL_DEPTH_TEST);
        // Permet de ne pas render les faces des triangles qui ne sont pas dans l'angle de vue de la caméra
        glEnable(GL_CULL_FACE);

        
        loadShaderPrograms();
        
        // Partie 1
        initShapeData();
        
        // Partie 2
        loadModels();
        
        // TODO: Insérez les initialisations supplémentaires ici au besoin.
        glClear(GL_COLOR_BUFFER_BIT);
	}
	
	    
    void checkShaderCompilingError(const char* name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetShaderiv(id, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(id, 1024, NULL, infoLog);
            glDeleteShader(id);
            std::cout << "Shader \"" << name << "\" compile error: " << infoLog << std::endl;
        }
    }


    void checkProgramLinkingError(const char* name, GLuint id)
    {
        GLint success;
        GLchar infoLog[1024];

        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(id, 1024, NULL, infoLog);
            glDeleteProgram(id);
            std::cout << "Program \"" << name << "\" linking error: " << infoLog << std::endl;
        }
    }
	

	// Appelée à chaque trame. Le buffer swap est fait juste après.
	void drawFrame() override
	{
	    // TODO: Nettoyage de la surface de dessin.
	    // TODO: Partie 2: Ajoutez le nettoyage du tampon de profondeur.
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        ImGui::Begin("Scene Parameters");
        ImGui::Combo("Scene", &currentScene_, SCENE_NAMES, N_SCENE_NAMES);
        ImGui::End();
        
        switch (currentScene_)
        {
            case 0: sceneShape();  break;
            case 1: sceneModels(); break;
        }
	}

	// Appelée lorsque la fenêtre se ferme.
	void onClose() override
	{
	    // TODO: Libérez les ressources allouées (buffers, shaders, etc.).
	}

	// Appelée lors d'une touche de clavier.
	void onKeyPress(const sf::Event::KeyPressed& key) override
	{
		using enum sf::Keyboard::Key;
		switch (key.code)
		{
		    case Escape:
		        window_.close();
	        break;
		    case Space:
		        isMouseMotionEnabled_ = !isMouseMotionEnabled_;
		        if (isMouseMotionEnabled_)
		        {
		            window_.setMouseCursorGrabbed(true);
		            window_.setMouseCursorVisible(false);
	            }
	            else
	            {
	                window_.setMouseCursorGrabbed(false);
	                window_.setMouseCursorVisible(true);
                }
	        break;
	        case T:
                currentScene_ = ++currentScene_ < N_SCENE_NAMES ? currentScene_ : 0;
            break;
		    default: break;
		}
	}

	void onResize(const sf::Event::Resized& event) override
	{	
	}
	
	void onMouseMove(const sf::Event::MouseMoved& mouseDelta) override
	{	    
	    if (!isMouseMotionEnabled_)
	        return;
        
        const float MOUSE_SENSITIVITY = 0.1;
        float cameraMouvementX = mouseDelta.position.y * MOUSE_SENSITIVITY;
        float cameraMouvementY = mouseDelta.position.x * MOUSE_SENSITIVITY;
	    cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;
	}
	
	void updateCameraInput() 
    {
        if (!window_.hasFocus())
            return;
            
        if (isMouseMotionEnabled_)
        {
            sf::Vector2u windowSize = window_.getSize();
            sf::Vector2i windowHalfSize(windowSize.x / 2.0f, windowSize.y / 2.0f);
            sf::Mouse::setPosition(windowHalfSize, window_);
        }
        
        float cameraMouvementX = 0;
        float cameraMouvementY = 0;
        
        const float KEYBOARD_MOUSE_SENSITIVITY = 1.5f;
        
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up))
            cameraMouvementX -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down))
            cameraMouvementX += KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
            cameraMouvementY -= KEYBOARD_MOUSE_SENSITIVITY;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
            cameraMouvementY += KEYBOARD_MOUSE_SENSITIVITY;
        
        cameraOrientation_.y -= cameraMouvementY * deltaTime_;
        cameraOrientation_.x -= cameraMouvementX * deltaTime_;

        // Keyboard input
        glm::vec3 positionOffset = glm::vec3(0.0);
        const float SPEED = 10.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W))
            positionOffset.z -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
            positionOffset.z += SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A))
            positionOffset.x -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D))
            positionOffset.x += SPEED;
            
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Q))
            positionOffset.y -= SPEED;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::E))
            positionOffset.y += SPEED;

        positionOffset = glm::rotate(glm::mat4(1.0f), cameraOrientation_.y, glm::vec3(0.0, 1.0, 0.0)) * glm::vec4(positionOffset, 1);
        cameraPosition_ += positionOffset * glm::vec3(deltaTime_);
    }
    
    void loadModels()
    {
        windmill_.loadModels();
        grass_.load("../models/grass.ply");
    }
    
    GLuint loadShaderObject(GLenum type, const char* path)
    {
        // TODO: Chargement d'un shader object.
        //       Utilisez readFile pour lire le fichier.
        //       N'oubliez pas de vérifier les erreurs suite à la compilation
        //       avec la méthode App::checkShaderCompilingError.

        // Lis l'entièreté du fichier dans un string
        const std::string shaderFile = readFile(path);

        // Converti le std string en GL string type
        const GLchar* glShaderFile = shaderFile.c_str();

        // Créé le shader object
        const GLuint shader = glCreateShader(type);

        // Met le source code de shaderFile dans le shader object
        glShaderSource(shader, 1, &glShaderFile, nullptr);

        // Compile le shader
        glCompileShader(shader);

        // Retourne void, ne peut pas vérifier si erreur il y a
        App::checkShaderCompilingError(path, shader);

  
        return shader;

    }
    
    // Shader program == programme qui contient les shaders à compiler sur le GPU (.glsl)
    void loadShaderPrograms()
    {
        // TODO: Chargement des shader programs.
        //       N'oubliez pas de vérifier les erreurs suite à la liaison (linking)
        //       avec la méthode App::checkProgramLinkingError. Vous pouvez
        //       donner un nom unique pour plus facilement lire les erreurs 
        //       dans la console.
        //       Il est recommandé de détacher et de supprimr les shaders objects
        //       directement après la liaison.
        
        // Partie 1
        const char* BASIC_VERTEX_SRC_PATH = "./shaders/basic.vs.glsl";
        const char* BASIC_FRAGMENT_SRC_PATH = "./shaders/basic.fs.glsl";
        
        // Partie 2
        const char* TRANSFORM_VERTEX_SRC_PATH = "./shaders/transform.vs.glsl";
        const char* TRANSFORM_FRAGMENT_SRC_PATH = "./shaders/transform.fs.glsl";

        // Compiler les shader objects des fichiers glsl.
        const GLuint basicVertexShader = loadShaderObject(GL_VERTEX_SHADER, BASIC_VERTEX_SRC_PATH);
        const GLuint basicFragmentShader = loadShaderObject(GL_FRAGMENT_SHADER, BASIC_FRAGMENT_SRC_PATH);

        // Les attacher au programme, puis effectuer le linking.
        basicSP_ = glCreateProgram();
        glAttachShader(basicSP_, basicVertexShader);
        glAttachShader(basicSP_, basicFragmentShader);
        glLinkProgram(basicSP_);
        checkProgramLinkingError("basic", basicSP_);

        // Le programme lie contient maintenant le code compile : les shader objects
        // ne sont donc plus necessaires.
        glDetachShader(basicSP_, basicVertexShader);
        glDetachShader(basicSP_, basicFragmentShader);
        glDeleteShader(basicVertexShader);
        glDeleteShader(basicFragmentShader);

        // Repeter les memes etapes pour le programme qui applique les transformations.
        const GLuint transformVertexShader = loadShaderObject(GL_VERTEX_SHADER, TRANSFORM_VERTEX_SRC_PATH);
        const GLuint transformFragmentShader = loadShaderObject(GL_FRAGMENT_SHADER, TRANSFORM_FRAGMENT_SRC_PATH);

        transformSP_ = glCreateProgram();
        glAttachShader(transformSP_, transformVertexShader);
        glAttachShader(transformSP_, transformFragmentShader);
        glLinkProgram(transformSP_);
        checkProgramLinkingError("transform", transformSP_);

        glDetachShader(transformSP_, transformVertexShader);
        glDetachShader(transformSP_, transformFragmentShader);
        glDeleteShader(transformVertexShader);
        glDeleteShader(transformFragmentShader);
        
        // TODO: Allez chercher les locations de vos variables uniform dans le shader
        //       pour initialiser mvpUniformLocation_ et windmill_.mvpUniformLocation.
        mvpUniformLocation_ = glGetUniformLocation(transformSP_, "mvp");
        windmill_.mvpUniformLocation = mvpUniformLocation_;
    }
    
    void generateNgon()
    {
        // TODO: Générez un polygone à N côtés (couramment appelé N-gon).
        //       Vous devez gérer les cas entre 5 et 12 côtés (pentagone, hexagone
        //       , etc.). Ceux-ci ont un rayon constant de 0.7.
        //       Chaque point possède une couleur (libre au choix).
        //       Vous devez minimiser le nombre de points et définir des indices
        //       pour permettre la réutilisation.       
        
        vertices_[0].position = { -0.5f, -0.5f };
        vertices_[0].color = { 1.0f, 1.0f, 1.0f };

        vertices_[1] = {
            { 0.5f, -0.5f },
            { 0.0f,  1.0f, 0.0f } // vert
        };

        vertices_[2] = {
            { 0.0f, 0.5f },
            { 0.0f, 0.0f, 1.0f } // bleu
        };

        elements_[0] = 0;
        elements_[1] = 1;
        elements_[2] = 2;
        
        const float RADIUS = 0.7f;
    }
    
    void initShapeData()
    {
        // TODO: Initialisez les objets graphiques pour le dessin du polygone.
        //       Ne passez aucune donnée pour le moment (déjà géré dans App::sceneShape),
        //       on demande seulement de faire l'allocation de buffers suffisamment gros
        //       pour contenir le polygone durant toute l'exécution du programme.
        //       Réfléchissez bien à l'usage des buffers (paramètre de glBufferData).

        // Déclaration du VAO (Vertex Array Object), du VBO (Vertex Buffer Object) et de l'EBO (Element Buffer Object, stocke les indices des sommets).
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);
    

        // Sélectionne le vao à utiliser pour faire sa configuration
        glBindVertexArray(vao_);

        // Sélectionne le vbo à utiliser
        glBindBuffer(GL_ARRAY_BUFFER, vbo_);

        // Initialisation du VBO 
        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * MAX_N_SIDES * 3,nullptr, GL_STATIC_DRAW );


        // Sélectionne l'ebo à utiliser 
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

        // Initialisation de l'EBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * MAX_N_SIDES * 3,nullptr, GL_STATIC_DRAW );

        //Explique comment lire l'attribut position
        glVertexAttribPointer(
            0, // index
            2, // taille
            GL_FLOAT, // type 
            GL_FALSE,
            sizeof(Vertex), // Distance en bytes entre chaque vertex dans le buffer
            reinterpret_cast<void*>(offsetof(Vertex, position))
        );
        glEnableVertexAttribArray(0);

        //Explique comment lire l'attribut couleur
        glVertexAttribPointer(
            1, 3, GL_FLOAT, GL_FALSE,
            sizeof(Vertex),
            reinterpret_cast<void*>(offsetof(Vertex, color))
        );
        glEnableVertexAttribArray(1);

        // TODO: Créez un vao et spécifiez le format des données dans celui-ci.
        //       N'oubliez pas de lier le ebo avec le vao et de délier le vao
        //       du contexte pour empêcher des modifications sur celui-ci.

        glBindVertexArray(0);
    }
    
    void sceneShape()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderInt("Sides", &nSide_, MIN_N_SIDES, MAX_N_SIDES);
        ImGui::End();
        
        bool hasNumberOfSidesChanged = nSide_ != oldNSide_;
        if (hasNumberOfSidesChanged)
        {
            oldNSide_ = nSide_;
            App::generateNgon();
            
            // TODO: Le nombre de côtés a changé, la méthode App::generateNgon
            //       (que vous avez implémentée) a modifié les données sur le CPU.
            //       Ici, il faut envoyer les données à jour au GPU.
            //       Attention, il ne faut pas faire d'allocation/réallocation, on veut
            //       seulement mettre à jour les buffers actuels.

            // Sélectionne le vao
            glBindVertexArray(vao_);

            // Sélectionne le vbo
            glBindBuffer(GL_ARRAY_BUFFER, vbo_);
            
            // Met à jour les vertices du vbo
            glBufferSubData(
                GL_ARRAY_BUFFER,
                0,
                sizeof(Vertex) * (nSide_ + 1),
                vertices_.data()
            );

            // Sélectionne l'ebo            
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);

            // Met à jour les indices dans l'EBO.
            glBufferSubData(
                GL_ELEMENT_ARRAY_BUFFER,
                0,
                sizeof(GLuint) * nSide_ * 3,
                elements_.data()
            );


            oldNSide_ = 1;
        }
        
        // TODO: Dessin du polygone.

        // Sélectionner le programme à utiliser
        glUseProgram(basicSP_);

        // Dessiner
        glDrawElements(GL_TRIANGLES, nSide_ * 3, GL_UNSIGNED_INT, nullptr);

        // Déselectionner le vao une fois modif terminées
        glBindVertexArray(0);
    }
    
    void drawGround(glm::mat4& projView)
    {
        // TODO: Dessin du sol.
        //
        //       Ici, le modèle original est un carré de 1 unité.
        //       
        //       Le gazon a une mise à l'échelle pour être long de 50
        //       unités et large de 50. Celui-ci doit aussi être légèrement
        //       baisé de 0.1.

        // Modele 3D contenu dans grass.ply taille 1x1
        // Il faut baisser ses coordonnées dans le monde de 0.1 on fait ca à partir de la matrice modele
        static const vec3 translationVector = vec3(0.0f,-0.1f,0.0f);
        static const vec3 scalingVector = vec3(50.0f,1.0f,50.0f);
        static const mat4 modelMatrix = scale(translate(mat4(1.0f), translationVector),scalingVector);

        mat4 mvp = projView*modelMatrix;

        glUniformMatrix4fv(
            mvpUniformLocation_,
            1,
            GL_FALSE,                
            value_ptr(mvp) // créé un pointeur vers la matrice mvp
        );


        grass_.draw();

    }
    
    glm::mat4 getViewMatrix()
    {
        // TODO: Calculer la matrice de vue.
        //
        //       Vous n'avez pas le droit d'utiliser de fonction lookAt ou 
        //       d'inversion de matrice. À la place, procéder en inversant
        //       les opérations. N'oubliez pas que cette matrice est appliquée
        //       aux éléments de la scène. Au lieu de déplacer la caméra 10
        //       unités vers la gauche, on déplace le monde 10 unités vers la
        //       droite, ce qui donne le même résultat final.
        //
        //       La caméra est placée à la position cameraPosition et orientée
        //       par les angles cameraOrientation (en radian).

        //glm::rotate(matrice, angle, axe);
        mat4 viewMatrix(1.0f);

        viewMatrix = rotate(viewMatrix, -cameraOrientation_.x,vec3(1.0,0.0,0.0));
        viewMatrix = rotate(viewMatrix, -cameraOrientation_.y,vec3(0.0,1.0,0.0));
        

        vec3 translationVector = vec3(-cameraPosition_.x,-cameraPosition_.y,-cameraPosition_.z);
        
        viewMatrix = translate(viewMatrix,translationVector);

        
        return viewMatrix;
    }
    
    glm::mat4 getPerspectiveProjectionMatrix()
    {
        // TODO: Calculer la matrice de projection.
        //
        //       Celle-ci aura un fov de 70 degrés, un near à 0.1 et un far à 300.
        //       
        
        // getWindowAspect();

        mat4 perspectiveMatrix  = perspective(radians(70.0f),getWindowAspect(),0.1f,300.0f);
        
        return perspectiveMatrix;
    }
    
    void sceneModels()
    {
        ImGui::Begin("Scene Parameters");
        ImGui::SliderFloat("Wind Speed", &windmill_.windSpeed, 0.0f, 20.0f, "%.2f m/s");
        ImGui::SliderFloat("Wind Angle", &windmill_.windAngle, -M_PI, M_PI, "%.2f°");
        ImGui::End();
    
        updateCameraInput();
        windmill_.update(deltaTime_);
        
        // TODO: Dessin de la totalité de la scène graphique.
        //       On devrait voir le gazon et le moulin.
        //       Le moulin est contrôlable avec l'interface graphique.
        glm::mat4 projection = getPerspectiveProjectionMatrix();
        glm::mat4 view = getViewMatrix();
        glm::mat4 projView = projection * view;

        // sélectionner les shaders transform
        glUseProgram(transformSP_);

        drawGround(projView);
        windmill_.draw(projView);
    }
    
private:
    // Shaders
    GLuint basicSP_;
    GLuint transformSP_;
    GLuint mvpUniformLocation_;
    
    // Partie 1
    GLuint vbo_, ebo_, vao_;
    
    static constexpr unsigned int MIN_N_SIDES = 5;
    static constexpr unsigned int MAX_N_SIDES = 12;
    
    // TODO: Modifiez les types de vertices_ et elements_ pour votre besoin.

    // Contient les sommets du polygone
    std::array<Vertex, MAX_N_SIDES + 1> vertices_;

    // Indique quels sommets utiliser pour faire le triangle
    // Chaque element corresponds à un vertex
    std::array<GLuint, MAX_N_SIDES * 3> elements_;
    
    int nSide_, oldNSide_;
    
    // Partie 2
    Model grass_;
    
    Windmill windmill_;
    
    glm::vec3 cameraPosition_;
    glm::vec2 cameraOrientation_;
    
    // Imgui var
    const char* const SCENE_NAMES[2] = {
        "Introduction",
        "3D Model & transformation",
    };
    const int N_SCENE_NAMES = sizeof(SCENE_NAMES) / sizeof(SCENE_NAMES[0]);
    int currentScene_;
    
    bool isMouseMotionEnabled_;
};


int main(int argc, char* argv[])
{
	WindowSettings settings = {};
	settings.fps = 144;
	settings.context.depthBits = 24;
	settings.context.stencilBits = 8;
	settings.context.antiAliasingLevel = 4;
	settings.context.majorVersion = 3;
	settings.context.minorVersion = 3;
	settings.context.attributeFlags = sf::ContextSettings::Attribute::Core;

	App app;
	app.run(argc, argv, "Tp1", settings);
}
