#include "consistent_hash.h"

// the hash ring size is 0 ~ 2**32
#define HASH_LEN 32

unsigned int MurMurHash(const void *key, int len) {
  const unsigned int m = 0x5bd1e995;
  const int r = 24;
  const int seed = 97;
  unsigned int h = seed ^ len;
  // Mix 4 bytes at a time into the hash
  const unsigned char *data = (const unsigned char *)key;
  while (len >= 4) {
    unsigned int k = *(unsigned int *)data;
    k *= m;
    k ^= k >> r;
    k *= m;
    h *= m;
    h ^= k;
    data += 4;
    len -= 4;
  }
  // Handle the last few bytes of the input array
  switch (len) {
  case 3:
    h ^= data[2] << 16;
  case 2:
    h ^= data[1] << 8;
  case 1:
    h ^= data[0];
    h *= m;
  };
  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
}
/*
unsigned int MurMurHash(const void* key, int len)
{
    const unsigned char* data = (const unsigned char*)key;
    unsigned int k = *(unsigned int*)data;
    return k % len;
}
*/

// Peixuan 09262020
unsigned int SimpleHash(const void *key, int len) {
  const unsigned char *data = (const unsigned char *)key;
  unsigned int k = *(unsigned int *)data;
  return k % len;
}

std::pair<unsigned int, unsigned int>
consistent_hash::look_up_simple(const std::string &content) {
  // return the ip of virtual node that serve this content
  auto hash_position = SimpleHash(content.c_str(), HASH_LEN);
  auto virtual_node_index = find_nearest_node_simple(hash_position);
  virtual_node_index++;
  if (virtual_node_index >= sorted_node_hash_list.size()) { // cross the zero
    virtual_node_index = 0;
  }
  return std::pair<unsigned int, unsigned int>(
      virtual_node_index,
      virtual_node_map.find(sorted_node_hash_list[virtual_node_index])
          ->second.cache_index);
}

unsigned int consistent_hash::find_nearest_node_simple(
    unsigned int
        hash_value) { // find the nearest virtual node for given hash position
  int low = 0;
  int high = sorted_node_hash_list.size() - 1;
  int mid;
  if (hash_value > sorted_node_hash_list[high]) {
    return 0;
  }
  while (low < high) {
    mid = (low + high) / 2;
    if (sorted_node_hash_list[mid] == hash_value) {
      return mid;
    } else if (sorted_node_hash_list[mid] > hash_value) {
      high = mid;
    } else { // sorted_node_hash_list[mid]<data_hash
      low = mid + 1;
    }
  }
  return low;
}

consistent_hash::consistent_hash() {
  real_node_sum = 0;
  virtual_node_sum = 0;
}

consistent_hash::~consistent_hash() {
  virtual_node_map.clear();
  real_node_map.clear();
  sorted_node_hash_list.clear();
}

unsigned int consistent_hash::find_nearest_node(
    unsigned int
        hash_value) { // find the nearest virtual node for given hash position
  int low = 0;
  int high = sorted_node_hash_list.size() - 1;
  int mid;
  if (hash_value > sorted_node_hash_list[high]) {
    return 0;
  }
  while (low < high) {
    mid = (low + high) / 2;
    if (sorted_node_hash_list[mid] == hash_value) {
      return mid;
    } else if (sorted_node_hash_list[mid] > hash_value) {
      high = mid;
    } else { // sorted_node_hash_list[mid]<data_hash
      low = mid + 1;
    }
  }
  return low;
}

std::pair<unsigned int, unsigned int>
consistent_hash::look_up(const std::string &content) {
  // return the ip of virtual node that serve this content
  auto hash_position = MurMurHash(content.c_str(), content.length());
  auto virtual_node_index = find_nearest_node(hash_position);
  virtual_node_index++;
  if (virtual_node_index >= sorted_node_hash_list.size()) { // cross the zero
    virtual_node_index = 0;
  }

  // Peixuan 10262020: File : vnode and rnode map:

  fileID_vnode_map[content] = virtual_node_index;
  fileID_rnode_map[content] =
      virtual_node_map.find(sorted_node_hash_list[virtual_node_index])
          ->second.cache_index;

  return std::pair<unsigned int, unsigned int>(
      virtual_node_index,
      virtual_node_map[sorted_node_hash_list[virtual_node_index]].cache_index);
}

