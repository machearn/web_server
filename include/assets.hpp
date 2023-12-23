#ifndef ASSETS_H_
#define ASSETS_H_

#include <string>

namespace web_server {
namespace assets {

inline const std::string NOT_FOUND_RESPONSE{"HTTP/1.1 404 Not Found\r\n"
                                            "Content-Length: 89\r\n"
                                            "Content-Type: text/html\r\n"
                                            "\r\n"
                                            "<html>"
                                            "<head><title>404 Not Found</title></head>"
                                            "<body><h1>404 Not Found</h1></body>"
                                            "</html>"};

inline const std::string BAD_REQUEST_RESPONSE{"HTTP/1.1 400 Bad Request\r\n"
                                              "Content-Length: 93\r\n"
                                              "Content-Type: text/html\r\n"
                                              "\r\n"
                                              "<html>"
                                              "<head><title>400 Bad Request</title></head>"
                                              "<body><h1>400 Bad Request</h1></body>"
                                              "</html>"};

inline const std::string BAD_GATEWAY_RESPONSE{"HTTP/1.1 502 Bad Gateway\r\n"
                                              "Content-Length: 93\r\n"
                                              "Content-Type: text/html\r\n"
                                              "\r\n"
                                              "<html>"
                                              "<head><title>502 Bad Gateway</title></head>"
                                              "<body><h1>502 Bad Gateway</h1></body>"
                                              "</html>"};

} // namespace assets
} // namespace web_server

#endif // ASSETS_H_
