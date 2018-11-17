//
//  Boolean Algebra Library (BAL)
//  https://cgen.sophisticatedways.net
//  Copyright © 2018 Volodymyr Skladanivskyy. All rights reserved.
//  Published under terms of MIT license.
//

#ifndef cnf_hpp
#define cnf_hpp

#include <vector>
#include "container.hpp"
#include "binarytreeindex.hpp"
#include "variables.hpp"
#include "variablesarray.hpp"
#include "formula.hpp"
#include "cnfclauses.hpp"

namespace bal {
    class CnfProcessor;
    
    class CnfL0Index: public AvlTreesIndex<uint32_t, &compare_clauses> {
    public:
        using base_t = AvlTreesIndex<uint32_t, &compare_clauses>;
        using insertion_point_t = typename base_t::insertion_point_t;
        
    protected:
        // simply rebuild the index on rollback, with rebalancing
        // this is because of "replacement" of items which is a modification rather than addition
        // also, rebalancing is needed anyway
        virtual void rollback(const container_size_t size,
                              const container_size_t instances_size,
                              const container_size_t container_size) override;
        
    public:
        CnfL0Index(const Container<uint32_t>& container): base_t(container) {};
    };
    
    class Cnf: public Formula {
    public:
        class CnfVariableGenerator: public VariableGenerator { friend class Cnf; };
        
        friend class CnfProcessor;

        // ADD_NAIVE enforces encoding of simple binary addition with carry
        static const bool constexpr ADD_NAIVE_DEFAULT = false;
        
        // ADD_MAX_ARGS determines maximal number of arguments for an add expression
        // longer expressions are split into batches of the given lenth, last one can be shorter
        // ADD_MAX_ARGS is applied at bit level; it determines max number of variables
        // including any carry in bits, while any constants are optimized out
        // 6 arguments max supported, see cnfwordadd.hpp
        static const uint32_t constexpr ADD_MAX_ARGS_DEFAULT = 3;
        static const uint32_t constexpr ADD_MAX_ARGS_MIN = 2;
        static const uint32_t constexpr ADD_MAX_ARGS_MAX = 6;
        
        // XOR_MAX_ARGS determines maximal number of arguments for a xor expression
        // longer expressions are split into batches of the given lenth max, last one can be shorter
        // default value of 3 is chosen to minimize both variables and clauses
        // 10 means 2^10 = 1K clauses per expression - seems enough
        static const uint32_t constexpr XOR_MAX_ARGS_DEFAULT = 3;
        static const uint32_t constexpr XOR_MAX_ARGS_MIN = 2;
        static const uint32_t constexpr XOR_MAX_ARGS_MAX = 10;
        
        // index data types
        using l0_index_t = CnfL0Index;
        
    private:
        // the buffer is a sequence of 32 bit words
        // each clause is a sequence of words
        // first word is a header; it is followed by a sequence of sorted variable/literal IDs
        // lowest 2 bytes of the clause header define clause size (number of literals)
        // higher 2 bytes of the header are clause-size-dependent flags
        // clauses size 2, 3, 4 (aggregated):
        //   variable ids are used
        //   flags identify which variable combinations are present
        // other clauses: flags not used, composed of literal ids
        Container<uint32_t> clauses_;
        
        // lists of clauses where the variable is the first literal
        // sorted by comparing clauses
        l0_index_t l0_index_;
        
        // no clauses can be modified below this offset
        // new aggregated clauses are produced above this offset only
        // 0 means none
        container_offset_t immutable_offset_;
        
        CnfVariableGenerator variable_generator_;
        
        // generation options
        uint32_t add_max_args_;
        uint32_t xor_max_args_;
        bool add_naive_;
        