void consistent_hash::add_real_node(std::string ip,
                                    unsigned int virtual_node_num) {
  std::cout << "[add_real_node]\t" << ip << std::endl;
  real_node *node;
  if (real_node_map.find(ip) !=
      real_node_map.end()) { // this real node has added before
    node = &real_node_map[ip];
  } else {
    real_node new_node = real_node(ip);
    node = &new_node;
    real_node_sum++;
  }
  int cur_port = node->cur_max_port;
  unsigned int vir_node_num = 0;
  std::string tmp_ip;    // the ip of virtual node
  unsigned int tmp_hash; // the position of virtual node on hash ring
  while (vir_node_num < virtual_node_num) {
    do { // find a  virtual node and avoid collision
      cur_port++;
      tmp_ip = ip + ":" + std::to_string(cur_port);
      tmp_hash = MurMurHash(tmp_ip.c_str(),
                            tmp_ip.length()); // 09262020 Peixuan : simple hash
      // tmp_hash = SimpleHash(tmp_ip.c_str(), HASH_LEN);    // 09262020 Peixuan
      // : simple hash
      // double ratio = 1/3;
      // tmp_hash = HASH_LEN*ratio*vir_node_num + HASH_LEN*ratio;    // 09262020
      // Peixuan : simple hash
    } while (virtual_node_map.find(tmp_hash) != virtual_node_map.end());
    vir_node_num++;
    virtual_node_map[tmp_hash] =
        virtual_node(tmp_ip, tmp_hash, real_node_sum - 1);
    sorted_node_hash_list.push_back(tmp_hash);
    sort(sorted_node_hash_list.begin(), sorted_node_hash_list.end());
    unsigned int id = find_nearest_node(tmp_hash);
    unsigned int next_id = id + 1;
    if (next_id >= sorted_node_hash_list.size()) { // cross the zero
      next_id = 0;
    }
    // below is data immigration, here only provide look_up service
    // unsigned int next_hash = sorted_node_hash_list[next_id];
    // std::vector<unsigned int> tobe_deleted;
    // std::map<unsigned int, std::string> *tobe_robbed =
    // &(virtual_node_map[next_hash].data);
    // for (auto data = tobe_robbed->begin(); data != tobe_robbed->end();
    // data++)
    // {
    //     if (data->first < tmp_hash)
    //     {
    //         virtual_node_map[tmp_hash].data[data->first] =
    //         data->second;
    //         tobe_deleted.push_back(data->first);
    //     }
    // }
    // for (auto deleted : tobe_deleted)
    // {
    //     tobe_robbed->erase(deleted);
    //     cout << "[move data]\t" << deleted << "\tfrom node:\t" <<
    //     virtual_node_map[next_hash].ip << "("
    //          << next_hash << ")"
    //          << "\tto\t"
    //          << virtual_node_map[tmp_hash].ip << "(" << tmp_hash << ")"
    //          << endl;
    // }
    // node->virtual_node_hash_list.push_back(tmp_hash);
  }
  node->cur_max_port = cur_port;
  node->virtual_node_num += virtual_node_num;
  real_node_map[ip] = *node;

  virtual_node_sum += virtual_node_num;
  std::cout << "[add_real_node finished]\t" << ip << std::endl << std::endl;
}

