// solve graph coloring problem with hybrid evolutionary algorithm

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

#include "GraphColoring.h"
#include "solver.h"


using namespace std;


int main()
{
    ofstream csvFile( LOG_FILE, ios::app );
    //GraphColoring::initResultSheet( csvFile );

    for (int inst = 0; inst < 7; inst++) {
        run( inst, csvFile );
    }
    //run( 4, csvFile );

    csvFile.close();
    system( "pause" );
    return 0;
}



void run( int inst, ofstream &logFile )
{
    const string &instName = INSTANCE[inst];
    GraphColoring::AdjVertexList adjVertexList( readInstance( instName ) );

    int colorNum = readOptima( inst );
    int vertexNum = adjVertexList.size();

    //int tabuTenureBase = static_cast<int>(sqrt( colorNum ));
    int tabuTenureBase = 0;
    int maxGenerationCount = static_cast<int>(1E4);
    int maxIterCount = static_cast<int>(1E5);
    int populationSize = 4;
    int mutateIndividualNum = populationSize / 4;

    for (int runTime = 8; runTime > 0; runTime--) {
        GraphColoring gc( adjVertexList, colorNum );

        gc.init( tabuTenureBase, maxGenerationCount, maxIterCount,
            populationSize, mutateIndividualNum );
        gc.solve();
        gc.print();
        gc.appendResultToSheet( instName, logFile );
    }

    populationSize = 8;
    for (int runTime = 8; runTime > 0; runTime--) {
        GraphColoring gc( adjVertexList, colorNum );

        gc.init( tabuTenureBase, maxGenerationCount, maxIterCount,
            populationSize, mutateIndividualNum );
        gc.solve();
        gc.print();
        gc.appendResultToSheet( instName, logFile );
    }
}


int readOptima( int inst )
{
    string instName;
    int optima;
    string time;
    ifstream ifs( INST_DIR + OPTIMA_FILE );

    for (; inst >= 0; inst--) {
        ifs >> instName >> optima >> time;
    }

    ifs.close();

    return optima;
}

GraphColoring::AdjVertexList readInstance( const string &fileName )
{
    char c;                 // read useless characters
    char s[MAX_BUF_LEN];   // read useless strings
    int vertexNum;
    int edgeNum;
    int v1, v2;
    ifstream ifs( INST_DIR + fileName );

    // skip comment
    while (true) {
        ifs >> c;
        if (c == 'c') {
            ifs.getline( s, MAX_BUF_LEN );
        } else {
            break;
        }
    }

    ifs >> s >> vertexNum >> edgeNum;
    GraphColoring::AdjVertexList adjVertexList( vertexNum, vector<int>() );
    while (edgeNum--) {
        ifs >> c >> v1 >> v2;
        --v1;
        --v2;
        adjVertexList[v1].push_back( v2 );
        adjVertexList[v2].push_back( v1 );
    }

    ifs.close();

    return adjVertexList;
}