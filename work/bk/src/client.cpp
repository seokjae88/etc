
#include "../inc/client.hpp"

client::client(boost::asio::io_context &io_context,
               const std::string &host,
               const std::string &port,
               const std::string &filename) : socket_(io_context), filename(filename), packetNum(0), fileSize(0),
                                              progerss(0) {
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::query query(host, port);
    boost::asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    //비동기(Unblock) 상태로 서버와 접속한다.
    boost::asio::async_connect(this->socket_, endpoint_iterator,
                               boost::bind(&client::handle_connect, this, boost::asio::placeholders::error));
}

void client::handleReadHeader(const boost::system::error_code &error, size_t bytes_transferred) {
    if (!error) {
        HeaderPacket *pHeader = (HeaderPacket *) this->data_;
        int code = pHeader->getCode();
        int size = pHeader->getSize();

        unsigned short crc = (pHeader->getCode()^pHeader->getErrCode()^pHeader->getSize());
        if (crc == pHeader->getCrc()) {
            this->asyncReadBody(code, size);
        } else {
            std::cout << "crc error!!\n";
            this->asyncReadBody(code, size);
        }
    } else if (boost::asio::error::would_block) {
        std::cout << "would_block!\n";
    } else {
        std::cout << "handle_read err!\n";
    }
}

void client::handleReadBody(const boost::system::error_code &error, size_t bytes_transferred, int code) {
    if (!error) {
        switch (code) {
            case LOGIN_RES_CODE: {
                std::cout << "recv LOGIN_RES_CODE\n";
                this->asyncSend(
                        (char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_START_CODE, ErrCode::NONE, this->filename),
                        sizeof(MsgPacket));
                break;
            }
            case DOWNLOAD_RES_START_CODE: {
                std::cout << "recv DOWNLOAD_RES_START_CODE\n";
                this->recvDownloadStart((MsgBody *) this->data_);
                break;
            }
            case DOWNLOAD_RES_DATA_CODE: {
                this->recvDownloadData((DataBody *) this->data_);
                break;
            }
            case DOWNLOAD_RES_STOP_CODE: {
                this->recvDownloadStop((MsgBody *) this->data_);
                std::cout << "recv DOWNLOAD_REQ_CODE-STOP\n";
                break;
            }
            default: {
                std::cout << "unknown code!!\n";
                this->asyncSend((char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_START_CODE, ErrCode::UNKNOWN_CODE, ""),
                                 sizeof(MsgPacket));
                break;
            }
        }
    } else if (boost::asio::error::would_block) {
        std::cout << "would_block!\n";
    } else {
        std::cout << "handle_read err!\n";
    }
}

void client::recvDownloadStart(MsgBody *recvData) {
    std::cout << "file info : " << recvData->getMsg() << "\n";
    this->fileSize = atoi(recvData->getMsg());
    this->outputFile.open(this->outpath + this->filename,
                          std::ofstream::trunc | std::ios_base::out | std::ios_base::binary);
    if (this->outputFile.is_open()) {
        std::cout << "file open!!! / send DOWNLOAD_REQ_CODE-DATA\n";
        this->asyncSend((char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_DATA_CODE, ErrCode::NONE, ""),
                        sizeof(MsgPacket));
        std::cout << "[";
        this->start = clock();
    } else {
        std::cout << "file open fail!!\n";
        this->asyncSend((char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_DATA_CODE, ErrCode::FILE_OPEN_FAIL, this->filename), sizeof(MsgPacket));
    }
}
void client::recvDownloadData(DataBody *recvData) {
    if (this->outputFile.is_open()) {
        if (recvData->getLength() > 0) {
            if (recvData->getPacketNum() == this->packetNum) {
                this->packetNum++;
                this->outputFile.write(recvData->data_, recvData->getLength());
                std::cout << this->packetNum << "\r";
                this->asyncSend((char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_DATA_CODE, ErrCode::NONE, ""), sizeof(MsgPacket));
            } else {
                this->asyncSend((char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_DATA_CODE, ErrCode::PACKET_NUM_WRONG,std::to_string(this->packetNum)), sizeof(MsgPacket));
            }
        } else {
            this->asyncSend((char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_DATA_CODE, ErrCode::PACKET_LENGTH_WRONG, ""), sizeof(MsgPacket));
        }
    } else {
        std::cout << "file open fail!!\n";
        this->asyncSend((char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_DATA_CODE, ErrCode::FILE_OPEN_FAIL, this->filename), sizeof(MsgPacket));
    }
}
void client::recvDownloadStop(MsgBody *recvData) {
    std::cout << "]\n";
    std::cout << "recv DOWNLOAD_RES_CODE-STOP\n";
    if (this->outputFile.is_open()) {
        this->end = clock();
        this->outputFile.close();
        std::cout << "file closed!!! / send DOWNLOAD_REQ_CODE-STOP\n";
        std::cout << "total packet : " << this->packetNum << "\n";
        this->asyncSend((char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_STOP_CODE, ErrCode::NONE, this->filename), sizeof(MsgPacket));
    } else {
        std::cout << "file open fail!!\n";
        this->asyncSend((char *) packetManager::makeMsgPacket(DOWNLOAD_REQ_STOP_CODE, ErrCode::FILE_OPEN_FAIL, this->filename), sizeof(MsgPacket));
    }
}
void client::handle_connect(const boost::system::error_code &err) {
    if (!err) {
        std::cout << "Connected!\n";
        std::cout << "send LOGIN_REQ_CODE!\n";
        this->asyncSend((char *) packetManager::makeMsgPacket(LOGIN_REQ_CODE, ErrCode::NONE, "seokjae_1111"), sizeof(MsgPacket));
    } else {
        std::cout << "Connect Error!\n";
    }
}
void client::handle_write(const boost::system::error_code &error, const char *sendData) {
    if (sendData) {
        delete sendData;
    }
    if (!error) {
        this->asyncReadHeader();
    } else {
        std::cout << "handle_write err!\n";
    }
}
void client::setOutPath(const std::string &path) {
    this->outpath = path;
}
void client::syncSend(const char *dataByte, int dataLen) {
    boost::asio::write(this->socket_, boost::asio::buffer(dataByte, dataLen));
}
void client::asyncSend(const char *dataByte, int dataLen) {
    boost::asio::async_write(socket_, boost::asio::buffer(dataByte, dataLen),
                             boost::bind(&client::handle_write, this, boost::asio::placeholders::error, dataByte));
}
void client::asyncReadHeader() {
    boost::asio::async_read(this->socket_, boost::asio::buffer(this->data_, sizeof(HeaderPacket)),
                            boost::bind(&client::handleReadHeader, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}
void client::asyncReadBody(int code, int size) {
    boost::asio::async_read(this->socket_, boost::asio::buffer(this->data_, size),
                            boost::bind(&client::handleReadBody, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred,
                                        code));
}