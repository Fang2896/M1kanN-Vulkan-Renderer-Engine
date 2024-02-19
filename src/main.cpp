//
// Created by fangl on 2023/11/10.
//

#include "m1k_application.hpp"


// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>


int main() {
    m1k::M1kApplication app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
