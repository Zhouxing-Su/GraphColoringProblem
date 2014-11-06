// solve graph coloring problem with hybrid evolutionary algorithm

#include <iostream>
#include <fstream>
#include <string>

#include "GraphColoring.h"
#include "solver.h"


using namespace std;


int main()
{
    ofstream csvFile( LOG_FILE, ios::app );
    //GraphColoring::initResultSheet( csvFile );

    run( 0, csvFile );

    csvFile.close();
    system( "pause" );
    return 0;
}



void run( int inst, ofstream &logFile )
{
    const string &instName = INSTANCE[0];
    GraphColoring::AdjVertexList adjVertexList( readInstance( instName ) );

    int colorNum = readOptima( instName );
    int vertexNum = adjVertexList.size();

    GraphColoring gc( adjVertexList, colorNum );

    int tabuTenureBase = 0;
    int maxGenerationCount = 100;
    int maxIterCount = 100000;
    int populationSize = 1;

    gc.init( tabuTenureBase, maxGenerationCount, maxIterCount, populationSize );
    gc.solve();
    gc.print();
    gc.appendResultToSheet( instName, logFile );
}


int readOptima( const string &instName )
{
    string inst;
    int optima;
    string time;
    ifstream ifs( INST_DIR + OPTIMA_FILE );

    do {
        ifs >> inst >> optima >> time;
    } while (inst != instName);

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