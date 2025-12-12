///////////////////////////////////////////////////////////////////////////////
// scenemanager.h
// ============
// Manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
// AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
// Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ShaderManager.h"
#include "ShapeMeshes.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <cstdint> // For uint32_t

/***********************************************************
 *  SceneManager
 *
 *  This class contains the code for preparing and rendering
 *  3D scenes, including the shader settings.
 ***********************************************************/
class SceneManager
{
public:
    // Constructor
    SceneManager(ShaderManager* pShaderManager);
    // Destructor
    ~SceneManager();

    struct TEXTURE_INFO
    {
        std::string tag;
        uint32_t ID;
    };

    struct OBJECT_MATERIAL
    {
        glm::vec3 diffuseColor;
        glm::vec3 specularColor;
        float shininess;
        std::string tag;
    };

private:
    // Pointer to shader manager object
    ShaderManager* m_pShaderManager;
    // Pointer to basic shapes object
    ShapeMeshes* m_basicMeshes;
    // Total number of loaded textures
    int m_loadedTextures;
    // Loaded textures info
    TEXTURE_INFO m_textureIDs[16];
    // Defined object materials
    std::vector<OBJECT_MATERIAL> m_objectMaterials;

    // Load texture images and convert to OpenGL texture data
    bool CreateGLTexture(const char* filename, std::string tag);
    // Bind loaded OpenGL textures to slots in memory
    void BindGLTextures();
    // Free the loaded OpenGL textures
    void DestroyGLTextures();
    // Find a loaded texture by tag
    int FindTextureID(std::string tag);
    int FindTextureSlot(std::string tag);
    // Find a defined material by tag
    bool FindMaterial(std::string tag, OBJECT_MATERIAL& material);

    // Set the transformation values into the transform buffer
    void SetTransformations(
        glm::vec3 scaleXYZ,
        float XrotationDegrees,
        float YrotationDegrees,
        float ZrotationDegrees,
        glm::vec3 positionXYZ);

    // Set the color values into the shader
    void SetShaderColor(
        float redColorValue,
        float greenColorValue,
        float blueColorValue,
        float alphaValue);

    // Set the texture data into the shader
    void SetShaderTexture(
        std::string textureTag);

    // Set the UV scale for the texture mapping
    void SetTextureUVScale(
        float u, float v);

    // Set the object material into the shader
    void SetShaderMaterial(
        std::string materialTag);

public:
    // Prepare the 3D scene for rendering
    void PrepareScene();
    // Render the objects in the 3D scene
    void RenderScene();

    // Load all of the needed textures before rendering
    void LoadSceneTextures();
    // Define all the object materials before rendering
    void DefineObjectMaterials();
    // Add and define the light sources before rendering
    void SetupSceneLights();

    // Methods for rendering the individual objects in the 3D scene
    void RenderTable();       // Rectangular table
    void RenderPercolator();      // Coffee Percolator
    void RenderBackdrop();    // Backdrop
    void RenderBook();       // Books on the round table
    void RenderCoffeeCup();   // Coffee cup on the round table
    void RenderTray();          // Tray for Pot and Mug
    void RenderFlowerPot();    // Small flower pot

};
