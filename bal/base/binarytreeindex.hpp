//
//  Boolean Algebra Library (BAL)
//  https://cgen.sophisticatedways.net
//  Copyright © 2018 Volodymyr Skladanivskyy. All rights reserved.
//  Published under terms of MIT license.
//

#ifndef binarytreeindex_hpp
#define binarytreeindex_hpp

#include "containerindex.hpp"

namespace bal {
    
    typedef enum {btipkRoot, btipkLeft, btipkRight, btipkCurrent} binary_tree_insertion_point_kind_t;
    
    struct avl_tree_insertion_point_t: container_index_insertion_point_t {
        binary_tree_insertion_point_kind_t kind;
        container_offset_t offset; // instance offset for btipkRoot, index offset otherwise
    };
    
    // iterates over all list items for particular instance
    // used as a default iterator for AvlTreesIndex
    template<typename, typename, typename>
    class BinaryTreesIndexInstanceOffsetIterator;
    
    template<typename INDEX_DATA_T, typename CONTAINER_DATA_T, typename INSERTION_POINT_T>
    class BinaryTreesIndex: public ContainerIndex<INDEX_DATA_T, CONTAINER_DATA_T, BinaryTreesIndexInstanceOffsetIterator<INDEX_DATA_T, CONTAINER_DATA_T, INSERTION_POINT_T>, INSERTION_POINT_T> {
    private:
        using container_index_t = ContainerIndex<INDEX_DATA_T, CONTAINER_DATA_T, BinaryTreesIndexInstanceOffsetIterator<INDEX_DATA_T, CONTAINER_DATA_T, INSERTION_POINT_T>, INSERTION_POINT_T>;
        
        template<typename, typename, typename>
        friend class BinaryTreesIndexInstanceOffsetIterator;
        
    public:
        BinaryTreesIndex(const Container<CONTAINER_DATA_T>& container): container_index_t(container) {};
        
        using instance_iterator_t = BinaryTreesIndexInstanceOffsetIterator<INDEX_DATA_T, CONTAINER_DATA_T, INSERTION_POINT_T>;
    };
    
    template<typename INDEX_DATA_T, typename CONTAINER_DATA_T, typename INSERTION_POINT_T>
    class BinaryTreesIndexInstanceOffsetIterator {
    public:
        using index_t = BinaryTreesIndex<INDEX_DATA_T, CONTAINER_DATA_T, INSERTION_POINT_T>;
        
    protected:
        const index_t& index_;
        container_offset_t instance_offset_ = CONTAINER_END;
        container_offset_t item_offset_ = CONTAINER_END;
        
    public:
        BinaryTreesIndexInstanceOffsetIterator(const index_t& index): index_(index) {};
        
        // positions at the first list item for the given instance
        // returns offset of the corresponding container data element or CONTAINER_END
        inline const container_offset_t first(const container_offset_t instance_offset) {
            instance_offset_ = instance_offset;
            item_offset_ = instance_offset >= index_.instances_.size_ ? CONTAINER_END : index_.instances_.data_[instance_offset];
            // start from the deepest leftmost leaf node
            if (item_offset_ != CONTAINER_END) {
                while (index_.data_[item_offset_].left_offset != CONTAINER_END) {
                    item_offset_ = index_.data_[item_offset_].left_offset;
                };
            };
            return item_offset_ == CONTAINER_END ? CONTAINER_END : index_.data_[item_offset_].container_offset;
        };
        
        // moves to the next list item for the stored instance
        // returns offset of the corresponding container data element or CONTAINER_END
        inline const container_offset_t next() {
            if (item_offset_ != CONTAINER_END) {
                if (index_.data_[item_offset_].right_offset != CONTAINER_END) {
                    // there is a right element - move one right then the deepest leftmost element
                    item_offset_ = index_.data_[item_offset_].right_offset;
                    while (item_offset_ != CONTAINER_END && index_.data_[item_offset_].left_offset != CONTAINER_END) {
                        item_offset_ = index_.data_[item_offset_].left_offset;
                    };
                } else {
                    // no right element, move up as long as we come from right
                    // if coming from right and reached the root - its the end
                    while (item_offset_ != CONTAINER_END) {
                        container_offset_t parent_offset = index_.data_[item_offset_].parent_offset;
                        if (parent_offset != CONTAINER_END && index_.data_[parent_offset].right_offset == item_offset_) {
                            item_offset_ = parent_offset;
                        } else {
                            item_offset_ = parent_offset;
                            break;
                        };
                    };
                };
            };
            return item_offset_ == CONTAINER_END ? CONTAINER_END : index_.data_[item_offset_].container_offset;
        };
    };
    
    typedef struct {
        container_offset_t parent_offset;
        container_offset_t left_offset;
        container_offset_t right_offset;
        container_offset_t container_offset;
    } avl_tree_index_item_t;
    
