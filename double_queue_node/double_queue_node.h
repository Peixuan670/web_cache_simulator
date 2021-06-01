#ifndef DOUBLE_QUEUE_NODE_H
#define DOUBLE_QUEUE_NODE_H

#include <iostream>
#include <map>
#include <stdint.h>
#include <vector>

class dequeue_node {
public:
  dequeue_node(dequeue_node *previous);
  void copy_arr(std::map<uint32_t, uint32_t> &frag_arr_vnode,
                std::pair<unsigned int, unsigned int> &look_up_res,
                uint32_t content_s);
  void copy_arr(std::map<uint32_t, uint32_t> &frag_arr_vnode,
                std::map<uint32_t, uint32_t> &frag_arr_rnode,
                std::pair<unsigned int, unsigned int> &look_up_res);
  void copy_arr_rnode(std::map<uint32_t, uint32_t> &frag_arr_rnode);

  /*
    for arr, only record the start index of each increment fragment (see the
    paper)
    example : [2, 1, 1, 1, 1, 0 ... 0] -> [0, 1, 5]
    now consider real size of content, should record the true value of the arr
    fragment
    example : [0, 1, 5] -> [<0, 7>, <1, 2>, <5, 0>]
  */
  std::vector<std::pair<uint32_t, uint32_t>> arr; // request array of this vnode
  std::vector<std::pair<uint32_t, uint32_t>>
      arr_rnode; // request array of this vnode
  uint32_t content_size;
  uint32_t size;
  uint32_t size_rnode;
  uint32_t last_access;
  uint32_t c;       // stack distance of this request in its real node
  uint32_t c_vnode; // stack distance of this request in its virtual node
  dequeue_node *prev = nullptr;
  dequeue_node *next = nullptr;
  uint32_t virtual_node;
  uint8_t real_node;
  uint32_t c_value(uint32_t pos);       // stack distance int this rnode
  uint32_t c_value_vnode(uint32_t pos); // stack distance int this vnode
};

#endif
