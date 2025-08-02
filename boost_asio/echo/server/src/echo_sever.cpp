module;

#include <boost/asio.hpp>
#include <boost/cobalt.hpp>

#include <array>
#include <exception>
#include <expected>
#include <print>

module echo_server;

namespace co = boost::cobalt;

namespace echo_server {

EchoServer::EchoServer(boost::asio::cancellation_slot slot, io::ip::address address, unsigned short port)
    : m_address(std::move(address)), m_port(port), m_slot(std::move(slot))
{}

boost::cobalt::task<void> EchoServer::Start()
{
    try
    {
        std::println("Starting EchoServer on {}:{}", m_address.to_string(), m_port);

        auto executor = co_await co::this_coro::executor;
        auto socketGenerator = Listen();

        while (auto result = co_await socketGenerator)
        {
            co_await ProcessSocket(std::move(result.value()));
        }
    }
    catch (const std::exception& exp)
    {
        std::println("EchoServer::Start exception: {}", exp.what());
    }

    co_return;
}

boost::cobalt::generator<std::expected<io::ip::tcp::socket, boost::system::error_code>> EchoServer::Listen()
{
    try
    {
        io::ip::tcp::acceptor acceptor(co_await co::this_coro::executor, {m_address, m_port});

        for (;;)
        {
            auto sock = co_await acceptor.async_accept(io::bind_cancellation_slot(m_slot, co::use_op));

            std::println("Accepted connection from {}", sock.remote_endpoint().address().to_string());
            co_yield std::move(sock);
        }
    }
    catch (const std::exception& exp)
    {
        std::println("EchoServer::Listen exception: {}", exp.what());
    }

    co_return std::unexpected(boost::asio::error::operation_aborted);
}

co::task<void> EchoServer::ProcessSocket(io::ip::tcp::socket socket)
{
    try
    {
        std::array<std::byte, 1024> buff;

        std::println("Reading from socket...");
        auto count = co_await socket.async_read_some(io::buffer(buff), io::bind_cancellation_slot(m_slot, co::use_op));
        std::println("Read {} bytes from socket", count);

        std::println("Writing from socket...");
        auto written = co_await socket.async_write_some(io::buffer(buff, count), io::bind_cancellation_slot(m_slot, co::use_op));
        std::println("Wrote {} bytes to socket", written);
    }
    catch (const std::exception& e)
    {
        std::println("EchoServer::ProcessSocket exception: {}", e.what());
    }

    co_return;
}

} // namespace echo_server
