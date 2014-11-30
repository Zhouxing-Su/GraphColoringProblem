#include "solver.h"

using namespace std;



int main()
{
    ofstream csvFile( LOG_FILE, ios::app );
    GraphColoring::initResultSheet( csvFile );

    for (int inst = 0; inst < 6; inst++) {
        //run( inst, csvFile );
        run_tabu( inst, csvFile );
    }
    //run( 5, csvFile );
    //run_tabu( 5, csvFile );

    csvFile.close();
    system( "pause" );
    return 0;
}