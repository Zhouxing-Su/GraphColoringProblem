#include "GraphColoring.h"

using namespace std;


///=== [ solving procedure ] ===============================

GraphColoring::GraphColoring( const AdjVertexList &avl, int cn )
    : MAX_CONFLICT( avl.size() * avl.size() ), vertexNum( avl.size() ), colorNum( cn ),
    adjVertexList( avl ), population(), optima( MAX_CONFLICT ),
    iterCount( 0 ), generationCount( 0 ), timer()
{
    Random::setSeed();
}


void GraphColoring::init( int tabuTenureBase,
    int maxGenerationCount, int maxIterCount,
    int populationSize, int mutateIndividualNum )
{
    timer.reset();

    POPULATION_SIZE = populationSize;
    MAX_GENERATION_COUNT = maxGenerationCount;
    MAX_ITERATION_COUNT = maxIterCount;
    TABU_TENURE_BASE = tabuTenureBase;
    MUTATE_INDIVIDUAL_NUM = mutateIndividualNum;

    ostringstream ss;
    ss << "HEA(PS=" << POPULATION_SIZE
        << "|GC=" << MAX_GENERATION_COUNT
        << "|IC=" << MAX_ITERATION_COUNT
        << "|TB=" << TABU_TENURE_BASE
        << "|MN=" << MUTATE_INDIVIDUAL_NUM
        << ')';
    SOLVING_ALGORITHM = ss.str();


    genInitPopulation( POPULATION_SIZE );
}

void GraphColoring::solve()
{
    if (optima.conflictEdgeNum > 0) {   // in case the optima is found in init()
        for (; generationCount < MAX_GENERATION_COUNT; generationCount++) {
            // select parents
            VertexSet parentSet( selectParents() );

            // combine
            Solution offspring( combineParents( parentSet ) );

            // local search on offspring
            //iterCount += offspring.localSearch( MAX_ITERATION_COUNT );
            iterCount += offspring.tabuSearch( MAX_ITERATION_COUNT, TABU_TENURE_BASE );

            // update optima and check if there is no conflict
            if (updateOptima( offspring )) {
                break;
            }

            // replace bad individual or resize the population
            if (updatePopulation( offspring )) {
                // increase the diversification of the population
                mutateIndividuals( MUTATE_INDIVIDUAL_NUM );
            }
        }
    }

    timer.record();
}


void GraphColoring::genInitPopulation( int size )
{
    while (size--) {
        Solution s( this, genRandomColorAssign( vertexNum, colorNum ) );
        s.tabuSearch( MAX_ITERATION_COUNT, TABU_TENURE_BASE );
        //s.localSearch( MAX_ITERATION_COUNT );
        population.push_back( s );
        if (updateOptima( s )) {
            return;
        }
    }
}

GraphColoring::VertexSet GraphColoring::selectParents()
{
    // select one individual randomly as first parent
    RangeRand rr( 0, population.size() - 1 );
    int parent1 = rr();

    // then select one of the best individuals as second parent
    RandSelect rs;
    int parent2 = ((parent1 == 0) ? 1 : 0);
    int minConflict = population[parent2].evaluate();
    for (int i = parent2 + 1; i < static_cast<int>(population.size()); i++) {
        if (i != parent1) {
            int conflict = population[i].evaluate();
            if (conflict < minConflict) {
                parent2 = i;
                minConflict = conflict;
                rs.reset();
            } else if ((conflict == minConflict)
                && (rs.isSelected())) {
                parent2 = i;
                minConflict = conflict;
            }
        }
    }

    // record the parents
    VertexSet parents;
    parents.insert( parent1 );
    parents.insert( parent2 );

    return parents;
}

GraphColoring::Solution GraphColoring::combineParents( const VertexSet &parents )
{
    vector<ColorVertex> pcv( parents.size() );

    int i = 0;
    for (VertexSet::const_iterator iter = parents.begin();
        iter != parents.end(); iter++, i++) {
        pcv[i] = population[*iter];
    }

    VertexColor vc( vertexNum );

    // for each color, loop select in parents
    for (int i = 0, parent = 0; i < colorNum;
        i++, ((++parent) %= parents.size())) {
        ColorVertex &cv( pcv[parent] );
        // find color with most vertices
        RandSelect rs;
        int colorWithMostVertices = 0;
        int maxVertexNum = cv[colorWithMostVertices].size();
        for (int c = 1; c < colorNum; c++) {
            if (static_cast<int>(cv[c].size()) > maxVertexNum) {
                maxVertexNum = cv[c].size();
                colorWithMostVertices = c;
                rs.reset();
            } else if ((cv[c].size() == maxVertexNum)
                && rs.isSelected()) {
                maxVertexNum = cv[c].size();
                colorWithMostVertices = c;
            }
        }

        // assign color with most vertices to its vertices
        for (VertexSet::iterator iter = cv[colorWithMostVertices].begin();
            iter != cv[colorWithMostVertices].end(); iter++) {
            vc[*iter] = i;
            // remove this vertex from VertexSet of all color of all parents
            for (int p = 0; p < static_cast<int>(pcv.size()); p++) {
                if (p != parent) {  // leave it out in case the iter becomes invalidate
                    for (int c = 0; c < colorNum; c++) {
                        if (pcv[p][c].erase( *iter ) == 1) {
                            break;
                        }
                    }
                }
            }
        }
        // clear this VertexSet alone also make it more efficient
        cv[colorWithMostVertices].clear();
    }

    // assign random color to rest vertices
    RangeRand rColor( 0, colorNum - 1 );
    ColorVertex &cv( pcv[0] );
    for (int i = 0; i < colorNum; i++) {
        for (VertexSet::iterator iter = cv[i].begin();
            iter != cv[i].end(); iter++) {
            vc[*iter] = rColor();
        }
    }

    return Solution( this, vc );
}

