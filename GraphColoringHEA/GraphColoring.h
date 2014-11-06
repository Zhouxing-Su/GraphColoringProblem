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
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "../CPPutilibs/Timer.h"
#include "../CPPutilibs/Random.h"
#include "../CPPutilibs/RangeRand.h"
#include "../CPPutilibs/RandSelect.h"


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

    typedef std::set<int> ParentSet;

    const int MAX_CONFLICT; // calculated by vertex number
    const int colorNum;     // total color number

    struct Output
    {
    public:
        Output( int c, const VertexColor &vc = VertexColor() )
            :conflictEdgeNum( c ), vertexColor( vc )
        {
        }

        int conflictEdgeNum;
        VertexColor vertexColor;
    };

private:    // private types
    class Solution
    {
    public:
        struct ConflictReduce
        {
        public:
            ConflictReduce( int r, int v = 0, int d = 0 )
                : reduce( r ), vertex( v ), desColor( d )
            {
            }

            int reduce;     // positive value if improved
            int vertex;
            int desColor;
        };

        // generate color for each node randomly
        Solution( const GraphColoring *pgc );
        // copy solution and reset the tabu table
        Solution( const Solution &s );
        // copy solution and reset the tabu table
        Solution& operator=(const Solution &s);

        // search until local optima is found, then return iteration count
        // (the object will be the optima in the search path after this is called)
        int localSearch( int maxIterCount );
        // search until maxIterCount is meet, then return iteration count
        // (the object will be the optima in the search path after this is called)
        int tabuSearch( int maxIterCount, int tabuTenureBase );

        // return color conflictEdgeNum
        int evaluate() const { return conflictEdgeNum; }

        // convert to output format
        operator Output() const { return Output( conflictEdgeNum, vertexColor ); }

    private:
        const GraphColoring *gc;  // avoid deep copy
        int conflictEdgeNum;

        VertexColor vertexColor;
        AdjColorTable adjColorTab;

        AdjColorTable tabu;   // tabu a vertex changes to a color
    };

public:     // solving procedure
    GraphColoring( const AdjVertexList &adjVertexList, int colorNum );

    // set arguments of the algorithm and generate the initial population
    void init( int tabuTenureBase = 0,
        int maxGenerationCount = 1000, int maxIterCount = 10000,
        int populationSize = 1 );
    // find the optima and record it to attribute "optima".
    void solve();

    // return color conflictEdgeNum number
    int check() const;     // check optima
    // return color conflictEdgeNum number
    int check( const VertexColor &vertexColor ) const;
    // log to console
    void print() const;
    // log to file ( require ios::app flag or "a" mode )
    static void initResultSheet( std::ofstream &csvFile );
    void appendResultToSheet( const std::string &instanceFileName,
        std::ofstream &csvFile ) const;  // contain check()

private:    // functional procedure
    void genInitPopulation( int size ); // contain optima recording
    ParentSet selectParents();
    Solution combineParents( const ParentSet &parents );
    bool updateOptima( const Solution &sln );   // return true if there is no conflict
    void updatePopulation( const Solution &offspring );

private:    // attribute
    AdjVertexList adjVertexList;

    // solution and output
    std::vector<Solution> population;
    Output optima;

    // information for log
    int iterCount;
    int generationCount;
    Timer timer;
    // information about the algorithm (initialized in init())
    std::string SOLVING_ALGORITHM;
    int TABU_TENURE_BASE;
    int POPULATION_SIZE;
    int MAX_GENERATION_COUNT;
    int MAX_ITERATION_COUNT;
};



#define GRAPH_COLORING_H
#endif
