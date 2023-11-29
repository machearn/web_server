#ifndef DATA_BUFFER_H_
#define DATA_BUFFER_H_

#include <streambuf>

namespace web_server {
namespace message {

class DataBuffer: public std::streambuf {
public:
  DataBuffer() = delete;
  DataBuffer(const char* data, std::size_t size) {
    char* p{const_cast<char*>(data)};
    this->setg(p, p, p + size);
  }
};

} // namespace message
} // namespace web_server


#endif // DATA_BUFFER_H_