    // TODO: does not implement rollback while changing matched items within the transaction bound
    // for this implementation, this must be addressed externally e.g. by descendants
    template<typename CONTAINER_DATA_T, typename Container<CONTAINER_DATA_T>::comparator_p comparator>
    class AvlTreesIndex: public BinaryTreesIndex<avl_tree_index_item_t, CONTAINER_DATA_T, avl_tree_insertion_point_t> {
    public:
        using base_t = BinaryTreesIndex<avl_tree_index_item_t, CONTAINER_DATA_T, avl_tree_insertion_point_t>;
        using insertion_point_t = typename base_t::insertion_point_t;
        
    public:
        AvlTreesIndex(const Container<CONTAINER_DATA_T>& container): base_t(container) {};
        
        // replaces equivalent items because the sequence of duplicates
        // is not guaranteed after rebalancing
        inline void append(const insertion_point_t& insertion_point, const container_size_t container_offset) {
            assert(insertion_point.offset != CONTAINER_END);
            assert(insertion_point.version_stamp == this->size_);
            if (insertion_point.kind == btipkRoot) {
                assert(insertion_point.offset < this->instances_.size_);
                assert(this->instances_.data_[insertion_point.offset] == CONTAINER_END);
                // point the instance to the new element
                this->instances_.data_[insertion_point.offset] = this->size_;
            } else if (insertion_point.kind == btipkLeft || insertion_point.kind == btipkRight) {
                // it is a leaf element
                assert(insertion_point.offset < this->size_);
                if (insertion_point.kind == btipkLeft) {
                    assert(this->data_[insertion_point.offset].left_offset == CONTAINER_END);
                    this->data_[insertion_point.offset].left_offset = this->size_;
                } else {
                    assert(this->data_[insertion_point.offset].right_offset == CONTAINER_END);
                    this->data_[insertion_point.offset].right_offset = this->size_;
                };
            };
            
            if (insertion_point.kind == btipkCurrent) {
                assert(insertion_point.offset < this->size_);
                this->data_[insertion_point.offset].container_offset = container_offset;
            } else {
                this->reserve(1);
                this->data_[this->size_] = {
                    insertion_point.kind == btipkRoot ? CONTAINER_END : insertion_point.offset,
                    CONTAINER_END,
                    CONTAINER_END,
                    container_offset
                };
                this->size_++;
            };
        };
        
        // find a match for p_object starting from the supplied index offset
        // returns container offset of the first matching object from the list if found,
        // otherwise returns CONTAINER_END
        inline container_offset_t find(const container_offset_t instance_offset, const CONTAINER_DATA_T* const p_object) const {
            container_offset_t offset = instance_offset < this->instances_.size_ ? this->instances_.data_[instance_offset] : CONTAINER_END;
            while (offset != CONTAINER_END) {
                int result = comparator(p_object, this->container_.data_ + this->data_[offset].container_offset);
                if (result > 0) {
                    offset = this->data_[offset].right_offset;
                } else if (result < 0) {
                    offset = this->data_[offset].left_offset;
                } else {
                    return this->data_[offset].container_offset;
                };
            };
            return CONTAINER_END;
        };
        
        // find a match for p_object for the specific instance
        // return container offset of the matching object if found
        // if not found, return CONTAINER_END plus p_index_offset, an insertion point
        // insertion point is an index offset reference which can be updated if a new item is inserted
        // Note: p_index_offset will be invalidated when the list memory is reallocated
        inline void find(const container_offset_t instance_offset, const CONTAINER_DATA_T* const p_object, insertion_point_t &insertion_point) {
            if (instance_offset >= this->instances_.size_) {
                // update instances offset table if necessary
                this->instances_.append(CONTAINER_END, instance_offset - this->instances_.size_ + 1);
            };
            insertion_point.version_stamp = this->size_;
            // take the root element from the instance
            container_offset_t offset = this->instances_.data_[instance_offset];
            if (offset != CONTAINER_END) {
                while (true) {
                    int result = comparator(p_object, this->container_.data_ + this->data_[offset].container_offset);
                    if (result > 0) {
                        if (this->data_[offset].right_offset == CONTAINER_END) {
                            insertion_point.kind = btipkRight;
                            insertion_point.offset = offset;
                            break;
                        };
                        offset = this->data_[offset].right_offset;
                    } else if (result < 0) {
                        if (this->data_[offset].left_offset == CONTAINER_END) {
                            insertion_point.kind = btipkLeft;
                            insertion_point.offset = offset;
                            break;
                        };
                        offset = this->data_[offset].left_offset;
                    } else {
                        insertion_point.kind = btipkCurrent;
                        insertion_point.offset = offset;
                        insertion_point.container_offset = this->data_[offset].container_offset;
                        return;
                    };
                };
            } else {
                insertion_point.kind = btipkRoot;
                insertion_point.offset = instance_offset;
                
            };
            insertion_point.container_offset = CONTAINER_END;
        };
    };
};

#endif /* binarytreeindex_hpp */
