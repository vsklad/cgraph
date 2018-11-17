//
//  CGraph - Convertor from DIMACS CNF to GraphML format
//  https://www.sophisticatedways.net
//  Copyright © 2018 Volodymyr Skladanivskyy. All rights reserved.
//  Published under terms of MIT license.
//

#include <iostream>
#include "cnf.hpp"
#include "dimacs.hpp"
#include "graphml.hpp"
#include "fileutils.hpp"

using namespace bal;

int main(int argc, const char * argv[]) {
    std::cout << "CGraph 1.0 - Convert DIMACS CNF to Grapf ML" << std::endl;
    if (argc == 3) {
        std::cout << "Input file: " << argv[1] << std::endl;
        Cnf cnf;
        read_from_file<Cnf, DimacsStreamReader>(cnf, argv[1]);
        
        std::cout << "CNF: " << std::dec;
        std::cout << cnf.variables_size() << " variables";
        std::cout << ", " << cnf.clauses_size()  << " clauses";
        std::cout << ", " << cnf.literals_size() << " literals";
        std::cout << std::endl;
        
        std::cout << "Output file: " << argv[2] << std::endl;
        write_to_file<Cnf, GraphMLStreamWriter>(cnf, argv[2]);
        
    } else {
        std::cout << "Usage: expect 2 parameters." << std::endl;
        std::cout << "  1) input DIMACS CNF file name" << std::endl;
        std::cout << "  2) output Graph ML file name" << std::endl;
    };
    return 0;
}
