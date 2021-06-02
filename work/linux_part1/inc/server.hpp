#ifndef ASIO_ASYNC_SERVER_HPP
#define ASIO_ASYNC_SERVER_HPP

#include "global.hpp"
#include "session.hpp"

class server {
private:
    void listen() {
        session *new_session = new session(io_service_, this->filepath);
        //client로부터 접속될 때 까지 대기한다.
        acceptor_.async_accept(new_session->socket(),
                               boost::bind(&server::accept, this, new_session,
                                           boost::asio::placeholders::error));
    }
    void accept(session *new_session, const boost::system::error_code &error) {
        if (!error) {
            new_session->start();
        } else {
            delete new_session;
        }
        this->listen();
    }
    boost::asio::io_context &io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::string filepath;
public:
    server(boost::asio::io_context &io_service, short port, const std::string &path) :
            io_service_(io_service),
            acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
        this->filepath = path;
        this->listen();
    }
};

#endif //ASIO_ASYNC_SERVER_HPP
