//
// Created by fangl on 2023/11/10.
//

#include "m1k_application.hpp"


namespace m1k {

void M1kApplication::run() {
    while(!m1kWindow_.shouldClose()) {
        glfwPollEvents();
    }
}


}
