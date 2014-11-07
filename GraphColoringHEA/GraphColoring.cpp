#include "GraphColoring.h"

using namespace std;


///=== [ solving procedure ] ===============================

GraphColoring::GraphColoring( const AdjVertexList &avl, int cn )
    : MAX_CONFLICT( avl.size() * avl.size() ), colorNum( cn ),
    adjVertexList( avl ), population(), optima( MAX_CONFLICT ),
    iterCount( 0 ), generationCount( 0 ), timer()
{
    Random::setSeed();
}


void GraphColoring::init( int tabuTenureBase,
    int maxGenerationCount, int maxIterCount, int populationSize )
{
    POPULATION_SIZE = populationSize;
    MAX_GENERATION_COUNT = maxGenerationCount;
    MAX_ITERATION_COUNT = maxIterCount;
    TABU_TENURE_BASE = tabuTenureBase;

    ostringstream ss;
    ss << "HEA(PS=" << POPULATION_SIZE
        << "|GC=" << MAX_GENERATION_COUNT
        << "|IC=" << MAX_ITERATION_COUNT
        << "|TB=" << TABU_TENURE_BASE
        << ')';
    SOLVING_ALGORITHM = ss.str();


    genInitPopulation( POPULATION_SIZE );
}

void GraphColoring::solve()
{
    timer.reset();

    for (; generationCount < MAX_GENERATION_COUNT; generationCount++) {
        // select parents
        ParentSet parentSet( selectParents() );

        // combine
        Solution offspring( combineParents( parentSet ) );

        // local search on offspring
        //offspring.localSearch( MAX_ITERATION_COUNT );
        iterCount += offspring.tabuSearch( MAX_ITERATION_COUNT, TABU_TENURE_BASE );

        // update optima and check if there is no conflict
        if (updateOptima( offspring )) {
            break;
        }

        // population update
        updatePopulation( offspring );
    }

    timer.record();
}


void GraphColoring::genInitPopulation( int size )
{
    while (size--) {
        population.push_back( Solution( this ) );
        updateOptima( population.back() );
    }
}

GraphColoring::ParentSet GraphColoring::selectParents()
{
    RangeRand rr( 0, population.size() - 1 );
    ParentSet parents;

    parents.insert( rr() );

    return parents;
}

GraphColoring::Solution GraphColoring::combineParents( const ParentSet &parents )
{
    Solution offspring( population[*parents.begin()] );

    return offspring;
}

bool GraphColoring::updateOptima( const Solution &sln )
{
    if (optima.conflictEdgeNum > sln.evaluate()) {
        optima = sln;
    }
    return (optima.conflictEdgeNum <= 0);
}

void GraphColoring::updatePopulation( const Solution &offspring )
{
    population[0] = offspring;
}

///=== [ Solution ] ===============================

GraphColoring::Solution::Solution( const GraphColoring *pgc )
    : gc( pgc ), conflictEdgeNum( 0 ), vertexColor( pgc->adjVertexList.size() ),
    adjColorTab( pgc->adjVertexList.size(), vector<int>( pgc->colorNum, 0 ) ),
    tabu( pgc->adjVertexList.size(), vector<int>( pgc->colorNum, 0 ) )
{
    // assign color for each vertex randomly
    RangeRand rColor( 0, pgc->colorNum - 1 );
    for (size_t vertex = 0; vertex < vertexColor.size(); vertex++) {
        vertexColor[vertex] = rColor();
    }

    // generate adjColorTable and evaluate conflictEdgeNum
    for (size_t vertex = 0; vertex < adjColorTab.size(); vertex++) {
        AdjColor &adjColor = adjColorTab[vertex];
        const AdjVertex &adjVertex = pgc->adjVertexList[vertex];
        for (size_t adj = 0; adj < adjVertex.size(); adj++) {
            adjColor[vertexColor[adjVertex[adj]]]++;
        }
        conflictEdgeNum += adjColor[vertexColor[vertex]];
    }
    conflictEdgeNum /= 2;
}