    private:
        // assume the clause is new
        // search for the same clause
        //    if found:
        //      if not the first variable, the index is corrupted
        //      if first variable, merge and skip updating other variables indexes
        //    if not found:
        //      the clause is new, it should not exist for any other variables
        // avoid_merging forces to append new clause and prevents merging with an existing one
        template<bool avoid_merging>
        inline void __append_clause(l0_index_t::insertion_point_t& l0_insertion_point) {
            __append_clause_++;
            
            uint32_t* const p_clause = clauses_.data_ + clauses_.size_;
            literalid_t* const literals = p_clause + 1;
            const clause_size_t literals_size = *p_clause & 0xFFFF;
            
            assert(literals_size != 0);
            
            // should be sorted at this stage
            // check is sorted; remove later for performance reasons
            for (auto i = 1; i < literals_size; i++) {
                assert(*(p_clause + i) <= *(p_clause + i + 1));
            };
            
            // header + uncomplemented literals if needed
            if (literals_size <= 4) {
                if ((*p_clause & 0xFFFF0000) == 0) {
                    uint16_t clause_bitmap = 0;
                    for (auto i = 0; i < literals_size; i++) {
                        if (literals[i] & 0x1) {
                            clause_bitmap |= 0x1 << i;
                        } else {
                            literals[i] |= 0x1;
                        };
                    };
                    *p_clause = literals_size | (0x1 << (16 + clause_bitmap));
                } else {
                    assert(literals_size < 1 || !literal_t__is_negation(p_clause[1]));
                    assert(literals_size < 2 || !literal_t__is_negation(p_clause[2]));
                    assert(literals_size < 3 || !literal_t__is_negation(p_clause[3]));
                    assert(literals_size < 4 || !literal_t__is_negation(p_clause[4]));
                    assert(literals_size != 1 || (*p_clause & 0xFFFC0000) == 0);
                    assert(literals_size != 2 || (*p_clause & 0xFFF00000) == 0);
                    assert(literals_size != 3 || (*p_clause & 0xFF000000) == 0);
                };
            };
            
            // check if the clause exists by looking up the first literal index
            // find insertion pont at the same time
            if (!l0_index_.is_valid_insertion_point(l0_insertion_point)) {
                l0_index_.find(literal_t__variable_id(_clause_literal(p_clause, 0)), p_clause, l0_insertion_point);
            };
            const container_offset_t existing_offset = l0_insertion_point.container_offset;
            
            // if a match for an aggregated clause is found, check if should
            // merge existing header from an immutable clause then still insert it
            // immediately in front of the existing clause
            bool is_extending_existing = false;
            if (((avoid_merging && existing_offset != CONTAINER_END) || existing_offset < immutable_offset_) && literals_size <= 4) {
                *p_clause |= clauses_.data_[existing_offset];
                is_extending_existing = true;
            };
            
            if (existing_offset == CONTAINER_END || is_extending_existing) {
                // clause not added yet but clauses_.size_ is its valid offset
                // append clauses index items for all literals, insert into ordered list for l0
                l0_index_.append(l0_insertion_point, clauses_.size_);
                clauses_.size_ += literals_size + 1; // "commits" the clause
            } else if (literals_size <= 4) {
                // a matching aggregated clause is found; it is enough to merge headers
                clauses_.data_[existing_offset] |= *p_clause;
            } else {
                // a duplicate for an existing non-aggregated clause
                /* deal with this later, ok if external, bad if derived */
                __print_clause(existing_offset);
                __print_clause(clauses_.size_);
                assert(false);
            };
        };
        
        inline void __set_variables_size(const variableid_t value) {
            variable_generator_.reset(value);
            l0_index_.reset_instaces_size(value);
        };
        
    public:
        Cnf(): Cnf(0, 0) {};
        Cnf(const variables_size_t variables_size, const clauses_size_t clauses_size): l0_index_(clauses_) {
            initialize(variables_size, clauses_size);
        };
        
        // reset internal structures and resize
        inline void initialize(const variables_size_t variables_size, const clauses_size_t clauses_size) {
            Formula::initialize();
            variable_generator_.reset(variables_size);
            clauses_.reset(clauses_size << 2); // set initial buffer with 4 words per clause
            l0_index_.reset(variables_size, clauses_size);
            immutable_offset_ = 0;
            add_max_args_ = ADD_MAX_ARGS_DEFAULT;
            xor_max_args_ = XOR_MAX_ARGS_DEFAULT;
            add_naive_ = ADD_NAIVE_DEFAULT;
        };

