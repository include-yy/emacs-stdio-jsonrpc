#include "jsonrpc.hpp"
#include <windows.h>

#define WM_JSONRPC_WAKEUP (WM_USER + 1)

int main() {
    uint32_t main_thread_id = GetCurrentThreadId();

    // 1. Define a Waker to signal the main thread from the reader thread
    auto waker = [main_thread_id]() {
        PostThreadMessage(main_thread_id, WM_JSONRPC_WAKEUP, 0, 0);
        };

    jsonrpc::Conn server(waker);

    // 2. Register a synchronous method
    server.register_method("add", [](const jsonrpc::json& params) {
        return params[0].get<double>() + params[1].get<double>();
        });

    // 3. Register a asynchronous method (non-blocking)
    // Useful for heavy tasks that should not freeze the main message loop.
    server.register_async_method("heavy_task", [](jsonrpc::Context ctx, const jsonrpc::json& params) {
        // Move task to a background thread; 'ctx' and 'params' are captured by value
        std::thread([ctx, params]() mutable {
            std::this_thread::sleep_for(std::chrono::seconds(3));
            ctx.reply("Task Complete!"); // Safely reply from any thread
            }).detach();
        });

    // 4. Register an exit notification (no response)
    server.register_notification("exit", [](const jsonrpc::json&) {
        PostQuitMessage(0); // Signal the GetMessage loop to terminate.
        });

    // Start the background I/O reader thread.
    server.start();

    // 5. Standard Win32 Message Loop
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_JSONRPC_WAKEUP) {
            // Process the message queue on the main thread
            server.process_queue();
            // Check if the reader thread died.
            if (!server.is_running()) {
                break;
            }
        } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    // 6. Graceful Shutdown Implementation
    // Note: CancelIoEx provides a non-destructive way to interrupt a blocking stdin read.
    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != INVALID_HANDLE_VALUE) {
        CancelIoEx(hIn, nullptr); // Forcefully abort pending I/O on the reader thread
    }
    server.stop();
    return 0;
}
