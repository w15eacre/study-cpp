module;

#include <boost/asio.hpp>
#include <boost/cobalt.hpp>

#include <expected>
#include <print>
#include <string>

module tcp_client;

namespace io = boost::asio;
namespace co = boost::cobalt;

namespace tcp_client {

TcpClient::TcpClient(io::cancellation_slot slot, io::ip::tcp::socket socket) : m_socket(std::move(socket)), m_slot(std::move(slot))
{
    if (!m_socket.is_open())
    {
        throw std::runtime_error("Socket is not open");
    }
}

co::promise<std::expected<std::string, boost::system::error_code>> TcpClient::SendMessage(std::string message)
{
    boost::system::error_code ec;

    std::println("Sending message: {}", message);
    if (co_await m_socket.async_write_some(io::buffer(message), io::redirect_error(io::bind_cancellation_slot(m_slot, co::use_op), ec)) !=
        message.size())
    {
        co_return std::unexpected(boost::asio::error::broken_pipe);
    }

    if (ec)
    {
        co_return std::unexpected(ec);
    }

    std::println("Message sent successfully, waiting for response...");

    if (co_await m_socket.async_read_some(io::buffer(message), io::redirect_error(io::bind_cancellation_slot(m_slot, co::use_op), ec)) !=
        message.size())
    {
        co_return std::unexpected(boost::asio::error::broken_pipe);
    }

    if (ec)
    {
        co_return std::unexpected(ec);
    }

    co_return message;
}

co::promise<TcpClient> CreateTcpClient(io::cancellation_slot slot, io::ip::tcp::endpoint endpoint)
{
    io::ip::tcp::socket socket(co_await co::this_coro::executor);

    std::println("Connecting to {}:{}", endpoint.address().to_string(), endpoint.port());
    co_await socket.async_connect(endpoint, io::bind_cancellation_slot(slot, co::use_op));

    co_return TcpClient(std::move(slot), std::move(socket));
}

} // namespace tcp_client
