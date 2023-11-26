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

    const glm::mat4& getProjection() const { return projection_matrix; }

   private:
    glm::mat4 projection_matrix{1.0f};

};



}

