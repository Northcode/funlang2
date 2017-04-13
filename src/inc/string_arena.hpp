#ifndef STRARENA_H
#define STRARENA_H

#include <list>
#include <string>
#include <cassert>
#include <iostream>

#include "mystr.hpp"

constexpr long PAGE_SIZE = 1000 * 1000 * 4; // 4 MB
// constexpr long PAGE_SIZE = 80;

struct arena_page {
  char* data;
  size_t len;

  size_t first_unused;
  size_t current_head;
  size_t last_head;

  arena_page() {
    data = new char[PAGE_SIZE];
    len = PAGE_SIZE;
    first_unused = 0;

    memset(data, 0, len);
  }

  arena_page(const arena_page& a) = delete;

  ~arena_page() {
    delete [] data;
  }

  void dump() {
    assert(data);
    for (size_t i = 0; i < first_unused; i++) {
      if (i % 10 == 0) {
	// printf("\n");
	std::cout << std::endl;
      }
      if (isprint(data[i])) printf("%c ", data[i]);
      else printf("%X ", data[i]);
      // std::cout << c;
    }
    std::cout << "remaining data empty.. ";
  }

  mystr alloc_str(size_t size) {
    assert(first_unused + size < len);
    mystr s;

    last_head = current_head;

    s.data = (data + first_unused);
    s.len = size;
    current_head = first_unused;
    first_unused += size;

    return s;
  }

  mystr alloc_null_term_str(mystr src) {
    mystr ntermd = alloc_str(src.len + 1);
    strncpy(ntermd.data,src.data, ntermd.len - 1);
    ntermd.data[ntermd.len - 1] = '\0';
    return ntermd;
  }

  void discard_head() {
    first_unused = current_head;
    current_head = last_head;
  }

  bool is_head(mystr s) {
    return s.data == data + current_head;
  }

  void append_char(mystr* dest, char c) {
    assert(first_unused + 1 < len);
    assert(dest->data >= data && dest->data < data + len);
    first_unused++;
    dest->data[dest->len] = c;
    dest->len++;
  }

  void append_str(mystr* dest, const char* src) {
    size_t srclen = strlen(src);
    assert(first_unused + srclen < len);
    assert(dest->data >= data && dest->data < data + len);

    strncpy((dest->data + dest->len), src, srclen);

    first_unused += srclen;
    dest->len += srclen;
  }

  void append_str(mystr* dest, mystr src) {
    assert(first_unused + src.len < len);
    assert(dest->data > data && dest->data < data + len);

    strncpy((dest->data + dest->len), src.data, src.len);

    first_unused += src.len;
    dest->len += src.len;
  }

  void realloc_head(mystr* src, size_t size) {
    assert(current_head + size < len);
    assert(is_head(*src));
    assert(src->data >= data && src->data < data + len);

    first_unused = current_head + size;
    src->len = size;
    
  }

  void make_null_term(mystr* src) {
    assert(current_head + src->len + 1 < len);
    assert(is_head(*src));
    assert(src->data >= data && src->data < data + len);

    src->data[src->len] = '\0';
  }

};

struct arena {

  std::list<arena_page> pages;

  arena () {
    pages = std::list<arena_page>();
    pages.emplace_back();
  }

  void dump() {
    for (auto& page : pages) {
      page.dump();
      std::cout << std::endl << "-------------------" << std::endl;
    }
  }

  arena_page* new_page() {
    pages.emplace_back();
    return &pages.back();
  }

  mystr alloc_str(size_t size) {
    arena_page* cur_page = &pages.back();
    if(cur_page->first_unused + size >= cur_page->len) {
      cur_page = new_page();
    }
    return cur_page->alloc_str(size);
  }

  mystr alloc_str_from(const char* from) {
    mystr s = alloc_str(strlen(from));
    strcpy(s.data, from);
    return s;
  }

  void discard_head() {
    pages.back().discard_head();
  }

  bool is_head(mystr s) {
    return pages.back().is_head(s);
  }

  void delete_head(mystr* dest) {
    discard_head();
    dest->data = NULL;
    dest->len = 0;
  }

  mystr alloc_null_term_str(mystr src) {
    arena_page* cur_page = &pages.back();
    if (cur_page->first_unused + src.len + 1 >= cur_page->len) {
      cur_page = new_page();
    }
    return cur_page->alloc_null_term_str(src);
  }

  void append_char(mystr* dest, char c) {
    arena_page* cur_page = &pages.back();
    if (cur_page->first_unused + 1 >= cur_page->len) {
      cur_page = new_page();

      mystr newstr = cur_page->alloc_str(dest->len + 1);
      
      strncpy(newstr.data, dest->data, dest->len);
      *dest = newstr;

      dest->data[dest->len - 1] = c;
    } else {
      cur_page->append_char(dest, c);
    }
  }

  void append_str(mystr* dest, mystr src) {
    arena_page* cur_page = &pages.back();
    if (cur_page->first_unused + src.len >= cur_page->len) {
      cur_page = new_page();

      mystr newstr = cur_page->alloc_str(dest->len + src.len);
      
      strncpy(newstr.data, dest->data, dest->len);
      strncpy(newstr.data + dest->len, src.data, src.len);
      *dest = newstr;

    } else {
      cur_page->append_str(dest, src);
    }
  }

  void append_str(mystr* dest, const char* src) {
    arena_page* cur_page = &pages.back();
    size_t srclen = strlen(src);
    if (cur_page->first_unused + srclen >= cur_page->len) {
      cur_page = new_page();

      mystr newstr = cur_page->alloc_str(dest->len + srclen);
      
      strncpy(newstr.data, dest->data, dest->len);
      strncpy(newstr.data + dest->len, src, srclen);
      *dest = newstr;

    } else {
      cur_page->append_str(dest, src);
    }
  }
  
  void realloc_head(mystr* src, size_t size) {
    arena_page* cur_page = &pages.back();
    if (cur_page->current_head + size >= cur_page->len) {
      cur_page = new_page();

      mystr newstr = cur_page->alloc_str(size);
      strncpy(newstr.data, src->data, size);
      *src = newstr;
      
    } else {
      cur_page->realloc_head(src, size);
    }
  }

  void make_null_term(mystr* src) {
    arena_page* cur_page = &pages.back();
    if (cur_page->current_head + src->len + 1 >= cur_page->len) {
      cur_page = new_page();

      mystr newstr = cur_page->alloc_str(src->len);
      strncpy(newstr.data, src->data, src->len);
      cur_page->make_null_term(&newstr);
      *src = newstr;
      
    } else {
      cur_page->make_null_term(src);
    }
  }

  
};

#endif
