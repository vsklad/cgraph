//
//  Boolean Algebra Library (BAL)
//  https://cgen.sophisticatedways.net
//  Copyright © 2018 Volodymyr Skladanivskyy. All rights reserved.
//  Published under terms of MIT license.
//

#include <chrono>
#include "cnf.hpp"

namespace bal {
    
    unsigned __find_clause_found = 0;
    unsigned __find_clause_unfound = 0;
    unsigned __compare_clauses_ = 0;
    unsigned __append_clause_ = 0;
    std::chrono::time_point<std::chrono::system_clock> __time_start_;
    
    // CnfL0Index
    
    void CnfL0Index::rollback(const container_size_t size,
                          const container_size_t instances_size,
                          const container_size_t container_size) {
        base_t::rollback(size, instances_size, container_size);
        this->size_ = 0;
        this->instances_.size_ = 0;
        this->instances_.append(CONTAINER_END, instances_size);
        container_offset_t container_offset = 0;
        while (container_offset < container_size) {
            const uint32_t* const p_clause = this->container_.data_ + container_offset;            
            insertion_point_t insertion_point;
            this->find(literal_t__variable_id(_clause_literal(p_clause, 0)), p_clause, insertion_point);
            container_offset_t offset_found = insertion_point.container_offset;
            assert(offset_found == CONTAINER_END);
            this->append(insertion_point, container_offset);
            container_offset += _clause_size(p_clause) + 1;
        };
    };
    
    // Cnf
    
    const clauses_size_t Cnf::clauses_size(const clause_size_t clause_size, bool aggregated) const {
        clauses_size_t result = 0;
        uint32_t offset = 0;
        while (offset < clauses_.size_) {
            const clause_size_t literals_size = _clause_size(clauses_.data_ + offset);
            if (clause_size == 0 || clause_size == literals_size) {
                if(!aggregated && literals_size <= 4) {
                    result += get_cardinality_uint16(_clause_flags(clauses_.data_ + offset));
                } else {
                    result ++;
                };
            };
            offset += literals_size + 1;
        };
        return result;
    };
    
    const clauses_size_t Cnf::literals_size(bool aggregated) const {
        clauses_size_t result = 0;
        uint32_t offset = 0;
        while (offset < clauses_.size_) {
            const clause_size_t literals_size = _clause_size(clauses_.data_ + offset);
            if(!aggregated && literals_size <= 4) {
                result += get_cardinality_uint16(_clause_flags(clauses_.data_ + offset)) * literals_size;
            } else {
                result += literals_size;
            };
            offset += literals_size + 1;
        };
        return result;
    };
     
    void Cnf::transaction_begin() {
        assert(immutable_offset_ == 0);
        immutable_offset_ = clauses_.size_;
        l0_index_.transaction_begin();
    };
    
    void Cnf::transaction_commit() {
        immutable_offset_ = 0;
        l0_index_.transaction_commit();
    };
    
    void Cnf::transaction_rollback() {
        assert(clauses_.size_ == 0 || (immutable_offset_ > 0 && immutable_offset_ <= clauses_.size_));
        clauses_.size_ = immutable_offset_;
        l0_index_.transaction_rollback();
        immutable_offset_ = 0;
    };

    void __statistics_reset() {
        __find_clause_found = 0;
        __find_clause_unfound = 0;
        __compare_clauses_ = 0;
        __append_clause_ = 0;
        __time_start_ = std::chrono::system_clock::now();
    };
    
    void __statistics_print() {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - __time_start_);
        std::cout << "Statistics: append: " << __append_clause_ << ", find: " << __find_clause_found << "/" << __find_clause_unfound << ", compare: " << __compare_clauses_ << ", " << duration.count() << " ms" << std::endl;
    };
    
};
