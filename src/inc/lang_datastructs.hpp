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
		assert(nodeptr);
                for (key_type i = 0; i < nodeptr->count; i++) {
                    stream << nodeptr->values[i] << " ";
                }
            } else {
                internal_node_t* nodeptr = (internal_node_t*)&data;
		assert(nodeptr);
		for (key_type i = 0; i < nodeptr->count; ++i) {
		    assert(nodeptr->children[i]);
		    stream << *(nodeptr->children[i]) << " ";
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

    static inline internal_node make_internal(allocator* _allocator)
    {
        assert(_allocator);
        return alb::make_shared<internal_node_t>(*_allocator);
    }

    static inline leaf_node make_leaf(allocator* _allocator)
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
        this->root = make_internal(alloc);
    }

    pvec(const pvec&) = default;

    static pvec create(allocator* allocator, std::initializer_list<T> list)
    {
	tvec res{ allocator };
	for (auto& item : list) {
	    res.conj(item);
        }
	pvec result = res.to_persistent();
	return result;
    }

    static leaf_node copy_leaf(allocator* _allocator, const leaf_node& node)
    {
        leaf_node newnode = make_leaf(_allocator);
        newnode->values = node->values;
        newnode->count = node->count;
        return newnode;
    }

    static internal_node copy_internal(allocator* _allocator, const internal_node& node)
    {
        internal_node newnode = make_internal(_allocator);
        newnode->children = node->children;
        newnode->count = node->count;
        return newnode;
    }

    static inline internal_node copy_internal(allocator* _allocator, const node& node)
    {
        internal_node casted = std::static_pointer_cast<internal_node_t>(node);
        return copy_internal(_allocator, casted);
    }

    static inline leaf_node copy_leaf(allocator* _allocator, const node& node)
    {
        leaf_node casted = std::static_pointer_cast<leaf_node_t>(node);
        return copy_leaf(_allocator, casted);
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

    static node new_path(size_t level, node to_node, allocator* _allocator)
    {
        if (level == 0)
            return to_node;
        internal_node new_node = make_internal(_allocator);
        new_node->children[0] = new_path(level - bits, to_node, _allocator);
        return new_node;
    }

    internal_node push_tail(size_t level, internal_node parent, leaf_node tail)
    {
        size_t subidx = ((count - 1) >> level) & index_mask;
        internal_node ret = copy_internal(_allocator, parent);
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
                to_insert = new_path(level - bits, tail, _allocator);
            }
        }
        ret->children[subidx] = to_insert;
        return ret;
    }

    node do_assoc(size_t level, node parent, key_type key, T item)
    {
        if (level == 0) {
            leaf_node ret = copy_leaf(_allocator, parent);
            ret->values[key & index_mask] = item;
            return ret;
        } else {
            internal_node ret = copy_internal(_allocator, parent);
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
                leaf_node newtail = copy_leaf(_allocator, this->tail);
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
                newvec.tail = make_leaf(_allocator);
            } else {
                newvec.tail = copy_leaf(_allocator, this->tail);
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
                newroot = make_internal(_allocator);

                newroot->children[0] = root;
                newroot->children[1] = new_path(shift, tail, _allocator);

                newshift += 5;

            } else {
                newroot = push_tail(shift, root, tail);
            }

            leaf_node newtail = make_leaf(_allocator);
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
            leaf_node newtail = make_leaf(_allocator);
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
            newroot = make_internal(_allocator);
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
                auto ret = copy_internal(_allocator, node);
                ret->children[subidx] = newchild;
                return ret;
            }
        } else if (subidx == 0) {
            return nullptr;
        } else {
            auto ret = copy_internal(_allocator, node);
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


    struct tvec {
	size_t count;
	size_t shift;
	internal_node root;
	leaf_node tail;

	allocator* _allocator;

	tvec(size_t count, size_t shift, internal_node root, leaf_node tail, allocator* _allocator)
	    : count(count)
	    , shift(shift)
	    , root(root)
	    , tail(tail)
	    , _allocator(_allocator)
	{}

	tvec(allocator* _allocator)
	    : tvec(0, bits, make_internal(_allocator), make_leaf(_allocator), _allocator)
	{}
	     

	tvec(const pvec& v) : tvec(v.count, v.shift, copy_internal(v._allocator,v.root), copy_leaf(v._allocator,v.tail), v._allocator)
	{}

	void ensure_editable()
	{
	    // assert some atomic bool 
	}

	void disable_edit()
	{
	    // set editable bool to false
	}

	size_t tail_offset() const
	{
	    if (count < width) {
		return 0;
	    }
	    return ((count - 1) >> bits) << 5;
	}

	pvec to_persistent()
	{
	    ensure_editable();

	    disable_edit();
	    leaf_node trimmed_tail = make_leaf(_allocator);
	    std::copy(tail->values.begin(), tail->values.begin() + (count - tail_offset()), trimmed_tail->values.begin());
	    trimmed_tail->count = tail->count;
	    return {count, shift, root, trimmed_tail, _allocator};
	}

	tvec& conj(T item)
	{
	    ensure_editable();

	    size_t i = count;

	    if (i - tail_offset() < width) {
		tail->values[i & index_mask] = item;
		++tail->count;
		++this->count;
		return *this;
	    }

	    internal_node newroot;
	    leaf_node tailnode = make_leaf(_allocator);
	    tail = make_leaf(_allocator);
	    tail->values[0] = item;

	    size_t newshift = shift;

	    if ((count >> bits) > (1 << shift)) {
		newroot = make_internal(_allocator);
		newroot->children[0] = root;
		newroot->children[1] = new_path(shift, tailnode,_allocator);
		newroot->count = 2;
		newshift += 5;
	    }
	    else {
		newroot = push_tail(shift, root, tailnode);
	    }
	    root = newroot;
	    shift = newshift;
	    ++count;
	    return *this;
	}

	tvec& assoc(key_type key, const T& item)
	{
	    return assoc_n(key, item);
	}

	tvec& assoc_n(key_type i, const T& item) {
	    if (i < count) {
		if (i >= tail_offset()) {
		    tail->values[i & index_mask] = item;
		    return *this;
		}
		root = std::static_pointer_cast<internal_node_t>(do_assoc(shift, root, i, item));
		return *this;
	    }
	    if (i == count) {
		return conj(item);
	    }
	    throw std::runtime_error("Index not found in transient vector!");
	}

	node do_assoc(size_t level, node node, size_t i, const T& item) {
	    if (level == 0) {
		leaf_node nodeptr = std::static_pointer_cast<leaf_node_t>(node);
		nodeptr->values[i & index_mask] = item;
	    }
	    else {
		size_t subidx = (i >> level) & index_mask;
		internal_node nodeptr = std::static_pointer_cast<internal_node_t>(node);
		nodeptr->children[subidx] = do_assoc(level - bits, nodeptr->children[subidx], i, item);
	    }
	    return node;
	}

	internal_node push_tail(size_t level, internal_node parent, leaf_node tailnode)
	{
	    ensure_editable();

	    size_t subidx = ((count - 1) >> level) & index_mask;
	    internal_node ret = parent;
	    node to_insert;
	    if (level == bits) {
		to_insert = tail;
	    }
	    else {
		assert(parent->type == node_type::internal);
		internal_node child = std::static_pointer_cast<internal_node_t>(
		    parent->children[subidx]);

		if (child) {
		    to_insert = push_tail(level - bits, child, tailnode);
		}
		else {
		    to_insert = new_path(level - bits, tailnode, _allocator);
		}
	    }
	    ret->children[subidx] = to_insert;
	    return ret;
	}


	
    };


    tvec as_transient()
    {
	return {*this};
    }
};

#endif
