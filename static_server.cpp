#include "include/static_server.hpp"

namespace web_server {
std::uint32_t StaticServer::file_size(std::fstream& file) {
  file.seekg(0, std::ios::end);
  std::size_t content_size = file.tellg();
  file.seekg(0, std::ios::beg);
  return content_size;
}

std::uint32_t StaticServer::generate_header(std::stringstream& header_ss,
                                            const std::string& response_line,
                                            std::uint32_t content_size) {
  header_ss << response_line << "\r\n"
            << "Content-Length: " << content_size << "\r\n"
            << "Content-Type: text/html\r\n"
            << "\r\n";

  header_ss.seekg(0, std::ios::end);
  std::size_t header_size = header_ss.tellg();
  header_ss.seekg(0, std::ios::beg);

  return header_size;
}

message::Data StaticServer::read_file(std::fstream& file, std::uint32_t content_size,
                                      std::uint32_t connection_id, std::stringstream& header_ss,
                                      std::uint32_t header_size) {
  message::Data data(header_size + content_size);

  data.append(header_ss, header_size);
  data.append(file, content_size);

  data.set_connection_id(connection_id);

  return data;
}

message::Data StaticServer::implement_handle_request(std::uint32_t connection_id,
                                                     const message::Data& data) {
  message::Request request(data);
  std::string_view path_view(request.header().path());
#ifdef DEBUG
  utils::Logger::logger().debug("StaticServer::Request path: " + std::string(path_view));
#endif

  std::filesystem::path file_path(_root_path);
  file_path.append(path_view.substr(1));
#ifdef DEBUG
  utils::Logger::logger().debug("StaticServer::File path: " + file_path.string());
#endif

  if (std::filesystem::exists(file_path) && std::filesystem::is_directory(file_path)) {
    file_path.append("index.html");
  }

  if (std::filesystem::exists(file_path)) {
    std::fstream file(file_path, std::ios::in);
    std::stringstream header_ss;

    auto content_size = file_size(file);
    auto header_size = generate_header(header_ss, "HTTP/1.1 200 OK", content_size);

    return read_file(file, content_size, connection_id, header_ss, header_size);
  } else {
    std::filesystem::path error_file_path(_root_path);
    error_file_path.append("404.html");
    if (_custom_error_page && std::filesystem::exists(error_file_path)) {
      std::fstream file(error_file_path, std::ios::in);
      std::stringstream header_ss;

      auto content_size = file_size(file);
      auto header_size = generate_header(header_ss, "HTTP/1.1 404 Not Found", content_size);

      return read_file(file, content_size, connection_id, header_ss, header_size);
    } else {
      message::Data data(reinterpret_cast<const std::uint8_t*>(assets::NOT_FOUND_RESPONSE.data()),
                         assets::NOT_FOUND_RESPONSE.size(), connection_id);
      return data;
    }
  }
}

} // namespace web_server