        const Container<uint32_t>& get_clauses() const { return clauses_; };
        // the below two methods are deprecated
        const uint32_t* const data() const { return clauses_.data_; };
        const uint32_t data_size() const { return clauses_.size_; };
        
        virtual const variables_size_t variables_size() const override { return variable_generator_.next(); };

        const bool is_empty() const override { return clauses_.size_ == 0; };
        
        // clause_size - 0 means count clauses of all lengths, otherwise specified length only
        // aggregated - count aggregates if true, otherwise count individual clauses
        const clauses_size_t clauses_size(const clause_size_t clause_size = 0, bool aggregated = false) const;
        const uint32_t literals_size(const bool agregated = false) const;
        
        const size_t memory_size_clauses() const { return clauses_.size_ << 2; };
        const size_t memory_size_clauses_index() const { return l0_index_.memory_size(); };
        
        inline VariableGenerator& variable_generator() { return variable_generator_; };

        inline bool get_add_naive() const { return add_naive_; };
        inline void set_add_naive(const bool value) { add_naive_ = value; };
        
        inline uint32_t get_add_max_args() const { return add_max_args_; };
        inline void set_add_max_args(const uint32_t value) {
            assert(value >= ADD_MAX_ARGS_MIN && value <= ADD_MAX_ARGS_MAX);
            add_max_args_ = value;
        };
        
        inline uint32_t get_xor_max_args() const { return xor_max_args_; };
        inline void set_xor_max_args(const uint32_t value) {
            assert(value >= XOR_MAX_ARGS_MIN && value <= XOR_MAX_ARGS_MAX);
            xor_max_args_ = value;
        };
        
        // DEBUG method
        inline bool __print_clause(const container_offset_t offset) const {
            print_clause(std::cout, clauses_.data_ + offset, "; ");
            std::cout << std::endl;
            return true;
        };
         
        // returns 0 if the clause is always satisfied; otherwise returns the new size
        inline static clause_size_t normalize_clause(uint32_t* const literals, const clause_size_t literals_size) {
            assert(literals_size > 0);
            
            std::sort(literals, literals + literals_size);
            
            assert(literal_t__is_variable(literals[0]));
            clause_size_t validated_size = 1;
            for (auto i = 1; i < literals_size; i++) {
                assert(literal_t__is_variable(literals[i]));
                if (literals[i] == literals[validated_size - 1]) {
                    //warning(literals_size, literals, "ignoring duplicate literal");
                    continue;
                } else if ((literals[i] ^ 0x1) == literals[validated_size - 1]) {
                    //warning(literals_size, literals, "ignoring the clause, always satisfied");
                    return 0;
                } else {
                    if (validated_size < i) {
                        literals[validated_size] = literals[i];
                    };
                    validated_size++;
                };
            };
            
            return validated_size;
        };
        
        // normalizes and ensures no duplicates by performing the following:
        //   sorts the list of literals
        //   removes any duplicates
        //   if both a variable and its complement are present, ignores the clause
        //   for clauses size 2, 3, 4,
        //     make uncomplemented versions of literals and determine the clause flag
        //   looks for the clause with the same literals
        //     if subsumption determined, ignores the clause
        //   for clauses size 2, 3, 4
        //      merges the clause into existing or adds new if does not exist
        //   for all other clauses
        //      adds the clause unless its a duplicate
        inline void append_clause(const literalid_t* const literals, const clause_size_t literals_size) {
            clauses_.reserve(literals_size + 1);
            clauses_.data_[clauses_.size_] = literals_size; // no flags
            literalid_t* const dst_literals = clauses_.data_ + clauses_.size_ + 1;
            std::copy(literals, literals + literals_size, dst_literals);
            clauses_.data_[clauses_.size_] = normalize_clause(dst_literals, literals_size);
            l0_index_t::insertion_point_t l0_insertion_point;
            __insertion_point_t_init(l0_insertion_point);
            __append_clause<false>(l0_insertion_point);
        };
        