GraphColoring::Solution::Solution( const Solution &s )
    :gc( s.gc ), conflictEdgeNum( s.conflictEdgeNum ),
    vertexColor( s.vertexColor ), adjColorTab( s.adjColorTab ),
    tabu( gc->adjVertexList.size(), vector<int>( gc->colorNum, 0 ) )
{
}

GraphColoring::Solution& GraphColoring::Solution::operator=(const Solution &s)
{
    gc = s.gc;
    conflictEdgeNum = s.conflictEdgeNum;
    vertexColor = s.vertexColor;
    adjColorTab = s.adjColorTab;
    tabu = AdjColorTable( gc->adjVertexList.size(), vector<int>( gc->colorNum, 0 ) );
    return *this;
}

int GraphColoring::Solution::localSearch( int maxIterCount )
{
    RandSelect maxReduceSelect;

    int iterCount = 0;
    for (; iterCount < maxIterCount; iterCount++) {
        ConflictReduce maxReduce( 0 );  // positive value if improved

        // find best conflictEdgeNum reduction
        for (size_t v = 0; v < adjColorTab.size(); v++) {
            int color = vertexColor[v];
            AdjColor &ac = adjColorTab[v];
            if (ac[color] > 0) {    // for each vertex with conflictEdgeNum
                for (int c = 0; c < gc->colorNum; c++) {
                    if (c != color) {  // for each destination color
                        int reduce = ac[color] - ac[c];
                        if (reduce > maxReduce.reduce) {
                            maxReduce = ConflictReduce( reduce, static_cast<int>(v), c );
                            maxReduceSelect.reset();
                        } else if ((reduce == maxReduce.reduce) && maxReduceSelect.isSelected()) {
                            maxReduce = ConflictReduce( reduce, static_cast<int>(v), c );
                        }
                    }
                }
            }
        }

        // check if there is a conflictEdgeNum reduction
        if (maxReduce.reduce <= 0) {
            break;
        }

        // apply the conflictEdgeNum reduction
        conflictEdgeNum -= maxReduce.reduce;
        int srcColor = vertexColor[maxReduce.vertex];
        vertexColor[maxReduce.vertex] = maxReduce.desColor;
        const AdjVertex &av = gc->adjVertexList[maxReduce.vertex];
        for (AdjVertex::const_iterator iter = av.begin();
            iter != av.end(); iter++) {
            adjColorTab[*iter][srcColor]--;
            adjColorTab[*iter][maxReduce.desColor]++;
        }
    }

    return iterCount;
}

