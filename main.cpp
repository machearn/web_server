#include "include/static_server.hpp"
#include <atomic>
#include <signal.h>

std::atomic<bool> quit{false};
std::atomic<int> sig{0};

void sigint_handler(int s) {
  quit = true;
  sig = s;
}

int main(int argc, char** argv) {
  struct sigaction sigint_action;
  sigint_action.sa_handler = sigint_handler;
  sigemptyset(&sigint_action.sa_mask);
  sigint_action.sa_flags = 0;

  sigaction(SIGINT, &sigint_action, nullptr);

  using namespace web_server;
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <root directory>" << std::endl;
    exit(1);
  }
  std::filesystem::path root_dir{argv[1]};
  utils::Logger::logger().info("main::Root directory: " + root_dir.string());
  StaticServer server{8080, root_dir};
  server.start();
  for (;;) {
    if (quit) {
#ifdef DEBUG
      utils::Logger::logger().debug("main::Shutting down server.");
#endif
      server.stop();
      exit(sig);
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  return sig;
}
