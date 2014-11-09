/**
*   usage : 1. construct the GraphColoring object
*           2. call init() to set argument of the program and generate initial solutions
*           2. call solve() to find solution
*           3. call print() or appendResultToSheet() to record solution
*
*   algorithm:
*           1. generate POPULATION_SIZE individuals for initial population.
*           2. do tabu search on each individual.
*           3. if there is an individual without conflict, [END].
*               else :
*               4. select one individual randomly as first parent.
*               5. select one of the best individuals as second parent.
*               6. combine two parents to generate the offspring.
*               7. do local search on offspring.
*               8. if the offspring gets no conflict, [END].
*                   else :
*                   9. select the worst individual in the population.
*                   10. if it is worse than the offspring, replace it with offspring.
*                       else if the population size is smaller than (2 * POPULATION_SIZE), just add the offspring to population.
*                       else shrink the population size to POPULATION_SIZE.
*                    11. loop to 4.
*
*   note :  1. MAX_CONFLICT = vertexNum * vertexNum which may overflow if there are too many vertices.
*           2.
*/

#ifndef GRAPH_COLORING_H


#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>

#include "../CPPutilibs/Timer.h"
#include "../CPPutilibs/Random.h"
#include "../CPPutilibs/RangeRand.h"
#include "../CPPutilibs/RandSelect.h"


class GraphColoring
{
public:     // public types and constant
    typedef int Color;

    typedef std::set<int> VertexSet;

    // index of each vertex adjacent to certain vertex
    typedef std::vector<int> AdjVertex;
    // adjacent vertex for all vertices
    typedef std::vector< AdjVertex > AdjVertexList;

    // color ranged in [0,colorNum) for all vertices
    typedef std::vector<Color> VertexColor;
    // vertex ranged in [0,vertexNum) for all colors
    typedef std::vector<VertexSet> ColorVertex;

    // number of each color adjacent to certain vertex
    typedef std::vector<int> AdjColor;
    // adjacent color for all vertices
    typedef std::vector<AdjColor> AdjColorTable;

    const int MAX_CONFLICT; // calculated by vertex number
    const int vertexNum;    // total vertex number
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
        Solution( const GraphColoring *pgc, const VertexColor &vc );
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
        // compare the conflictEdgeNum (the less is the better)
        friend bool operator<(const Solution &l, const Solution &r)
        {
            return (l.evaluate() < r.evaluate());
        }
        friend bool operator==(const Solution &l, const Solution &r)
        {
            return (l.evaluate() == r.evaluate());
        }

        // convert to output format
        operator Output() const { return Output( conflictEdgeNum, vertexColor ); }
        // convert to the format of each color get which vertices
        operator ColorVertex() const;

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
        int populationSize = 1, int mutateIndividualNum = 0 );
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
    VertexSet selectParents();
    Solution combineParents( const VertexSet &parents );
    bool updateOptima( const Solution &sln );   // return true if there is no conflict
    void updatePopulation( const Solution &offspring );
    void mutateIndividuals( int mutateIndividualNum );

    static VertexColor genRandomColorAssign( int vertexNum, int colorNum );

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
    int MUTATE_INDIVIDUAL_NUM;
};



#define GRAPH_COLORING_H
#endif
