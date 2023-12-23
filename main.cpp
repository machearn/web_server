#include "include/static_server.hpp"
#include <atomic>
#include <signal.h>

std::atomic<bool> quit{false};
std::atomic<int> sig{0};

void sigint_handler(int s) {
  quit = true;
  sig = s;
}

int main() {
  struct sigaction sigint_action;
  sigint_action.sa_handler = sigint_handler;
  sigemptyset(&sigint_action.sa_mask);
  sigint_action.sa_flags = 0;

  sigaction(SIGINT, &sigint_action, nullptr);

  using namespace web_server;
  std::filesystem::path root_dir{"/Users/machearn/var/www/"};
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
