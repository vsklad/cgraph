//
//  Boolean Algebra Library (BAL)
//  https://cgen.sophisticatedways.net
//  Copyright © 2018 Volodymyr Skladanivskyy. All rights reserved.
//  Published under terms of MIT license.
//

#ifndef graphml_h
#define graphml_h

#include <set>
#include <map>
#include "streamable.hpp"
#include "cnf.hpp"

namespace bal {
    
    // outputs an undirected graph
    // nodes correspond to variables; node IDs match DIMACS variable numbers
    // there is an edge between two variables if those variables occur in the same clause
    class GraphMLStreamWriter: public StreamWriter<Cnf> {
    protected:
        virtual void write_header(const Cnf& value) {
            stream() << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
            stream() << "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns http://graphml.graphdrawing.org/xmlns/1.0/graphml.xsd\">" << std::endl;
            stream() << "<graph id=\"CNF\" edgedefault=\"undirected\">" << std::endl;
            stream() << "<key id=\"n_variable_name\" for=\"node\" attr.name=\"variable_name\" attr.type=\"string\"/>" << std::endl;
            stream() << "<key id=\"n_variable_index\" for=\"node\" attr.name=\"variable_index\" attr.type=\"int\"/>" << std::endl;
            stream() << "<key id=\"n_variable_id\" for=\"node\" attr.name=\"variable_id\" attr.type=\"int\"/>" << std::endl;
            stream() << "<key id=\"n_label\" for=\"node\" attr.name=\"label\" attr.type=\"string\"/>" << std::endl;
        };
        
        void write_footer(const Cnf& value) {
            stream() << "</graph>" << std::endl;
            stream() << "</graphml>" << std::endl;
        };
        
        void write_variable(const variableid_t variable_id, const char* const name = nullptr, const unsigned index = 0) {
            stream() << std::dec;
            stream() << "<node id=\"v"  << literal_t(variable_t(variable_id)) << "\">" << std::endl;
            stream() << "<data key=\"n_variable_id\">" << literal_t(variable_t(variable_id)) << "</data>" << std::endl;
            if (name != nullptr) {
                stream() << "<data key=\"n_variable_name\">" << name << "</data>" << std::endl;
                stream() << "<data key=\"n_variable_index\">" << index << "</data>" << std::endl;
            };
            stream() << "<data key=\"n_label\">";
            if (name != nullptr) {
                stream() << name << "[" << index << "](";
            };
            stream() << literal_t(variable_t(variable_id));
            if (name != nullptr) {
                stream() << ")";
            };
            stream() << "</data>" << std::endl;
            stream() << "</node>" << std::endl;
        };
        
        // first, write all variables that ar epart of named ones
        // second, write all other variables
        // link binary variable to the first named variable it occurs in
        // and ignore other ones if any
        void write_variables(const Cnf& value) {
            bool is_processed[value.variables_size()];
            std::fill(is_processed, is_processed + value.variables_size(), false);
            
            const formula_named_variables_t& nv = value.get_named_variables();
            for (formula_named_variables_t::const_iterator it = nv.begin(); it != nv.end(); ++it) {
                const VariablesArray& nv_variables = it->second;
                const std::string& nv_name = it->first;
                for (auto i = 0; i < nv_variables.size(); i++) {
                    if (literal_t__is_variable(nv_variables.data()[i])) {
                        const variableid_t variable_id = literal_t__variable_id(nv_variables.data()[i]);
                        if (!is_processed[variable_id]) {
                            write_variable(variable_id, nv_name.c_str(), i);
                            is_processed[variable_id] = true;
                        };
                    };
                };
            };
            
            for (auto i = 0; i < value.variables_size(); i++) {
                if (!is_processed[i]) {
                    write_variable(i);
                    is_processed[i] = true;
                };
            };
        };
        