        template<typename... Literals>
        inline void append_clause_l(Literals... literals) {
            constexpr auto n = sizeof...(literals);
            const literalid_t values[n] = { literals... };
            append_clause(values, n);
        };
        
        inline static void print_clause(std::ostream& stream, const uint32_t* const p_clause, const char* final_token = nullptr) {
            uint16_t clauses_bitmap = *p_clause >> 16;
            const clause_size_t literals_size = (*p_clause & 0xFFFF);
            
            if (literals_size <= 4 && clauses_bitmap != 0) {
                uint16_t clause_bitmap = 0;
                while (clauses_bitmap > 0) {
                    if (clauses_bitmap & 1) {
                        for (auto i = 0; i < literals_size; i++) {
                            literal_t literal(*(p_clause + i + 1));
                            literal.negate((clause_bitmap & (1 << i)) == 0);
                            stream << ((i > 0) ? " " : "") << literal;
                        };
                        
                        if (final_token != nullptr) {
                            stream << final_token;
                        };
                    };
                    clause_bitmap++;
                    clauses_bitmap >>= 1;
                };
            } else {
                for (auto i = 0; i < literals_size; i++) {
                    stream << ((i > 0) ? " " : "") << literal_t(*(p_clause + i + 1));
                };
                
                if (final_token != nullptr) {
                    stream << final_token;
                };
            };
        };
        
        using sorted_iterable_t = ContainerIterable<l0_index_t, ContainerIndexIterator>;
        sorted_iterable_t sorted_clauses() const { return sorted_iterable_t(l0_index_); };
        
        l0_index_t::instance_iterator_t variable_clauses() const { return l0_index_t::instance_iterator_t(l0_index_); };
        
        // transactions
        void transaction_begin();
        void transaction_commit();
        void transaction_rollback();
        
        inline bool is_clause_immutable(uint32_t offset) const { return offset < immutable_offset_; };
        inline const container_size_t get_immutable_offset() const { return immutable_offset_; };
    };
    
    class CnfProcessor {
    protected:
        Cnf& cnf_;
        
    protected:
        Container<uint32_t>& clauses_;
        Cnf::l0_index_t& l0_index_;
        formula_named_variables_t& named_variables_;
        
        inline void set_variables_size(const variableid_t value) {
            cnf_.__set_variables_size(value);
        };
        
        // assume the clause is normalized
        //   i.e. literals are sorted, no duplicates etc
        template<bool avoid_merging>
        inline void append_clause(const uint32_t* p_clause, Cnf::l0_index_t::insertion_point_t& l0_insertion_point) {
            assert((_clause_size(p_clause) > 4) ^ (_clause_flags(p_clause) != 0));
            if (p_clause != clauses_.data_ + clauses_.size_) {
                const clause_size_t literals_size = *p_clause & 0xFFFF;
                clauses_.reserve(literals_size + 1);
                std::copy(p_clause, p_clause + literals_size + 1, clauses_.data_ + clauses_.size_);
            };
            cnf_.__append_clause<avoid_merging>(l0_insertion_point);
        };
        
        // assume the clause is normalized
        //   i.e. literals are sorted, no duplicates etc
        template<bool avoid_merging>
        inline void append_clause(const uint32_t* p_clause) {
            Cnf::l0_index_t::insertion_point_t l0_insertion_point;
            __insertion_point_t_init(l0_insertion_point);
            append_clause<avoid_merging>(p_clause, l0_insertion_point);
        };
        
    public:
        CnfProcessor(Cnf& cnf): cnf_(cnf), clauses_(cnf.clauses_),
            l0_index_(cnf.l0_index_), named_variables_(cnf_.get_named_variables_()) {};
        
        virtual const bool execute() = 0;
    };
    
    void __statistics_reset();
    void __statistics_print();
};

#endif /* cnf_hpp */
