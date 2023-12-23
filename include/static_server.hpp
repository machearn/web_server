#ifndef STATIC_SERVER_H_
#define STATIC_SERVER_H_

#include "assets.hpp"
#include "request.hpp"
#include "server.hpp"

#include <filesystem>
#include <fstream>

namespace web_server {

class StaticServer: public Server<StaticServer> {
  friend class Server<StaticServer>;

public:
  StaticServer(std::uint16_t port, std::filesystem::path root_path, bool custom_error_page = false)
      : Server(port), _root_path(root_path), _custom_error_page(custom_error_page) {}

private:
  std::filesystem::path _root_path;
  bool _custom_error_page;
  std::uint32_t file_size(std::fstream& file);
  std::uint32_t generate_header(std::stringstream& header_ss, const std::string& response_line,
                                std::uint32_t content_size);
  message::Data read_file(std::fstream& file, std::uint32_t content_size,
                          std::uint32_t connection_id, std::stringstream& header_ss,
                          std::uint32_t header_size);
  message::Data implement_handle_request(std::uint32_t connection_id, const message::Data& request);
};

} // namespace web_server

#endif // STATIC_SERVER_H_
