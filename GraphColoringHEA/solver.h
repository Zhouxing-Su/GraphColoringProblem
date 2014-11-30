/**
*   usage : 1. set algorithm arguments in run()
*
*   note :  1.
*/

#ifndef SOLVER_H

#include <string>
#include <iostream>
#include <fstream>
#include <cmath>

#include "GraphColoring.h"


const int MAX_BUF_LEN = 1000;   // max length for char array buf

const std::string LOG_FILE = "log.csv";
const std::string INST_DIR = "../instance/";
const std::string OPTIMA_FILE = "optima.txt";
const int INSTANCE_NUM = 12;
const std::string INSTANCE[INSTANCE_NUM] = {
    "DSJC125.1.col",    // 0
    "DSJC250.1.col",    // 1
    "DSJC250.5.col",    // 2
    "DSJC250.9.col",    // 3
    "DSJC500.1.col",    // 4
    "DSJC500.5.col",    // 5    (49)
    "DSJC500.5.col",    // 6    (48)
    "DSJC500.9.col",    // 7
    "DSJC1000.1.col",   // 8
    "DSJC1000.5.col",   // 9
    "DSJC1000.9.col",   // 10   (224)
    "DSJC1000.9.col"    // 11   (223)
};



void run( int inst, std::ofstream &logFile );
void run_tabu( int inst, std::ofstream &logFile );
GraphColoring::AdjVertexList readInstance( const std::string &fileName );
int readOptima( int inst );


#define SOLVER_H
#endif