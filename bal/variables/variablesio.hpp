//
//  Boolean Algebra Library (BAL)
//  https://cgen.sophisticatedways.net
//  Copyright © 2018 Volodymyr Skladanivskyy. All rights reserved.
//  Published under terms of MIT license.
//

#ifndef variablesio_hpp
#define variablesio_hpp

#include <ostream>
#include "textreader.hpp"
#include "variablesarray.hpp"

namespace bal {

    // output a compact representation, aggregating constants and sequences
    // [-]<variable_id>[/<sequence_count>[/<sequence_increment>]] | 0x<hex_value> | 0b<bin_value>
    // sequential hexadecimal constants formed up to 64 bit long, split into several values if longer
    // binary constants formed from 1 to 3 symbols to aling the sequence to the closest hex symbol
    // constants assume big endian format, i.e. the most significant bit first
    std::ostream& operator << (std::ostream& stream, const VariablesArray& array);

    // read text representation produced by the above function
    // returns the variable as a sequence of literals in <value>
    class VariableTextReader: public virtual TextReader {
    private:
        inline void read_variable_sequence_(unsigned int& sequence_size, signed int& step_size);
        inline void read_variable_element_hex_(Container<literalid_t>& value);
        inline void read_variable_element_bin_(Container<literalid_t>& value);
        inline void read_variable_element_var_(Container<literalid_t>& value);
        inline void read_variable_element_item_(Container<literalid_t>& value);
        inline void read_variable_elements_sequence_(Container<literalid_t>& value, const variableid_t element_size);
        inline void read_variable_element_(Container<literalid_t>& value, variableid_t& element_size);
        
    protected:
        literalid_t read_binary_variable_value();
        VariablesArray read_variable_value();
    };
};

#endif /* variablesio_hpp */
