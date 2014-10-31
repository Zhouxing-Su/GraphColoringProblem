#include "GraphColoring.h"

using namespace std;


///=== [ solving procedure ] ===============================

GraphColoring::GraphColoring( const AdjVertexList &avl, int cn )
    : MAX_CONFLICT( avl.size() *avl.size() ), colorNum( cn ),
    adjVertexList( avl ), population(), optimaVertexColor(), minConflict( MAX_CONFLICT ),
    iterCount( 0 ), generationCount( 0 ), timer()
{
    Random::setSeed();
}


void GraphColoring::init()
{
    POPULATION_SIZE = 8;
    MAX_GENERATION_COUNT = static_cast<int>(1E3);
    MAX_NO_IMPROVE_COUNT = static_cast<int>(1E5);

    ostringstream ss;
    ss << "HEA(PS=" << POPULATION_SIZE
        << ')';
    SOLVING_ALGORITHM = ss.str();


    genInitPopulation( POPULATION_SIZE );
}

void GraphColoring::solve()
{
    timer.reset();



    timer.record();
}


void GraphColoring::genInitPopulation( int size )
{
    while (size--) {
        population.push_back( Solution( *this ) );
        int conflict = population.back().evalutate();
        if (minConflict > conflict) {
            minConflict = conflict;
            optimaVertexColor = population.back();
        }
    }
}


///=== [ Solution ] ===============================

GraphColoring::Solution::Solution( const GraphColoring &rgc )
    : gc( rgc ), conflict( 0 ), vertexColor( rgc.adjVertexList.size() ),
    adjColorTab( rgc.adjVertexList.size(), vector<int>( rgc.colorNum, 0 ) ),
    tabu( rgc.adjVertexList.size(), vector<int>( rgc.colorNum, 0 ) )
{
    // assign color for each vertex randomly
    RangeRand rColor( 0, gc.colorNum - 1 );
    for (size_t vertex = 0; vertex < vertexColor.size(); vertex++) {
        vertexColor[vertex] = rColor();
    }

    // generate adjColorTable and evaluate conflict
    for (size_t vertex = 0; vertex < adjColorTab.size(); vertex++) {
        AdjColor &adjColor = adjColorTab[vertex];
        const AdjVertex &adjVertex = gc.adjVertexList[vertex];
        for (size_t adj = 0; adj < adjVertex.size(); adj++) {
            adjColor[vertexColor[adjVertex[adj]]]++;
        }
        conflict += adjColor[vertexColor[vertex]];
    }
    conflict /= 2;
}

void GraphColoring::Solution::tabuSearch()
{

}


///=== [ output ] ===============================

int GraphColoring::check() const
{
    return check( optimaVertexColor );
}

int GraphColoring::check( const VertexColor &vertexColor ) const
{
    int conflict = 0;
    for (size_t vertex = 0; vertex < adjVertexList.size(); vertex++) {
        Color color = vertexColor[vertex];
        const AdjVertex &adjVertex = adjVertexList[vertex];
        for (size_t adj = 0; adj < adjVertex.size(); adj++) {
            if (vertexColor[adjVertex[adj]] == color) {
                conflict++;
            }
        }
    }

    return (conflict / 2);
}

void GraphColoring::print() const
{
    if (check() != minConflict) {
        cout << "[LogicError] ";
    }
    cout << minConflict << endl;
}


void GraphColoring::initResultSheet( std::ofstream &csvFile )
{
    csvFile << "Date, Instance, Algorithm, MAX_ITER_COUNT, MAX_NO_IMPROVE_COUNT, "
        "RandSeed, Duration, IterCount, GenerationCount, Optima, Solution" << std::endl;
}

void GraphColoring::appendResultToSheet(
    const std::string &instanceFileName, std::ofstream &csvFile ) const
{
    if (check() != minConflict) {
        csvFile << "[LogicError] ";
    }

    csvFile << Timer::getLocalTime() << ", "
        << instanceFileName << ", "
        << SOLVING_ALGORITHM << ", "
        << MAX_GENERATION_COUNT << ", "
        << MAX_NO_IMPROVE_COUNT << ", "
        << Random::getSeed() << ", "
        << timer.getTotalDuration() << ", "
        << iterCount << ", "
        << generationCount << ", "
        << minConflict << ", ";

    for (VertexColor::const_iterator iter = optimaVertexColor.begin();
        iter != optimaVertexColor.end(); iter++) {
        csvFile << *iter << ' ';
    }

    csvFile << std::endl;
}
