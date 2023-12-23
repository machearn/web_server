#ifndef LOGGER_H_
#define LOGGER_H_

#include <iomanip>
#include <iostream>
#include <sstream>

namespace web_server {
namespace utils {

enum class LogLevel { debug, info, warning, error, critical };

class Logger {
public:
  Logger() = delete;
  Logger(const Logger&) = delete;
  Logger(Logger&&) = delete;
  Logger& operator=(const Logger&) = delete;
  Logger& operator=(Logger&&) = delete;

  static Logger& logger(std::ostream& out = std::cout) {
    static Logger instance(out);
    return instance;
  }

  void log(LogLevel level, const std::string& message) {
    std::lock_guard<std::mutex> lock(_mutex);
    _out << get_time_string() << " ";
    switch (level) {
    case LogLevel::debug:
      _out << "[DEBUG]: " << message << std::endl;
      break;
    case LogLevel::info:
      _out << "[INFO]: " << message << std::endl;
      break;
    case LogLevel::warning:
      _out << "[WARNING]: " << message << std::endl;
      break;
    case LogLevel::error:
      _out << "[ERROR]: " << message << std::endl;
      break;
    case LogLevel::critical:
      _out << "[CRITICAL]: " << message << std::endl;
      break;
    }
  }

  void debug(const std::string& message) { log(LogLevel::debug, message); }
  void info(const std::string& message) { log(LogLevel::info, message); }
  void warning(const std::string& message) { log(LogLevel::warning, message); }
  void error(const std::string& message) { log(LogLevel::error, message); }
  void critical(const std::string& message) { log(LogLevel::critical, message); }

private:
  Logger(std::ostream& out): _out(out) {}
  std::string get_time_string() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = std::localtime(&now_time_t);
    std::stringstream ss;
    ss << std::put_time(now_tm, "[%Y-%m-%d %H:%M:%S]");
    return ss.str();
  }

  std::ostream& _out;
  std::mutex _mutex;
};

} // namespace utils
} // namespace web_server

#endif // LOGGER_H_
