//
// Created by fangl on 2023/11/22.
//

#pragma once

#include "core/m1k_device.hpp"
#include "core/m1k_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>
#include <memory>


namespace m1k {

class M1kModel {
   public:
    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal{};
        glm::vec2 uv{};

        static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        bool operator==(const Vertex& other) const {
            return position == other.position &&
                    color == other.color &&
                    normal == other.normal &&
                    uv == other.uv;
        }
    };

    struct Builder {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices{};

        void loadModel(const std::string &filepath);
    };

    M1kModel(M1kDevice& device, const M1kModel::Builder &builder);
    ~M1kModel();

    M1kModel(const M1kModel&) = delete;
    M1kModel operator=(const M1kModel&) = delete;

    static std::unique_ptr<M1kModel> createModelFromFile(M1kDevice &device, const std::string &filepath);

    void bind(VkCommandBuffer command_buffer);
    void draw(VkCommandBuffer command_buffer);

   private:
    void createVertexBuffers(const std::vector<Vertex> &vertices);
    void createIndexBuffers(const std::vector<uint32_t> &indices);

    M1kDevice& m1K_device_;

    std::unique_ptr<M1kBuffer> vertex_buffer_;
    uint32_t vertex_count_;

    bool has_index_buffer_{false};
    std::unique_ptr<M1kBuffer> index_buffer_;
    uint32_t index_count_;
};

}
