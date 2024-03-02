//
// Created by fangl on 2024/3/2.
//

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm/vec2.hpp"
#include "glm/vec3.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>

static uint32_t texture_id = 0;
std::unordered_map<uint32_t, std::string> global_textures_path;


struct Vertex {
    glm::vec3 position{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec3 tangent{};
    glm::vec2 uv{};

    bool operator==(const Vertex& other) const {
        return position == other.position &&
               color == other.color &&
               normal == other.normal &&
               tangent == other.tangent &&
               uv == other.uv;
    }
};


void loadMaterialTextures(aiMaterial* mat,
                                 aiTextureType type,
                                 std::string type_name)
{
    for(uint32_t i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        auto texture_string = mat->GetName();

        std::cout << "M1k::TEST::===" << type_name << "===" << str.C_Str() << std::endl;

        global_textures_path[texture_id++] = str.C_Str();
    }
}


std::vector<std::string> processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<Vertex> vertices{};
    std::vector<uint32_t> indices{};
    std::vector<std::string> textures{};

    for(uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        if(mesh->mTangents) {
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.tangent = vector;
        }

        if(mesh->mColors[0]) {
            vector.x = mesh->mColors[0][i].r;
            vector.y = mesh->mColors[0][i].g;
            vector.z = mesh->mColors[0][i].b;
            vertex.color = vector;
        } else {
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);
        }

        if(mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.uv = vec;
        } else {
            vertex.uv = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for(uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    if(mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // diffuse texture
        loadMaterialTextures(material,
                             aiTextureType_DIFFUSE,
                             "diffuse_texture");

        // base color texture
        loadMaterialTextures(material,
                             aiTextureType_BASE_COLOR,
                             "base_color_texture");

        // normal texture
        loadMaterialTextures(material,
                             aiTextureType_NORMALS,
                             "normal_texture");

        // roughness —— 根据gltf2.0流程，一般是在绿色通道
        loadMaterialTextures(material,
                             aiTextureType_DIFFUSE_ROUGHNESS,
                             "roughness_texture");

        // metalness
        loadMaterialTextures(material,
                             aiTextureType_METALNESS,
                             "metalness_texture");

        // ambient occlusion
        loadMaterialTextures(material,
                             aiTextureType_AMBIENT_OCCLUSION,
                             "AO_texture");

        // light map
        loadMaterialTextures(material,
                             aiTextureType_LIGHTMAP,
                             "lightmap_texture");

        // emissive map
        loadMaterialTextures(material,
                             aiTextureType_EMISSIVE,
                             "emissive_texture");

        // unknow
        loadMaterialTextures(material,
                             aiTextureType_UNKNOWN,
                             "unknown_texture");

        aiColor3D emissive_inten(0.0f,0.0f,0.0f);
        material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive_inten);
        float metallic_factor = 1.0f;
        float roughness_factor = 1.0f;
        float occlusion_factor = 1.0f;
        material->Get(AI_MATKEY_METALLIC_FACTOR, metallic_factor);
        material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness_factor);

    }

    return textures;
}


void processNode(aiNode *node, const aiScene *scene) {
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene);
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}


int main() {
    std::string flightHelmet_file_path = "D:/File/Repository/ToyProgram/M1kanN-Vulkan-Renderer-Engine/assets/models/glTF/FlightHelmet/glTF/FlightHelmet.gltf";
    std::string boomBox_file_path = "D:/File/Repository/ToyProgram/M1kanN-Vulkan-Renderer-Engine/assets/models/glTF/BoomBox/glTF/BoomBox.gltf";
    std::string damagedHelmet_file_path = "D:/File/Repository/ToyProgram/M1kanN-Vulkan-Renderer-Engine/assets/models/glTF/DamagedHelmet/glTF/DamagedHelmet.gltf";

    std::string test_file_path = boomBox_file_path;

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(test_file_path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return 0;
    }

    std::string model_directory_path = test_file_path.substr(0, test_file_path.find_last_of('/'));
    std::cout << "INFO::ASSIMP::model directory: " << model_directory_path << std::endl;

    processNode(scene->mRootNode, scene);
}