        virtual void write_clauses(const Cnf& value) {
            stream() << std::dec;
            std::set<uint64_t> existing_edges;
            
            const uint32_t* data = value.data();
            const uint32_t* data_end = data + value.data_size();
            while (data < data_end) {
                // iterate literal pairs
                // it is guaranteed that the sequence is sorted and no duplicates exist
                for (auto i = 0; i < _clause_size(data); i++) {
                    for (auto j = i + 1; j < _clause_size(data); j++) {
                        const variableid_t source = literal_t__variable_id(_clause_literal(data, i));
                        const variableid_t target = literal_t__variable_id(_clause_literal(data, j));
                        const uint64_t key = ((uint64_t)target << 32) | source;
                        
                        // check if the adge exists already; ignore if so, add otherwise
                        auto it = existing_edges.find(key);
                        if (it == existing_edges.end()) {
                            existing_edges.insert(key);
                            stream() << "<edge source=\"v" << literal_t(variable_t(source)) << "\" target=\"v" << literal_t(variable_t(target)) << "\"/>" << std::endl;
                        };
                    };
                };
                
                data += _clause_memory_size(data);
            };
        };
        
    public:
        GraphMLStreamWriter(std::ostream& stream): StreamWriter<Cnf>(stream) {};
        
        virtual void write(const Cnf& value) override {
            write_header(value);
            write_variables(value);
            write_clauses(value);
            write_footer(value);
        };
    };
    
    class GraphMLWeightedStreamWriter: public GraphMLStreamWriter {
    protected:
        virtual void write_header(const Cnf& value) override {
            GraphMLStreamWriter::write_header(value);
            stream() << "<key id=\"e_cardinality\" for=\"edge\" attr.name=\"cardinality\" attr.type=\"int\"/>" << std::endl;
            stream() << "<key id=\"e_weight\" for=\"edge\" attr.name=\"weight\" attr.type=\"double\"/>" << std::endl;
        };
        
        virtual void write_clauses(const Cnf& value) override {
            typedef struct {
                unsigned cardinality;
                double weight;
            } edge_data_t;
            
            stream() << std::dec;
            std::map<uint64_t, edge_data_t> existing_edges;
            
            const uint32_t* data = value.data();
            const uint32_t* data_end = data + value.data_size();
            while (data < data_end) {
                // iterate literal pairs
                // it is guaranteed that the sequence is sorted and no duplicates exist
                // weight is calculated such that the sum of weights of edges generated from a clause is 1
                for (auto i = 0; i < _clause_size(data); i++) {
                    if (_clause_size(data) > 1) {
                        uint16_t cardinality = 1;
                        if (_clause_size(data) < 4) {
                            cardinality = get_cardinality_uint16(_clause_flags(data));
                        };
                        
                        const double weight = 2.0 * cardinality / _clause_size(data) / (_clause_size(data) - 1);
                        
                        for (auto j = i + 1; j < _clause_size(data); j++) {
                            const variableid_t source = literal_t__variable_id(_clause_literal(data, i));
                            const variableid_t target = literal_t__variable_id(_clause_literal(data, j));
                            const uint64_t key = ((uint64_t)target << 32) | source;
                            
                            // check if the adge exists already; ignore if so, add otherwise
                            auto it = existing_edges.find(key);
                            if (it == existing_edges.end()) {
                                const edge_data_t edge_data{cardinality, weight};
                                existing_edges.insert({key, edge_data});
                            } else {
                                it->second.cardinality += cardinality;
                                it->second.weight += weight;
                            };
                        };
                    };
                };
                
                data += _clause_memory_size(data);
            };
            
            for (auto edge: existing_edges) {
                const variableid_t source = edge.first & 0xFFFFFFFF;
                const variableid_t target = edge.first >> 32;
                
                stream() << "<edge source=\"v" << literal_t(variable_t(source)) << "\" target=\"v" << literal_t(variable_t(target)) << "\">" << std::endl;
                stream() << "<data key=\"e_cardinality\">" << edge.second.cardinality << "</data>" << std::endl;
                stream() << "<data key=\"e_weight\">" << edge.second.weight << "</data>" << std::endl;
                stream() << "</edge>" << std::endl;
            };
        };
        
    public:
        GraphMLWeightedStreamWriter(std::ostream& stream): GraphMLStreamWriter(stream) {};
    };
    
};

#endif /* graphml_h */
