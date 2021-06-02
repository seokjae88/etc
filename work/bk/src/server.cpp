#include "../inc/server.hpp"

server::server(boost::asio::io_context &io_service, short port, const std::string& path) : io_service_(io_service), filepath(path),
                                              acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
    this->listen();
}

void server::listen() {
    session *new_session = new session(io_service_, this->filepath);
    //client로부터 접속될 때 까지 대기한다.
    acceptor_.async_accept(new_session->socket(),
                           boost::bind(&server::accept, this, new_session,
                                       boost::asio::placeholders::error));
}

void server::accept(session *new_session, const boost::system::error_code &error) {
    if (!error) {
        new_session->start();
    } else {
        delete new_session;
    }
    this->listen();
}