int GraphColoring::Solution::tabuSearch( int maxIterCount, int tabuTenureBase )
{
    Solution localOptima( *this );

    RandSelect maxReduceSelectT;
    RandSelect maxReduceSelectNT;

    int iterCount = 1;
    for (; iterCount < maxIterCount; iterCount++) {
        // positive value if improved
        ConflictReduce maxReduceT( -gc->MAX_CONFLICT );     // for tabu
        ConflictReduce maxReduceNT( -gc->MAX_CONFLICT );    // for none-tabu

        // find best conflictEdgeNum reduction
        for (int v = 0; static_cast<size_t>(v) < adjColorTab.size(); v++) {
            int color = vertexColor[v];
            AdjColor &ac = adjColorTab[v];
            if (ac[color] > 0) {    // for each vertex with conflictEdgeNum
                for (int c = 0; c < gc->colorNum; c++) {
                    if (c != color) {  // for each destination color
                        int reduce = ac[color] - ac[c];
                        if (tabu[v][c] < iterCount) {
                            if (reduce > maxReduceNT.reduce) {
                                maxReduceNT = ConflictReduce( reduce, v, c );
                                maxReduceSelectNT.reset();
                            } else if ((reduce == maxReduceNT.reduce)
                                && maxReduceSelectNT.isSelected()) {
                                maxReduceNT = ConflictReduce( reduce, v, c );
                            }
                        } else {
                            if (reduce > maxReduceT.reduce) {
                                maxReduceT = ConflictReduce( reduce, v, c );
                                maxReduceSelectT.reset();
                            } else if ((reduce == maxReduceT.reduce)
                                && maxReduceSelectT.isSelected()) {
                                maxReduceT = ConflictReduce( reduce, v, c );
                            }
                        }
                    }
                }
            }
        }

        // check if there is a conflictEdgeNum reduction
        ConflictReduce maxReduce( -gc->MAX_CONFLICT );
        if ((maxReduceNT.reduce < maxReduceT.reduce)
            && ((conflictEdgeNum - maxReduceT.reduce) < localOptima.conflictEdgeNum)) {
            maxReduce = maxReduceT;
        } else {
            maxReduce = maxReduceNT;
        }

        if (maxReduce.reduce <= 0) {
            if (localOptima.conflictEdgeNum > conflictEdgeNum) {    // update local optima
                localOptima = *this;
                if (conflictEdgeNum <= 0) {
                    break;
                }
            }
        }

        // apply the conflictEdgeNum reduction
        conflictEdgeNum -= maxReduce.reduce;
        int srcColor = vertexColor[maxReduce.vertex];
        vertexColor[maxReduce.vertex] = maxReduce.desColor;
        const AdjVertex &av = gc->adjVertexList[maxReduce.vertex];
        for (AdjVertex::const_iterator iter = av.begin();
            iter != av.end(); iter++) {
            adjColorTab[*iter][srcColor]--;
            adjColorTab[*iter][maxReduce.desColor]++;
        }

        // update tabu list
        RangeRand tabuTenurePerturb( 0, 10 );
        tabu[maxReduce.vertex][srcColor] = iterCount + conflictEdgeNum + tabuTenurePerturb();
    }


    // replace the current solution with the local optima
    *this = localOptima;

    return iterCount;
}


///=== [ output ] ===============================

int GraphColoring::check() const
{
    return check( optima.vertexColor );
}

int GraphColoring::check( const VertexColor &vertexColor ) const
{
    int conflictEdgeNum = 0;
    for (size_t vertex = 0; vertex < adjVertexList.size(); vertex++) {
        Color color = vertexColor[vertex];
        const AdjVertex &adjVertex = adjVertexList[vertex];
        for (size_t adj = 0; adj < adjVertex.size(); adj++) {
            if (vertexColor[adjVertex[adj]] == color) {
                conflictEdgeNum++;
            }
        }
    }

    return (conflictEdgeNum / 2);
}

void GraphColoring::print() const
{
    if (check() != optima.conflictEdgeNum) {
        cout << "[LogicError] ";
    }
    cout << optima.conflictEdgeNum << endl;
}


void GraphColoring::initResultSheet( std::ofstream &csvFile )
{
    csvFile << "Date, Instance, Algorithm, RandSeed, Duration, IterCount, GenerationCount, Optima, Solution" << std::endl;
}

void GraphColoring::appendResultToSheet(
    const std::string &instanceFileName, std::ofstream &csvFile ) const
{
    if (check() != optima.conflictEdgeNum) {
        csvFile << "[LogicError] ";
    }

    csvFile << Timer::getLocalTime() << ", "
        << instanceFileName << ", "
        << SOLVING_ALGORITHM << ", "
        << Random::getSeed() << ", "
        << timer.getTotalDuration() << ", "
        << iterCount << ", "
        << generationCount << ", "
        << optima.conflictEdgeNum << ", ";

    for (VertexColor::const_iterator iter = optima.vertexColor.begin();
        iter != optima.vertexColor.end(); iter++) {
        csvFile << *iter << ' ';
    }

    csvFile << std::endl;
}
