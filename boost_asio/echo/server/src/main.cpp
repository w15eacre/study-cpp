import echo_server;

#include <boost/asio.hpp>
#include <boost/cobalt.hpp>

#include <print>

namespace io = boost::asio;
namespace co = boost::cobalt;

using tcp = io::ip::tcp;
using namespace std::chrono_literals;

co::main co_main(int argc, char* argv[])
{
    io::cancellation_signal signal{};
    try
    {
        auto server = echo_server::EchoServer(signal.slot(), io::ip::make_address("127.0.0.1"), 8080);

        io::steady_timer timer(co_await co::this_coro::executor, 30s);

        auto task = server.Start();
        auto res = co_await co::race(timer.async_wait(co::use_op), task);

        if (!res)
        {
            std::println("Server timed out after 30 seconds.");
            signal.emit(io::cancellation_type::all);
        }
    }
    catch (std::exception& e)
    {
        std::println("Echo server exception: {}", e.what());
        signal.emit(io::cancellation_type::all);
    }

    co_return 0;
}
