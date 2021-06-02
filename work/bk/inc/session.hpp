#ifndef ASIO_ASYNC_SESSION_HPP
#define ASIO_ASYNC_SESSION_HPP

#include "global.hpp"
#include "packetManager.hpp"

class session {
public:
    session(boost::asio::io_context &io_service, const std::string& path);
    ~session();

    boost::asio::ip::tcp::socket &socket();
    void start();
    void asyncReadHeader();
    void asyncReadBody(int code, int size);
    void asyncWrite(const char* dataByte, int dataLen);
    void socketClose();
    void setFilePath(const std::string& path);

private:
    void recvLoginReq(MsgBody* recvData);
    void recvDownloadStart(MsgBody* recvData);
    void recvDownloadData(MsgBody* recvData);
    void handleReadHeader(const boost::system::error_code &error, size_t bytes_transferred);
    void handleReadBody(const boost::system::error_code &error, size_t bytes_transferred, int code);
    void handle_write(const boost::system::error_code &error, const char *sendData);
    void onClosed();

    boost::asio::ip::tcp::socket socket_;
    enum {
        max_length = sizeof(DataPacket)
    };
    char data_[max_length];
    std::ifstream sourceFile;
    int packetNum;
    std::string filepath;
};

#endif //ASIO_ASYNC_SESSION_HPP