bool GraphColoring::updateOptima( const Solution &sln )
{
    if (optima.conflictEdgeNum > sln.evaluate()) {
        optima = sln;
    }
    return (optima.conflictEdgeNum <= 0);
}

bool GraphColoring::updatePopulation( const Solution &offspring )
{
    // select one of the worst individuals to drop
    RandSelect rs;
    int worstSln = 0;
    for (int i = 1; i < static_cast<int>(population.size()); i++) {
        if (population[worstSln] < population[i]) {
            worstSln = i;
            rs.reset();
        } else if ((population[worstSln] == population[i])
            && (rs.isSelected())) {
            worstSln = i;
        }
    }

    // replace old or just add the offspring
    if (offspring < population[worstSln]) {
        population[worstSln] = offspring;
    } else if (population.size() < static_cast<size_t>(2 * POPULATION_SIZE)) {
        population.push_back( offspring );
    } else {    // cull excess bad individuals
        sort( population.begin(), population.end() );
        //population.resize( POPULATION_SIZE ); // need default constructor which is dangerous
        while (static_cast<int>(population.size()) > POPULATION_SIZE) {
            population.pop_back();
        }
        return true;
    }

    return false;
}

void GraphColoring::mutateIndividuals( int mutateIndividualNum )
{
    // called after population cull or other condition?
    RangeRand rr( 0, population.size() - 1 );
    set<int> mutatedIndividuals;

    while (mutateIndividualNum--) {
        int individual;
        do {
            individual = rr();
        } while (mutatedIndividuals.find( individual ) != mutatedIndividuals.end());
        mutatedIndividuals.insert( individual );

        population[individual].perturb();
    }
}

GraphColoring::VertexColor GraphColoring::genRandomColorAssign( int vertexNum, int colorNum )
{
    VertexColor vc( vertexNum );
    RangeRand rColor( 0, colorNum - 1 );
    for (int vertex = 0; vertex < vertexNum; vertex++) {
        vc[vertex] = rColor();
    }

    return vc;
}

///=== [ Solution ] ===============================

GraphColoring::Solution::Solution( const GraphColoring *pgc, const VertexColor &vc )
    : gc( pgc ), conflictEdgeNum( 0 ), vertexColor( vc ), adjColorTab(), tabu()
{
    initDataStructure();
}

void GraphColoring::Solution::initDataStructure()
{
    conflictEdgeNum = 0;
    adjColorTab = AdjColorTable( gc->vertexNum, vector<int>( gc->colorNum, 0 ) );
    tabu = AdjColorTable( gc->vertexNum, vector<int>( gc->colorNum, 0 ) );

    for (size_t vertex = 0; vertex < adjColorTab.size(); vertex++) {
        AdjColor &adjColor = adjColorTab[vertex];
        const AdjVertex &adjVertex = gc->adjVertexList[vertex];
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
    tabu( gc->vertexNum, vector<int>( gc->colorNum, 0 ) )
{
}

GraphColoring::Solution& GraphColoring::Solution::operator=(const Solution &s)
{
    gc = s.gc;
    conflictEdgeNum = s.conflictEdgeNum;
    vertexColor = s.vertexColor;
    adjColorTab = s.adjColorTab;
    tabu = AdjColorTable( gc->vertexNum, vector<int>( gc->colorNum, 0 ) );
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
    RangeRand tabuTenurePerturb( 0, 16 );

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
        tabu[maxReduce.vertex][srcColor] = iterCount + conflictEdgeNum + tabuTenureBase + tabuTenurePerturb();
    }


    // replace the current solution with the local optima
    *this = localOptima;

    return iterCount;
}

void GraphColoring::Solution::perturb()
{
    RangeRand rrColor( 0, gc->colorNum - 1 );
    RangeRand rrVertex( 0, gc->vertexNum - 1 );
    RangeRand rrVertexNum( 1, gc->vertexNum );
    int perturbVertexNum = rrVertexNum();

    for (; perturbVertexNum > 0; perturbVertexNum--) {
        vertexColor[rrVertex()] = rrColor();
    }

    initDataStructure();
}

GraphColoring::Solution::operator GraphColoring::ColorVertex() const
{
    ColorVertex cv( gc->colorNum );

    for (int i = 0; i < gc->vertexNum; i++) {
        cv[vertexColor[i]].insert( i );
    }

    return cv;
}

///=== [ output ] ===============================

int GraphColoring::check() const
{
    return check( optima.vertexColor );
}

int GraphColoring::check( const VertexColor &vertexColor ) const
{
    int conflictEdgeNum = 0;
    for (int vertex = 0; vertex < vertexNum; vertex++) {
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

    csvFile << '(' << colorNum << ')';
    for (VertexColor::const_iterator iter = optima.vertexColor.begin();
        iter != optima.vertexColor.end(); iter++) {
        csvFile << *iter << ' ';
    }

    csvFile << std::endl;
}
