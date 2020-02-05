#include "CLI11.hpp"

#include <cpp-utils/system.hpp>
#include <cpp-utils/filesystem.hpp>
#include <cpp-utils/adjacentGraph.hpp>
#include <cpp-utils/igraph.hpp>
#include <cpp-utils/Random.hpp>
#include <cpp-utils/Color.hpp>

#include <pathfinding-utils/GridMap.hpp>
#include <pathfinding-utils/MovingAIGridMapReader.hpp>
#include <pathfinding-utils/GridBranching.hpp>
#include <pathfinding-utils/GridMapGraphConverter.hpp>

#include "Globals.hpp"

using namespace map_creator;
using namespace cpp_utils;
using namespace cpp_utils::graphs;
using namespace pathfinding;
using namespace pathfinding::maps;

GridMap createGridMap(const std::string& mapName, const MovingAIGridMapReader& reader, const Globals& globals);

GridMap createGridMap(const std::string& mapName, const MovingAIGridMapReader& reader, const Globals& globals) {
    if (!globals.baseMapPath.empty()) {
        return reader.load(globals.baseMapPath);
    } else {
        return GridMap{
            mapName, 
            cpp_utils::vectorplus{globals.width*globals.height, '.'}, 
            globals.width,
            globals.height,
            reader.getTerrainCostMap(), 
            reader.getTerrainColorMap()
        };
    }
}

/**
 * @brief Create a gridmap by altering the given one.
 * 
 * The alteration is as follows: we just choose some random cells and make them untraversable.
 * No check is perform to ensure that the grah remain strongly connected
 * 
 * @param gridMap the grid map involved
 * @param gridMapGraph  the graph representing the map
 * @param cellsToObstruct number fo cells to obstruct
 * @return GridMap 
 */
GridMap createRandomGridMap(const GridMap& gridMap, const IImmutableGraph<std::string, xyLoc, cost_t>& gridMapGraph, int cellsToObstruct) {
    auto result{gridMap};

    auto traversableCells = gridMap.getTraversableCells();
    vectorplus<xyLoc> cellsObstructed{};

    for (int cellId = 0; cellId < cellsToObstruct; ++cellId) {
        auto index = traversableCells.getRandomIndex();
        cellsObstructed.add(traversableCells.at(index));
        traversableCells.removeAt(index);
    }

    for (auto loc : cellsObstructed) {
        info("obstructing", loc);
        result.setCellTerrain(loc, '@');
    }

    return result;
}

int main(int argc, const char* args[]) {

    Globals globals;

	CLI::App app{
        "Allows you to generate path finding maps. Right now gridmaps are the only one allowed",
        "pathfinding-map-creator"
    };

    app.add_option("--random-seed", globals.randomSeed,
		"the seed for any pseudo random generator."
	)->required();
	app.add_option("--output-main-directory", globals.outputMainDirectory,
		"The directory every MAIN output file will be positioned into. Main output files are:"
		" - map"
	)->required();
    app.add_option("--generated-map-pystring", globals.generatedMapPyString,
        "Name of the map to generate"
        " - METHOD: the method we use to perturbate the map"
    )->required();
    app.add_option("--generation-method", globals.generationMethod,
        "How we are going to generate the map. Allowed values are:"
        " - RANDOM: we are going to create the map by random"
    )->required();
    app.add_option("--base-map-path", globals.baseMapPath,
        "If present, it is the path to a preexistent map we need to load and generated perturbations from."
        "If absent, we will build the map from scratch."
    );
    app.add_option("--width", globals.width, 
        "If base-map-path is unspecified, the width of the gridmap to build"
    );
    app.add_option("--height", globals.height, 
        "If base-map-path is unspecified, the height of the gridmap to build"
    );
    app.add_option("--untraversable-cell-pystring", globals.untraversableCellPyString, 
        "If generation-method is RANDOM, the number of cells to randomly make untraversable"
        " - WIDTH: width of the map"
        " - HEIGHT: height of the map"
        " - TOTALCELL: number of traversable cells in the map"
    );

    CLI11_PARSE(app, argc, args);

    // ********************************************************************
	// *********************** SET DERIVED GLOBAL VALUES ******************
	// ********************************************************************

	globals.randomGenerator = Random{globals.randomSeed};

    auto mapName = cpp_utils::callPyEvalWithoutEval(globals.generatedMapPyString,
        "METHOD", globals.generationMethod
    );
    auto mapToGeneratePath = cpp_utils::join(globals.outputMainDirectory, scout(mapName, ".map"));

    MovingAIGridMapReader reader{'.', 1000, color_t::WHITE};
    reader.addTerrain('T', 1500, color_t::GREEN);
    reader.addTerrain('S', 2000, color_t::CYAN);
    reader.addTerrain('W', 2500, color_t::BLUE);
    reader.addTerrain('@', cost_t::INFTY, color_t::BLACK);

    pathfinding::maps::GridMap baseMapGridMap{createGridMap(
        mapName,
        reader,
        globals
    )};
    GridMapGraphConverter converter{GridBranching::EIGHT_CONNECTED};
    auto baseMapGraph = converter.toGraph(baseMapGridMap);

    // ********************************************************************
	// ************************** CREATE GRIDMAP **************************
	// ********************************************************************

    if (globals.generationMethod == "RANDOM") {
        auto cellsToObstruct = cpp_utils::callPyEvalAndCastNumberTo<int>(globals.untraversableCellPyString,
            "WIDTH", baseMapGridMap.getWidth(),
            "HEIGHT", baseMapGridMap.getHeight(),
            "TOTALCELL", baseMapGridMap.getSize()
        );


        auto result = createRandomGridMap(
            baseMapGridMap, *baseMapGraph,
            cellsToObstruct
        );

        reader.save(result, mapToGeneratePath);
    } else {
        throw cpp_utils::exceptions::makeInvalidArgumentException("invalid generation method", globals.generationMethod);
    }


    // ********************************************************************
	// ****************************** SAVE MAP ****************************
	// ********************************************************************

}