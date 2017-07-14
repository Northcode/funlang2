#ifndef DATASTRUCTS_H
#define DATASTRUCTS_H

#include <experimental/optional>
#include <iostream>
#include <memory>

#include "allocator_base.hpp"

template<typename TFrom, typename Func>
auto
operator>>=(std::experimental::optional<TFrom> from, Func&& func)
  -> decltype(func(from.value()))
{
    if (from) {
        return func(*from);
    } else {
        return {};
    }
}

template<typename T, typename Allocator>
struct plist
{
    struct node_t;
    typedef std::shared_ptr<node_t> node;

    using allocator = Allocator;

    struct node_t
    {
        T item;
        node rest;

        node_t()
          : item()
          , rest()
        {
        }

        node_t(const node_t&) = default;

        friend std::ostream& operator<<(std::ostream& stream, node_t& data)
        {
            stream << data.item;
            if (data.rest) {
                stream << " " << *data.rest;
            }
            return stream;
        }
    };

    node first;
    allocator* _allocator;
    size_t count;

    plist(allocator* allocator)
      : first()
      , _allocator(allocator)
      , count(0)
    {
    }

    plist(node first, size_t count, allocator* allocator)
      : first(first)
      , _allocator(allocator)
      , count(count)
    {
    }

    plist(const plist&) = default;

    static plist create(allocator* allocator, std::initializer_list<T> list)
    {
        plist res{ allocator };
        for (auto& item : list) {
            res = res.conj(item);
        }
        return res;
    }

    node make_node()
    {
        assert(_allocator);
        return alb::make_shared<node_t>(*_allocator);
    }

    const T& peek()
    {
        assert(first);
        return first->item;
    }

    plist pop()
    {
        if (first) {
            return plist(first->rest, count - 1, _allocator);
        } else {
            return *this;
        }
    }

    plist conj(T item)
    {
        node newnode = make_node();
        assert(newnode);
        newnode->item = item;
        size_t newcount = 0;
        if (first) {
            newnode->rest = first;
            newcount = count + 1;
        } else {
        }
        return plist(newnode, newcount, _allocator);
    }

    friend std::ostream& operator<<(std::ostream& stream, plist& data)
    {
        stream << "(";
        if (data.first) {
            stream << *data.first;
        }
        stream << ")";
        return stream;
    }
};

template<typename T, typename Allocator>
plist<T, Allocator>
create_plist(Allocator* allocator, std::initializer_list<T> list)
{
    return plist<T, Allocator>::create(allocator, list);
}

template<typename T, typename Allocator, size_t BITS = 5>
struct pvec
{

    static constexpr size_t bits = BITS;
    static constexpr size_t width = 1 << bits;
    static constexpr size_t index_mask = width - 1;

    using key_type = size_t;

    enum class node_type
    {
        internal = 0,
        leaf = 1
    };

    class node_t
    {

      protected:
        node_t(node_type type)
          : type(type)
        {
        }

      public:
        node_t(node_t& n) = default;

        node_type type;
        size_t count = 0;

        friend std::ostream& operator<<(std::ostream& stream, node_t& data)
        {
            if (data.type == node_type::leaf) {
                leaf_node_t* nodeptr = (leaf_node_t*)&data;
                for (key_type i = 0; i < nodeptr->count; i++) {
                    stream << nodeptr->values[i] << " ";
                }
            } else {
                internal_node_t* nodeptr = (internal_node_t*)&data;
                for (auto i : nodeptr->children) {
                    if (i)
                        stream << *i << " ";
                }
            }
            return stream;
        }
    };

    typedef std::shared_ptr<node_t> node;

    struct internal_node_t : node_t
    {
        using coll = std::array<node, width>;

        coll children;

        internal_node_t()
          : node_t(node_type::internal)
        {
            children.fill(nullptr);
        }
    };

    struct leaf_node_t : node_t
    {
        using coll = std::array<T, width>;

        coll values;

        leaf_node_t()
          : node_t(node_type::leaf)
        {
        }
    };