void consistent_hash::add_real_node_assign(
    std::string ip, unsigned int vnode_num_to_assign,
    unsigned int starting_id) // 10102020 Peixuan : uneven hash
{
  std::cout << "[add_real_node_assign]\t" << ip << std::endl;
  real_node *node;
  if (real_node_map.find(ip) !=
      real_node_map.end()) { // this real node has added before
    node = &real_node_map[ip];
  } else {
    real_node new_node = real_node(ip);
    node = &new_node;
    real_node_sum++;
  }

  for (unsigned int assigned_vnode = 0; assigned_vnode < vnode_num_to_assign;
       assigned_vnode++) {
    // std::cout << "[assiging]\t" << assigned_vnode << " node" << std::endl;
    // // 10122020 Peixuan vnode debug
    std::string tmp_ip = ip + ":" + std::to_string(assigned_vnode);

    // std::cout << "[UID: ]\t" << starting_id+assigned_vnode << std::endl;
    // // 10122020 Peixuan vnode debug

    unsigned int hash_value = uid_map_hash_value[starting_id + assigned_vnode];

    // std::cout << "[UID = ]\t" << virtual_node_map[hash_value].uid <<
    // std::endl; // 10122020 Peixuan vnode debug

    // std::cout << "[Hash_Value = ]\t" <<
    // virtual_node_map[hash_value].hash_value << std::endl; // 10122020
    // Peixuan vnode debug

    virtual_node_map[hash_value].SetIP(tmp_ip); // 10102020 Peixuan :
                                                // Update the tmp_ip and
                                                // real node index when
                                                // assigning the vnode

    virtual_node_map[hash_value].SetCacheIndex(
        real_node_sum - 1); // 10102020 Peixuan : Update the tmp_ip and real
                            // node index when assigning the vnode

    // unsigned int tmp_hash = MurMurHash(tmp_ip.c_str(), tmp_ip.length()); //
  }

  real_node_map[ip] = *node;

  std::cout << "[add_real_node finished]\t" << ip << std::endl << std::endl;
}

void consistent_hash::initial_virtual_node(
    unsigned int virtual_node_num) // 10102020 Peixuan : uneven hash
{
  // std::cout << "[Initializeing virtual nodes (Peixuan)]: \t" <<
  // virtual_node_num << std::endl; // 10122020 Peixuan vnode debug
  unsigned int vir_node_num = 0;

  std::string ip = "0.0.0.0";
  std::string tmp_ip; // the ip of virtual node
  int cur_port = 0;

  unsigned int tmp_hash = 0; // the position of virtual node on hash ring
  double tmp_hash_dub = 0;   // the position of virtual node on hash ring
  while (vir_node_num < virtual_node_num) {
    tmp_ip = ip + ":" + std::to_string(cur_port);

    // std::string max_str = "255.255.255.255:65535";
    // unsigned int max_hash = MurMurHash(max_str.c_str(), max_str.length()); //
    // 10122020 Peixuan: getting a large hash value for the maximum hash value
    unsigned int max_hash = 4294967295; //   ymj 20201013

    // std::cout << "[Max hash (Peixuan)]: \t" << max_hash << std::endl;

    double ratio = 0.25; // 09262020 Peixuan : simple hash

    // tmp_hash = tmp_hash + (max_hash - tmp_hash)*ratio;    // 09262020 Peixuan
    // : simple hash

    tmp_hash_dub = tmp_hash + (max_hash - tmp_hash) * ratio;
    // std::cout << "[tmp hash (Peixuan)]: \t" << tmp_hash << " + " << (max_hash
    // - tmp_hash) << "*" << ratio << "=" << tmp_hash_dub << std::endl;

    tmp_hash = (unsigned int)tmp_hash_dub;
    // std::cout << "[tmp hash dub (Peixuan)]: \t" << tmp_hash << std::endl;

    vir_node_num++;

    // This is from original

    // std::cout << "[This is vnodes (Peixuan)]: \t" << vir_node_num <<
    // std::endl; // 10122020 Peixuan vnode debug
    virtual_node new_vnode =
        virtual_node(tmp_ip, tmp_hash, 0, vir_node_num); // uid starting from 1

    new_vnode.SetUid(vir_node_num);
    new_vnode.SetHashValue(tmp_hash);

    virtual_node_map[tmp_hash] =
        new_vnode; // ****************��������ֻ�������һ��vnode��ӳ���ϵ***********************
                   // ymj 20201012
    uid_map_hash_value[vir_node_num] = tmp_hash;

    sorted_node_hash_list.push_back(tmp_hash);

    sort(sorted_node_hash_list.begin(), sorted_node_hash_list.end());

    unsigned int id = find_nearest_node(tmp_hash);

    unsigned int next_id = id + 1;

    if (next_id >= sorted_node_hash_list.size()) { // cross the zero
      next_id = 0;
    }
  }
  virtual_node_sum = virtual_node_num; // ymj 20201012
}

