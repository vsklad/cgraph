//
//  CGraph - Convertor from DIMACS CNF to GraphML format
//  https://www.sophisticatedways.net
//  Copyright © 2018 Volodymyr Skladanivskyy. All rights reserved.
//  Published under terms of MIT license.
//

#include <cstring>
#include <string>
#include <iostream>
#include "cnf.hpp"
#include "dimacs.hpp"
#include "graphml.hpp"
#include "fileutils.hpp"

using namespace bal;

int main(int argc, const char * argv[]) {
    std::cout << "CGraph 1.1 - Convert DIMACS CNF to Grapf ML" << std::endl;
    
    unsigned arg_index = 1;
    
    bool weighted = false;
    std::string input_file_name;
    std::string output_file_name;
    
    if (arg_index < argc && strlen(argv[arg_index]) == 2 && argv[arg_index][0] == '-' && argv[arg_index][1] == 'w') {
        weighted = true;
        arg_index++;
    };
    
    if (arg_index < argc) {
        input_file_name = argv[arg_index];
        arg_index++;
    };

    if (arg_index < argc) {
        output_file_name = argv[arg_index];
        arg_index++;
    };
    
    if (output_file_name.empty()) {
        output_file_name = input_file_name + ".graphml";
    };
    
    if (!input_file_name.empty()) {
        std::cout << "Input file: " << input_file_name << std::endl;
        Cnf cnf;
        read_from_file<Cnf, DimacsStreamReader>(cnf, input_file_name.c_str());
        
        std::cout << "CNF: " << std::dec;
        std::cout << cnf.variables_size() << " variables";
        std::cout << ", " << cnf.clauses_size()  << " clauses";
        std::cout << ", " << cnf.literals_size() << " literals";
        std::cout << std::endl;
        
        std::cout << "Output file: " << output_file_name << std::endl;
        if (weighted) {
            write_to_file<Cnf, GraphMLWeightedStreamWriter>(cnf, output_file_name.c_str());
        } else {
            write_to_file<Cnf, GraphMLStreamWriter>(cnf, output_file_name.c_str());
        };
    } else {
        std::cout << "Usage:" << std::endl;
        std::cout << "  cgraph [-w] <input file name> [<output file name>]" << std::endl;
        std::cout << "  <input file name> - input DIMACS CNF file name" << std::endl;
        std::cout << "  <output file name> - output Graph ML file name" << std::endl;
        std::cout << "  w - include edge weight and cardinality" << std::endl;
    };
    return 0;
}