    typedef std::shared_ptr<internal_node_t> internal_node;
    typedef std::shared_ptr<leaf_node_t> leaf_node;

    using allocator = Allocator;

    internal_node root;
    leaf_node tail;
    size_t count;
    size_t shift;

    allocator* _allocator;

    inline internal_node make_internal()
    {
        assert(_allocator);
        return alb::make_shared<internal_node_t>(*_allocator);
    }

    inline leaf_node make_leaf()
    {
        assert(_allocator);
        return alb::make_shared<leaf_node_t>(*_allocator);
    }

    pvec(size_t count,
         size_t shift,
         internal_node root,
         leaf_node tail,
         allocator* alloc)
    {
        this->shift = shift;
        this->root = root;
        this->tail = tail;
        this->count = count;
        this->_allocator = alloc;
        assert(_allocator);
    }

    pvec(allocator* alloc)
      : pvec(0, bits, nullptr, nullptr, alloc)
    {
        this->root = make_internal();
    }

    pvec(const pvec&) = default;

    static pvec create(allocator* allocator, std::initializer_list<T> list)
    {
	pvec res{ allocator };
	for (auto& item : list) {
	    res = res.conj(item);
        }
        return res;
    }

    leaf_node copy_leaf(const leaf_node& node)
    {
        leaf_node newnode = make_leaf();
        newnode->values = node->values;
        newnode->count = node->count;
        return newnode;
    }

    internal_node copy_internal(const internal_node& node)
    {
        internal_node newnode = make_internal();
        newnode->children = node->children;
        newnode->count = node->count;
        return newnode;
    }

    inline internal_node copy_internal(const node& node)
    {
        internal_node casted = std::static_pointer_cast<internal_node_t>(node);
        return copy_internal(casted);
    }

    inline leaf_node copy_leaf(const node& node)
    {
        leaf_node casted = std::static_pointer_cast<leaf_node_t>(node);
        return copy_leaf(casted);
    }

    node node_for(key_type key)
    {
        if (key < this->count) {
            node curr_node = this->root;
            for (size_t level = this->shift; level > 0; level -= bits) {
                internal_node tmp =
                  std::static_pointer_cast<internal_node_t>(curr_node);
                curr_node = tmp->children[(key >> level) & index_mask];
            }
            return curr_node;
        } else {
            return nullptr;
        }
    }

    size_t tail_offset() const
    {
        if (count < width) {
            return 0;
        } else {
            return ((count - 1) >> bits) << bits;
        }
    }

    const T& nth(key_type key)
    {
        leaf_node lookup_node;
        if (this->tail && (this->count - tail_offset() < width)) {
            lookup_node = this->tail;
        } else {
            lookup_node = std::static_pointer_cast<leaf_node_t>(node_for(key));
        }
        assert(lookup_node);
        // if (! lookup_node); // @TODO: handle error here sometime plz

        return lookup_node->values[key & index_mask];
    }

    node new_path(size_t level, node to_node)
    {
        if (level == 0)
            return to_node;
        internal_node new_node = make_internal();
        new_node->children[0] = new_path(level - bits, to_node);
        return new_node;
    }

    internal_node push_tail(size_t level, internal_node parent, leaf_node tail)
    {
        size_t subidx = ((count - 1) >> level) & index_mask;
        internal_node ret = copy_internal(parent);
        node to_insert;
        if (level == bits) {
            to_insert = tail;
        } else {
            assert(parent->type == node_type::internal);
            internal_node child = std::static_pointer_cast<internal_node_t>(
              parent->children[subidx]);

            if (child) {
                to_insert = push_tail(level - bits, child, tail);
            } else {
                to_insert = new_path(level - bits, tail);
            }
        }
        ret->children[subidx] = to_insert;
        return ret;
    }

