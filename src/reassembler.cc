#include "reassembler.hh"
#include <iostream>
#include <algorithm>

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if(data == "") { // filter empty data;
    if(is_last_substring)
      output_.writer().close();
    return;
  }
  if(is_last_substring) {  // save empty info
    if_end = true; end_pos = first_index + data.size() - 1;
  }

  cutString(first_index, data);
  if(data == "") return;

  mergeString(first_index, data);


  if((*cache.begin()).first == need_pos) {
    uint64_t begin = (*cache.begin()).first;
    string str = (*cache.begin()).second;
    output_.writer().push(str);
    need_pos += str.size();
    pend_size -= str.size();
    cache.pop_front();
    if(if_end && begin + str.size() - 1 == end_pos)
      output_.writer().close();
  }

}

uint64_t Reassembler::bytes_pending() const {
  return pend_size;
}

void Reassembler::cutString( uint64_t& first_index, std::string& data ) {
  uint64_t str_begin = 0;
  if(need_pos > first_index)
    str_begin = need_pos - first_index;
  first_index = max(first_index, need_pos);
  if(str_begin > data.size() - 1) {
    data = ""; return;
  }
  uint64_t avail = output_.writer().available_capacity();
  uint64_t max_pos = need_pos + avail - 1;
  if(first_index > max_pos) {
    data = ""; return;
  }
  uint64_t str_len = min(data.size(), max_pos - first_index + 1);
  data = data.substr(str_begin, str_len);
}

void Reassembler::mergeString( uint64_t first_index, std::string& data ) {
  if(cache.empty()) {
    cache.push_back(make_pair(first_index, data));
    pend_size += data.size(); return;
  }
  auto it = cache.begin();
  while (it != cache.end() && (*it).first + (*it).second.size() < first_index ) it ++;

  uint64_t real_index = first_index;
  string real_data = data;
  while (it != cache.end() && (*it).first <= real_index + real_data.size()) {
    int it_b = (*it).first, it_e = it_b + (*it).second.size() - 1;
    int re_b = real_index, re_e = real_index + real_data.size() - 1;

    if(re_b < it_b) {
      if(re_e < it_e) { // 1
        real_data = real_data.substr(0, real_data.size() - (re_e - it_b + 1));
        real_data += (*it).second;
      }
      // 4
    } else {
      if(re_e < it_e) { // 2
        real_data = (*it).second;
        real_index = (*it).first;
      } else { // 3
        string tem_str = real_data.substr(it_e - re_b + 1);
        real_index = (*it).first;
        real_data = (*it).second + tem_str;
      }
    }
    pend_size -= (*it).second.size();
    auto tem_it = it; it ++;
    cache.erase(tem_it);
  }
  cache.insert(it, make_pair(real_index, real_data));
  pend_size += real_data.size();
}


