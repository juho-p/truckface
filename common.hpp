#pragma once

// some sensible defaults

#include <iostream>
#include <array>
#include <string>
#include <memory>
#include <algorithm>
#include <cassert>
#include <glm/glm.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::array;
using std::string;
using std::unique_ptr;
using std::move;

class NoCopy {
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;

    public:
    NoCopy() {}
};

typedef unsigned int ObjectId;
