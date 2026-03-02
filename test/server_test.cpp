#include <iostream>
#include <atomic>
#include <chrono>
#include <thread>
#include "jsonrpc.hpp"

using namespace jsonrpc;

std::atomic<bool> g_quit{ false };

int main() {
    auto waker = []() {};

    Conn conn(waker);

    conn.register_method("add", [](const json& params) {
        if (!params.is_array() || params.size() < 2) {
            throw JsonRpcException(spec::kInvalidParams, "Expect an array of 2 numbers");
        }
        return params[0].get<int>() + params[1].get<int>();
        });

    conn.register_method("echo", [](const json& params) {
        return params;
        });

    conn.register_notification("exit", [](const json&) {
        g_quit = true;
        });

    conn.start();

    while (!g_quit && conn.is_running()) {
        conn.process_queue();
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // Need EOF
    conn.stop();
    return 0;
}
