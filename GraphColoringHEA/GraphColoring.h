/**
*   usage : 1. construct the GraphColoring object
*           2. call init() to set argument of the program and generate initial solutions
*           2. call solve() to find solution
*           3. call print() or appendResultToSheet() to record solution
*
*   note :  1.
*/

#ifndef GRAPH_COLORING_H


#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "../CPPutilibs/Timer.h"
#include "../CPPutilibs/Random.h"
#include "../CPPutilibs/RangeRand.h"


class GraphColoring
{
public:     // public types and constant
    typedef int Color;

    // index of each vertex adjacent to certain vertex
    typedef std::vector<int> AdjVertex;
    // adjacent vertex for all vertices
    typedef std::vector< AdjVertex > AdjVertexList;

    // color ranged in [0,colorNum) for all vertices
    typedef std::vector<Color> VertexColor;

    // number of each color adjacent to certain vertex
    typedef std::vector<int> AdjColor;
    // adjacent color for all vertices
    typedef std::vector<AdjColor> AdjColorTable;

    const int MAX_CONFLICT;

    // total color number
    const int colorNum;

private:    // private types
    class Solution
    {
    public:
        // generate color for each node randomly
        Solution( const GraphColoring &rgc );

        void tabuSearch();

        // return color conflict
        int evalutate() const { return conflict; }

        // convert to output format
        operator VertexColor() { return vertexColor; }

    private:
        const GraphColoring &gc;
        int conflict;

        VertexColor vertexColor;
        AdjColorTable adjColorTab;

        AdjColorTable tabu;   // tabu a vertex changes to a color
    };

public:     // solving procedure
    GraphColoring( const AdjVertexList &adjVertexList, int colorNum );

    void init();
    void solve();

    // return color conflict pair number
    int check() const;     // check optima
    // return color conflict pair number
    int check( const VertexColor &vertexColor ) const;
    // log to console
    void print() const;
    // log to file ( require ios::app flag or "a" mode )
    static void initResultSheet( std::ofstream &csvFile );
    void appendResultToSheet( const std::string &instanceFileName,
        std::ofstream &csvFile ) const;  // contain check()

private:    // functional procedure
    void genInitPopulation( int size ); // contain optima recording

private:    // data member
    AdjVertexList adjVertexList;

    // solution and output
    std::vector<Solution> population;
    VertexColor optimaVertexColor;
    int minConflict;

    // information for log
    int iterCount;
    int generationCount;
    Timer timer;
    // information about the algorithm (initialized in init())
    std::string SOLVING_ALGORITHM;
    int POPULATION_SIZE;
    int MAX_GENERATION_COUNT;
    int MAX_NO_IMPROVE_COUNT;
};



#define GRAPH_COLORING_H
#endif
