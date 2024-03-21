#include "byte_stream.hh"
#include <stdexcept>
#include <iostream>

using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) , buf() {}

void ByteStream::writeString(std::string str, size_t len) {
  for(size_t i = 0 ; i < len ; i ++)
    buf.push(str[i]);
}

bool Writer::is_closed() const {
  return isClosed_;
}

void Writer::push( string data ) {
  uint64_t data_len = (uint64_t)data.size();
  uint64_t avail = available_capacity();
  if (avail < data_len) {
    writeString(data, avail);
    write_flow += avail;
    return;
  }
  writeString(data, data_len);
  write_flow += data_len;
}

void Writer::push( char ch ) {
  std::string str(1, ch);
  push(str);
}

void Writer::close() {
  isClosed_ = true;
}

uint64_t Writer::available_capacity() const {
  uint64_t nowLen = (uint64_t)buf.size();
  return capacity_ - nowLen;
}

uint64_t Writer::bytes_pushed() const {
  return write_flow;
}

bool Reader::is_finished() const {
  return isClosed_ && buf.empty();
}

uint64_t Reader::bytes_popped() const {
  return read_flow;
}

std::string_view Reader::peek() const {
  return {std::string_view(&buf.front(), 1)};
}

void Reader::pop( uint64_t len ) {
  if(len > bytes_buffered()) {
    throw std::runtime_error( "pop len larger than buffer contain" );
  }
  for(uint64_t i = 0 ; i < len ; i ++)
    buf.pop();
  read_flow += len;
}

uint64_t Reader::bytes_buffered() const {
  return buf.size();
}
