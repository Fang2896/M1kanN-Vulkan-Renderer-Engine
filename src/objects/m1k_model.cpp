//
// Created by fangl on 2023/11/22.
//

#include "m1k_model.hpp"


namespace m1k {

M1kModel::M1kModel(M1kDevice& device,
                   M1kDescriptorSetLayout &set_layout,
                   M1kDescriptorPool &pool,
                   const std::string& filepath)
    : m1K_device_(device), descriptor_set_layout_(set_layout), descriptor_pool_(pool)
{
    std::string default_texture_path = "../assets/textures/NULL.png";
    textures_["NULL"] = std::make_shared<M1kTexture>(m1K_device_,
                                                        default_texture_path);

    loadModel(filepath);
}

M1kModel::~M1kModel() = default;

void M1kModel::draw(VkCommandBuffer command_buffer, VkPipelineLayout& pipeline_layout) {
    for(auto& mesh : meshes_) {
        mesh.bind(command_buffer, pipeline_layout);
        mesh.draw(command_buffer);
    }
}

void M1kModel::loadModel(const std::string& filepath) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filepath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }
    model_directory_path_ = filepath.substr(0, filepath.find_last_of('\\'));
    processNode(scene->mRootNode, scene);
}


void M1kModel::processNode(aiNode *node, const aiScene *scene) {
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        meshes_.push_back(std::move(processMesh(mesh, scene)));
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

M1kMesh M1kModel::processMesh(aiMesh *mesh, const aiScene *scene) {
    std::vector<M1kVertex> vertices{};
    std::vector<uint32_t> indices{};
    M1kMaterialSet material_set;

    bool has_tangent = false;
    bool has_uvs = false;

    uint32_t flags = 0;

    for(uint32_t i = 0; i < mesh->mNumVertices; i++) {
        M1kVertex vertex;
        glm::vec3 vector;

        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.position = vector;

        vector.x = mesh->mNormals[i].x;
        vector.y = mesh->mNormals[i].y;
        vector.z = mesh->mNormals[i].z;
        vertex.normal = vector;

        if(mesh->mTangents != nullptr) {
            has_tangent = true;
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
            has_uvs = true;
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.uv = vec;
        } else {
            vertex.uv = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    if(has_tangent) flags |= 1 << 5;
    if(has_uvs)     flags |= 1 << 6;

    for(uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for(uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    if(mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        // base color texture
        material_set.baseColor_texture = loadMaterialTextures(material,
                             aiTextureType_BASE_COLOR,
                             "base_color_texture");

        // roughness and metalness
        auto roughness_texture =
            loadMaterialTextures(material,
                                 aiTextureType_DIFFUSE_ROUGHNESS,
                                 "roughness_metalness_texture");
        auto metalness_texture = loadMaterialTextures(material,
                                  aiTextureType_METALNESS,
                                  "metalness_texture");
        if(metalness_texture->getTextureFilePath() !=  roughness_texture->getTextureFilePath())
            std::cout << "M1K::WARN::Roughness and Metalness texture are NOT in the same image! Choose roughness one." << std::endl;
        material_set.roughness_metalness_texture = roughness_texture;

        // occlusion (in Assimp, occlusion map equals to light map!)
        material_set.occlusion_texture =
            loadMaterialTextures(material,
                                 aiTextureType_LIGHTMAP,
                                 "light_map_texture");
//        material_set.occlusion_texture =
//            loadMaterialTextures(material,
//                                  aiTextureType_AMBIENT_OCCLUSION,
//                                  "AO_texture");

        // emissive texture
        material_set.emissive_texture =
            loadMaterialTextures(material,
                                 aiTextureType_EMISSIVE,
                                 "emissive_texture");

        // normal texture
        material_set.normal_texture =
            loadMaterialTextures(material,
                                 aiTextureType_NORMALS,
                                 "normal_texture");

        aiColor4D temp_BC(1.0f,1.0f,1.0f,1.0f);
        material->Get(AI_MATKEY_BASE_COLOR, temp_BC);
        material_set.base_color_factor = glm::vec4(temp_BC.r,temp_BC.g,temp_BC.b,temp_BC.a);

        aiColor3D temp_EI(1.0f,1.0f,1.0f);
        material->Get(AI_MATKEY_COLOR_EMISSIVE, temp_EI);
        material_set.emissive_factor = glm::vec3(temp_EI.r, temp_EI.g, temp_EI.b);

        material->Get(AI_MATKEY_METALLIC_FACTOR, material_set.metallic_factor);
        material->Get(AI_MATKEY_ROUGHNESS_FACTOR, material_set.roughness_factor);
        // TODO: Assimp没有提供访问 occlusion strength 的特性！
    }

    flags |= material_set.getTextureFlags();

    return M1kMesh(m1K_device_,
                   descriptor_set_layout_,
                   descriptor_pool_,
                   vertices, indices,
                   material_set, flags);
}

std::shared_ptr<M1kTexture> M1kModel::loadMaterialTextures(aiMaterial* mat,
                                  aiTextureType type,
                                  std::string type_name)
{
    std::shared_ptr<M1kTexture> texture = nullptr;
    uint32_t texture_num = mat->GetTextureCount(type);

    // TODO: gltf2.0 可以指定更多关于texture的选项，比如min mag Filter， warpT S
    if(texture_num > 0) {
        aiString texture_path;
        mat->GetTexture(type, 0, &texture_path);
        std::string std_texture_path = model_directory_path_ + "/" + texture_path.C_Str();
        if(textures_.find(std_texture_path) == textures_.end()) {   // 不存在
            texture = std::make_shared<M1kTexture>(m1K_device_, std_texture_path);
            textures_[std_texture_path] = texture; // 保证不会有重复的Texture
        } else {
            texture = textures_[std_texture_path];
        }
    } else {
        texture = textures_["NULL"];
    }

    if(texture_num > 1) {
        std::cout << "M1K::WARN::" << type_name << " has more than ONE texture." << std::endl;
    }

    return texture;
}


}



