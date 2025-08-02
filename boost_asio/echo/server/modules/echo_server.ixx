module;

#include <boost/asio/ip/tcp.hpp>
#include <boost/cobalt.hpp>

#include <expected>

export module echo_server;

namespace io = boost::asio;

export namespace echo_server {
class EchoServer
{
public:
    explicit EchoServer(boost::asio::cancellation_slot slot, io::ip::address address, unsigned short port);

    boost::cobalt::task<void> Start();
private:
    boost::cobalt::generator<std::expected<io::ip::tcp::socket, boost::system::error_code>> Listen();

    boost::cobalt::task<void> ProcessSocket(io::ip::tcp::socket socket);
private:
    io::ip::address m_address{};
    unsigned short m_port{};
    boost::asio::cancellation_slot m_slot{};
};

} // namespace echo_server
