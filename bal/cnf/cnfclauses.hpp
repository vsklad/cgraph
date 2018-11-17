//
//  Boolean Algebra Library (BAL)
//  https://cgen.sophisticatedways.net
//  Copyright © 2018 Volodymyr Skladanivskyy. All rights reserved.
//  Published under terms of MIT license.
//

#ifndef cnfclauses_hpp
#define cnfclauses_hpp

#include "container.hpp"
#include "variables.hpp"

namespace bal {

#define _clause_size(p_clause) (*(p_clause) & 0xFFFF)
#define _clause_memory_size(p_clause) (_clause_size(p_clause) + 1)
#define _clause_is_aggregated(p_clause) (_clause_size(p_clause) <= 4)
#define _clause_flags(p_clause) (*(p_clause) >> 16)
#define _clause_flags_set(p_clause, flags) (*(p_clause) = _clause_size(p_clause) | ((flags) << 16))
#define _clause_literal(p_clause, index) ((p_clause)[(index) + 1])
#define _clause_literals(p_clause) ((p_clause) + 1)
#define _clause_header(flags, size) (uint32_t)(((flags) << 16) | (size))
#define _clause_header_flags(header) ((header) >> 16)
#define _clause_header_size(header) ((header) & 0xFFFF)
#define _clause_header_memory_size(header) (_clause_header_size(header) + 1)
    
    extern unsigned __find_clause_found;
    extern unsigned __find_clause_unfound;
    extern unsigned __compare_clauses_;
    extern unsigned __append_clause_;
    extern unsigned __normalize_clause_;
    
    typedef uint16_t clause_flags_t;
    typedef uint16_t clause_size_t;
    typedef uint32_t clauses_size_t;
    
    // maximal supported length of the clause (number of literals)
    // due to clause header
    static const clause_size_t constexpr CLAUSE_SIZE_MAX = UINT16_MAX;
    // maximal theoretically supported number of clauses
    // due to clause index
    static const clauses_size_t constexpr CLAUSES_SIZE_MAX = CONTAINER_SIZE_MAX;
    static const clauses_size_t constexpr CLAUSES_END = CONTAINER_END;
    static const variables_size_t constexpr VARIABLES_SIZE_MAX = VARIABLEID_MAX;
    
    // assume the first SAME_LITERALS are the same
    inline const int compare_clauses(const uint32_t* lhs, const uint32_t* rhs) {
        
        __compare_clauses_++;
        
        const clause_size_t lhs_size = *lhs & 0xFFFF;
        const clause_size_t rhs_size = *rhs & 0xFFFF;
        const clause_size_t common_size = lhs_size > rhs_size ? rhs_size : lhs_size;
        
        lhs++;
        rhs++;
        
        for (auto i = 0; i < common_size; i++) {
            if (lhs[i] != rhs[i]) {
                return lhs[i] < rhs[i] ? -1 : 1;
            };
        };
        // if all the corresponding literals match then the shorter clause is smaller
        return lhs_size < rhs_size ? -1 : (lhs_size == rhs_size ? 0 : 1);
    };
    
    
    inline uint16_t get_cardinality_uint16(const uint16_t value) {
        // 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
        const uint16_t map[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
        return map[value & 0xF] + map[value >> 4 & 0xF] +
        map[value >> 8 & 0xF] + map[value >> 12 & 0xF];
    };
};

#endif /* cnfclauses_hpp */
