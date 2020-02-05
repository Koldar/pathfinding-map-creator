#ifndef _PATHFINDING_MAP_CREATOR_GLOBALS_HEADER__
#define _PATHFINDING_MAP_CREATOR_GLOBALS_HEADER__

#include <boost/filesystem.hpp>

#include <cpp-utils/Random.hpp>

namespace map_creator {

    using namespace cpp_utils;

    /**
     * @brief contains all the global variables
     * 
     */
    class Globals {
    public:
        unsigned int randomSeed;
        boost::filesystem::path outputMainDirectory;
        std::string generatedMapPyString;
        std::string generationMethod;
        boost::filesystem::path baseMapPath;
        size_t width;
        size_t height;
        std::string untraversableCellPyString;

        Random randomGenerator;
    };

}

#endif