#include "solver.h"

using namespace std;



int main()
{
    ofstream csvFile( LOG_FILE, ios::app );
    GraphColoring::initResultSheet( csvFile );

    for (int inst = 0; inst < 7; inst++) {
        run( inst, csvFile );
        //run_tabu( inst, csvFile );
    }
    //run( 6, csvFile );
    //run_tabu( 6, csvFile );

    csvFile.close();
    system( "pause" );
    return 0;
}