// void consistent_hash::print_real_node(std::string ip)
// {
//     std::cout << "------------consistent_hash.print_real_node------------" <<
//     std::endl;
//     std::cout << "real_node ip:" << ip << "\tvirtual_node_num=" <<
//     real_node_map[ip].virtual_node_num << std::endl;
//     for (auto tmp : real_node_map[ip].virtual_node_hash_list)
//     {
//         if (virtual_node_map[tmp].data.size() > 0)
//         {
//             cout << "virtual node:\t" << virtual_node_map[tmp].ip <<
//             "(" << tmp << ")"
//                  << "\thas data:";
//             for (auto data : virtual_node_map[tmp].data)
//             {
//                 cout << "(" << data.second << "," << data.first << ")\t";
//             }
//             cout << endl;
//         }
//     }
//     cout << endl;
// }

// void consistent_hash::print()
// {
//     cout << endl
//          << "------------consistent_hash.print()------------" << endl;
//     cout << "real_node_sum:\t" << real_node_sum <<
//     "\tvirtual_node_sum:\t" << virtual_node_sum << endl;
//     cout << endl;
//     for (auto tmp = real_node_map.begin(); tmp !=
//     real_node_map.end(); tmp++)
//     {
//         print_real_node(tmp->first);
//     }
// }

// unsigned int consistent_hash::put(string data_id)
// {
//     unsigned int data_hash = my_getMurMurHash(data_id.c_str(),
//     data_id.length());
//     unsigned int id = find_nearest_node(data_hash);
//     unsigned int put_on_virnode_hash = sorted_node_hash_list[id];
//     virtual_node_map[put_on_virnode_hash].data.insert(make_pair(data_hash,
//     data_id));
//     cout << "data:\t" << data_id << "(" << data_hash << ")\twas put on
//     virtual node:"
//          << virtual_node_map[put_on_virnode_hash].ip << "(" <<
//          put_on_virnode_hash << ")"
//          << endl;
//     return 0;
// }

// void consistent_hash::drop_real_node(std::string ip)
// {
//     std::cout << "[drop_real_node]\t" << ip << std::endl;
//     std::vector<unsigned int> *virtual_hash_list_p =
//     &real_node_map[ip].virtual_node_hash_list;
//     sort(virtual_hash_list_p->begin(), virtual_hash_list_p->end());
//     unsigned int next_id;
//     unsigned int next_hash;
//     unsigned int cur_id;
//     unsigned int cur_hash;
//     std::vector<unsigned int> tobe_delete;
//     for (int i = virtual_hash_list_p->size() - 1; i >= 0; i--)
//     {
//         cur_hash = (*virtual_hash_list_p)[i];
//         tobe_delete.push_back(cur_hash);
//         if (virtual_node_map[cur_hash].data.size() > 0)
//         {
//             cur_id = find_nearest_node(cur_hash);
//             next_id = cur_id;
//             std::string next_realnode_ip;
//             do
//             {
//                 next_id++;
//                 if (next_id >= sorted_node_hash_list.size())
//                 {
//                     next_id = 0;
//                 }
//                 next_hash = sorted_node_hash_list[next_id];
//                 next_realnode_ip = virtual_node_map[next_hash].ip;
//             } while (next_realnode_ip.find(ip) != -1);
//             std::map<unsigned int, std::string> *moveto =
//             &(virtual_node_map[next_hash].data);
//             for (auto &data : virtual_node_map[cur_hash].data)
//             {
//                 (*moveto)[data.first] = data.second;
//                 std::cout << "[move data]\t" << data.second << "\tfrom
//                 node:\t" << virtual_node_map[cur_hash].ip << "("
//                      << cur_hash << ")"
//                      << "\tto\t"
//                      << virtual_node_map[next_hash].ip << "(" <<
//                      next_hash << ")" << std::endl;
//             }
//         }
//     }
//     for (auto hash : tobe_delete)
//     {
//         virtual_node_map.erase(cur_hash);
//         virtual_node_sum--;
//         auto iter = find(sorted_node_hash_list.begin(),
//                          sorted_node_hash_list.end(), hash);
//         if (iter != sorted_node_hash_list.end())
//         {
//             sorted_node_hash_list.erase(iter);
//         }
//     }
//     sort(sorted_node_hash_list.begin(),
//     sorted_node_hash_list.end());
//     real_node_map[ip].virtual_node_hash_list.clear();
//     real_node_map.erase(ip);
//     real_node_sum--;
//     std::cout << "[drop_real_node finished]\t" << ip << std::endl
//          << std::endl;
// }
