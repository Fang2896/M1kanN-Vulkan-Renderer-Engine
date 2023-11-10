//
// Created by fangl on 2023/11/10.
//

#pragma once

#include "m1k_window.hpp"
#include "m1k_pipeline.hpp"
#include "m1k_device.hpp"

namespace m1k {

class M1kApplication {
   public:
    static constexpr int kWidth = 800;
    static constexpr int kHeight = 600;

    void run();

   private:
    M1kWindow m1kWindow_{kWidth, kHeight, "Hello Vulkan"};
    M1kDevice m1kDevice_{m1kWindow_};
    M1kPipeline m1kPipeline_{m1kDevice_,
                             M1kPipeline::defaultPipelineConfigInfo(kWidth, kHeight),
                             "../shaders/simple_shader.vert.spv",
                            "../shaders/simple_shader.frag.spv"};
};

}
