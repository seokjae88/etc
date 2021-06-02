#ifndef ASIO_ASYNC_SERVER_HPP
#define ASIO_ASYNC_SERVER_HPP

#include "global.hpp"
#include "session.hpp"

class server {
public:
    server(boost::asio::io_context &io_service, short port, const std::string& path);

private:
    void listen();
    void accept(session *new_session, const boost::system::error_code &error);

    boost::asio::io_context &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::string filepath;
};

#endif //ASIO_ASYNC_SERVER_HPP
