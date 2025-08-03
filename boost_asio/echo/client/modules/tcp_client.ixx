module;

#include <boost/asio.hpp>
#include <boost/cobalt.hpp>

#include <expected>

export module tcp_client;

namespace io = boost::asio;
namespace co = boost::cobalt;

export namespace tcp_client {

class TcpClient
{
public:
    explicit TcpClient(boost::asio::cancellation_slot slot, io::ip::tcp::socket socket);

    co::promise<std::expected<std::string, boost::system::error_code>> SendMessage(std::string message);
private:
    io::ip::tcp::socket m_socket;
    io::cancellation_slot m_slot{};
};

co::promise<TcpClient> CreateTcpClient(io::cancellation_slot slot, io::ip::tcp::endpoint endpoint);

} // namespace tcp_client
