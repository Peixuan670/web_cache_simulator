#include "double_queue_node.h"

dequeue_node::dequeue_node(dequeue_node *previous) : prev(previous) {}

void dequeue_node::copy_arr(std::map<uint32_t, uint32_t> &frag_arr_vnode,
                            std::pair<unsigned int, unsigned int> &look_up_res,
                            uint32_t content_s) {
  content_size = content_s;
  virtual_node = look_up_res.first;
  real_node = look_up_res.second;
  arr.clear();
  size = frag_arr_vnode.size();
  arr.resize(size);
  std::copy(frag_arr_vnode.begin(), frag_arr_vnode.end(), arr.begin());
}

void dequeue_node::copy_arr(
    std::map<uint32_t, uint32_t> &frag_arr_vnode,
    std::map<uint32_t, uint32_t> &frag_arr_rnode,
    std::pair<unsigned int, unsigned int> &look_up_res) {
  virtual_node = look_up_res.first;
  real_node = look_up_res.second;
  arr.clear();
  size = frag_arr_vnode.size();
  arr.resize(size);
  std::copy(frag_arr_vnode.begin(), frag_arr_vnode.end(), arr.begin());
  arr_rnode.clear();
  size_rnode = frag_arr_rnode.size();
  arr_rnode.resize(size_rnode);
  std::copy(frag_arr_rnode.begin(), frag_arr_rnode.end(), arr_rnode.begin());
}

void dequeue_node::copy_arr_rnode(
    std::map<uint32_t, uint32_t> &frag_arr_rnode) {
  arr_rnode.clear();
  size_rnode = frag_arr_rnode.size();
  arr_rnode.resize(size_rnode);
  std::copy(frag_arr_rnode.begin(), frag_arr_rnode.end(), arr_rnode.begin());
}

uint32_t dequeue_node::c_value(uint32_t pos) {
  if (pos >= arr_rnode[size_rnode - 1].first)
    return 0;
  uint32_t lo = 0, hi = size_rnode;
  uint32_t mid = 0;
  while (lo + 1 < hi) {
    mid = (lo + hi) / 2;
    if (pos < arr_rnode[mid].first) {
      hi = mid;
    } else if (arr_rnode[mid].first < pos) {
      lo = mid;
    } else {
      lo = mid;
      break;
    }
  }
  // uint32_t check = 0;
  // for (size_t i = 0; i < arr_rnode.size(); ++i) {
  //   if (arr_rnode[i].first <= pos && pos < arr_rnode[i + 1].first) {
  //     check = arr_rnode[i].second;
  //     break;
  //   }
  // }
  // if (check != arr_rnode[lo].second)
  //   std::cout<<"fuck";
  return arr_rnode[lo].second;
}

uint32_t dequeue_node::c_value_vnode(uint32_t pos) {
  if (pos >= arr[size - 1].first)
    return 0;
  uint32_t lo = 0, hi = size;
  uint32_t mid = 0;
  while (lo + 1 < hi) {
    mid = (lo + hi) / 2;
    if (pos < arr[mid].first) {
      hi = mid;
    } else if (arr[mid].first < pos) {
      lo = mid;
    } else {
      lo = mid;
      break;
    }
  }
  // uint32_t check = 0;
  // for (size_t i = 0; i < arr.size(); ++i) {
  //   if (arr[i].first <= pos && pos < arr[i + 1].first) {
  //     check = arr[i].second;
  //     break;
  //   }
  // }
  // if (check != arr[lo].second)
  //   std::cout<<"fuck";
  return arr[lo].second;
}