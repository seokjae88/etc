
#ifndef ASIO_ASYNC_CLIENT_HPP
#define ASIO_ASYNC_CLIENT_HPP

#include "../inc/global.hpp"
#include "../inc/packetManager.hpp"

class client {
public:
    client(boost::asio::io_context &io_context,
           const std::string &host,
           const std::string &port,
           const std::string& filename);
    void asyncReadHeader();
    void asyncReadBody(int code, int size);
    void syncSend(const char *dataByte, int dataLen);
    void asyncSend(const char *dataByte, int dataLen);
    void setOutPath(const std::string& path);

    clock_t start, end;
private:
    void recvDownloadStart(MsgBody* recvData);
    void recvDownloadStop(MsgBody* recvData);
    void recvDownloadData(DataBody* recvData);
    void handle_connect(const boost::system::error_code &err);
    void handle_write(const boost::system::error_code &err, const char *sendData);
    void handleReadHeader(const boost::system::error_code &error, size_t bytes_transferred);
    void handleReadBody(const boost::system::error_code &error, size_t bytes_transferred, int code);

    boost::asio::ip::tcp::socket socket_;
    enum {
        max_length = sizeof(DataPacket)
    };
    char data_[max_length];
    std::string filename;
    std::ofstream outputFile;
    int packetNum;
    std::string outpath;
    int fileSize;
    int progerss;
};

#endif //ASIO_ASYNC_CLIENT_HPP
