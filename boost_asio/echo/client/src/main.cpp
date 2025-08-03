import tcp_client;

#include <boost/asio.hpp>
#include <boost/cobalt.hpp>
#include <boost/cobalt/race.hpp>

#include <print>
#include <ranges>

namespace io = boost::asio;
namespace co = boost::cobalt;

co::main co_main(int argc, char* argv[])
{
    io::cancellation_signal signal{};
    try
    {
        for (auto attemts : std::views::iota(0, 10))
        {
            auto timer = io::steady_timer(co_await co::this_coro::executor, std::chrono::seconds(10));
            auto endpoint = io::ip::tcp::endpoint(io::ip::make_address("127.0.0.1"), 8080);
            auto client = co_await tcp_client::CreateTcpClient(signal.slot(), endpoint);

            std::string message = std::format("Hello, Echo Server {}!", attemts);
            auto result = co_await co::race(timer.async_wait(co::use_op), client.SendMessage(message));
            if (result.index() == 0)
            {
                std::println("TcpClient timed out after 10 seconds.");
                signal.emit(io::cancellation_type::all);
                continue;
            }

            auto response = boost::variant2::get<1>(result);
            if (!response)
            {
                std::println("Failed to send message: {}", response.error().message());
                continue;
            }

            std::println("Received response: {}", response.value());
        }
        signal.emit(io::cancellation_type::all);
    }
    catch (std::exception& e)
    {
        std::println("TcpClient exception: {}", e.what());
        signal.emit(io::cancellation_type::all);
    }

    co_return 0;
}
