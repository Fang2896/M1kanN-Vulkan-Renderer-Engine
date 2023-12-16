//
// Created by fangl on 2023/11/26.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>



namespace m1k {

class M1kCamera {
   public:
    void setOrthographicProjection (
        float left, float right, float top, float bottom, float near, float far);

    void setPerspectiveProjection (
        float fovy, float aspect, float near, float far);
    void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
    void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{0.0f, -1.0f, 0.0f});
    void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

    const glm::mat4& getProjection() const { return projection_matrix_; }
    const glm::mat4& getView() const { return view_matrix_; }
    const glm::mat4& getViewInverse() const { return inverse_view_matrix_; }
    const glm::vec3 getPosition() const { return glm::vec3(inverse_view_matrix_[3]); }

   private:
    glm::mat4 projection_matrix_{1.0f};
    glm::mat4 view_matrix_{1.0f};
    glm::mat4 inverse_view_matrix_{1.0f};

};



}