    node do_assoc(size_t level, node parent, key_type key, T item)
    {
        if (level == 0) {
            leaf_node ret = copy_leaf(parent);
            ret->values[key & index_mask] = item;
            return ret;
        } else {
            internal_node ret = copy_internal(parent);
            key_type subindex = (key >> level) & index_mask;
            ret->children[subindex] =
              do_assoc(level - bits,
                       std::static_pointer_cast<internal_node_t>(parent)
                         ->children[subindex],
                       key,
                       item);
            return ret;
        }
    }

    std::experimental::optional<pvec> assoc(key_type key, T item)
    {
        if (key >= 0 && key < count) {
            if (key >= tail_offset()) {
                leaf_node newtail = copy_leaf(this->tail);
                newtail->values[key & index_mask] = item;

                pvec newvec{ count, shift, this->root, newtail, _allocator };
                return newvec;
            } else {
                pvec newvec{ count,
                             shift,
                             std::static_pointer_cast<internal_node_t>(
                               do_assoc(shift, root, key, item)),
                             tail,
                             _allocator };
                return newvec;
            }
        } else if (key == count) {
            return conj(item);
        } else {
            return {};
        }
    }

    pvec conj(T item)
    {

        size_t i = count;

        pvec newvec{ *this };
        // is space in tail
        if (i - tail_offset() < width) {
            if (!this->tail) {
                newvec.tail = make_leaf();
            } else {
                newvec.tail = copy_leaf(this->tail);
            }

            newvec.tail->values[newvec.tail->count] = item;
            newvec.tail->count++;
            newvec.count++;

            return newvec;
        } else {
            internal_node newroot;
            size_t newshift = shift;
            // check for root overflow
            if ((count >> bits) > (1u << shift)) {
                newroot = make_internal();

                newroot->children[0] = root;
                newroot->children[1] = new_path(shift, tail);

                newshift += 5;

            } else {
                newroot = push_tail(shift, root, tail);
            }

            leaf_node newtail = make_leaf();
            newtail->values[0] = item;
            newtail->count++;

            newvec.tail = newtail;
            newvec.count++;

            newvec.root = newroot;

            newvec.shift = newshift;

            return newvec;
        }
    }

    pvec pop()
    {
        if (count == 0) {
            throw std::runtime_error("Cannot pop an empty vector!");
        }
        if (count == 1) {
            return { _allocator };
        }
        if (count - tail_offset() > 1) {
            leaf_node newtail = make_leaf();
            std::copy(this->tail->values.begin(),
                      this->tail->values.begin() + this->tail->count - 1,
                      newtail->values.begin());
            newtail->count = this->tail->count - 1;
            return { count - 1, shift, root, newtail, _allocator };
        }
        auto newtail =
          std::static_pointer_cast<leaf_node_t>(node_for(count - 2));

        internal_node newroot = pop_tail(shift, root);
        size_t newshift = shift;
        if (!newroot) {
            newroot = make_internal();
        }
        if (shift > bits && !newroot->children[1]) {
            newroot =
              std::static_pointer_cast<internal_node_t>(newroot->children[0]);
            newshift -= bits;
        }
        return { count - 1, newshift, newroot, newtail, _allocator };
    }

    internal_node pop_tail(size_t level, internal_node node)
    {
        key_type subidx = ((count - 2) >> level) & index_mask;
        if (level > bits) {
            internal_node newchild =
              pop_tail(level - bits,
                       std::static_pointer_cast<internal_node_t>(
                         node->children[subidx]));
            if (!newchild && subidx == 0) {
                return nullptr;
            } else {
                auto ret = copy_internal(node);
                ret->children[subidx] = newchild;
                return ret;
            }
        } else if (subidx == 0) {
            return nullptr;
        } else {
            auto ret = copy_internal(node);
            ret->children[subidx] = nullptr;
            return ret;
        }
    }

    friend std::ostream& operator<<(std::ostream& stream, pvec& data)
    {

        stream << "[";
        if (data.root) {
            stream << *data.root;
        }

        if (data.tail) {
            stream << *data.tail;
        }

        return stream << "]";
    }
};

#endif
