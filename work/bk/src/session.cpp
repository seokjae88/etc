#include "../inc/session.hpp"

session::session(boost::asio::io_context &io_service, const std::string& path) : socket_(io_service), filepath(path), packetNum(0) {
}

session::~session() {
    if (this->sourceFile && this->sourceFile.is_open()) {
        this->sourceFile.close();
    }
}

boost::asio::ip::tcp::socket &session::socket() {
    return this->socket_;
}

void session::handleReadHeader(const boost::system::error_code &error, size_t bytes_transferred) {
    if (!error) {
        HeaderPacket *pHeader = (HeaderPacket *) this->data_;
        int code = pHeader->getCode();
        int size = pHeader->getSize();

        unsigned short crc = (pHeader->getCode()^pHeader->getErrCode()^pHeader->getSize());
        if (crc == pHeader->getCrc()) {
            this->asyncReadBody(code, size);
        } else {
            this->asyncReadBody(code, size);
            std::cout << "crc error!!\n";
        }
    } else if (boost::asio::error::would_block) {
        std::cout << "would_block!\n";
        this->onClosed();
    } else {
        std::cout << "handle_read err!\n";
        this->onClosed();
    }
}

void session::handleReadBody(const boost::system::error_code &error, size_t bytes_transferred, int code) {
    if (!error) {
        switch (code) {
            case LOGIN_REQ_CODE: {
                std::cout << "recv LOGIN_REQ_CODE\n";
                this->recvLoginReq((MsgBody *) this->data_);
                break;
            }
            case DOWNLOAD_REQ_START_CODE: {
                std::cout << "recv DOWNLOAD_REQ_CODE-START\n";
                this->recvDownloadStart((MsgBody *) this->data_);
                break;
            }
            case DOWNLOAD_REQ_DATA_CODE: {
                this->recvDownloadData((MsgBody *) this->data_);
                break;
            }
            case DOWNLOAD_REQ_STOP_CODE: {
                std::cout << "recv DOWNLOAD_REQ_CODE-STOP\n";
                this->socketClose();
                break;
            }
            default: {
                std::cout << "unknown code!!\n";
                this->asyncWrite((char *) packetManager::makeMsgPacket(0, ErrCode::UNKNOWN_CODE, ""),
                                 sizeof(MsgPacket));
                this->socketClose();
                break;
            }
        }
    } else if (boost::asio::error::would_block) {
        std::cout << "would_block!\n";
        this->onClosed();
    } else {
        std::cout << "handle_read err!\n";
        this->onClosed();
    }
}

void session::recvLoginReq(MsgBody *recvData) {
    MsgBody *pBody = (MsgBody *) recvData;
    //char* data = new char[pBody->getLength()];
    std::cout << pBody->getMsg() << "\n";
    std::cout << "send LOGIN_RES_CODE!\n";
    this->asyncWrite((char *) packetManager::makeMsgPacket(LOGIN_RES_CODE, ErrCode::NONE, ""), sizeof(MsgPacket));
}

void session::recvDownloadStart(MsgBody *recvData) {
    std::string file(this->filepath);
    file.append(recvData->getMsg());

    std::cout << file << "\n";
    this->sourceFile.open(file.c_str(), std::ifstream::in | std::ios::binary);
    if (this->sourceFile.is_open()) {
        std::cout << "file open!!\n";
        this->sourceFile.seekg(0, std::ios::end);
        int fsize = this->sourceFile.tellg();
        this->sourceFile.seekg(0, std::ios::beg);
        std::string msg = std::to_string(fsize);
        std::cout << msg << "\n";
        std::cout << "send DOWNLOAD_RES_CODE-START!\n";
        this->asyncWrite((char *) packetManager::makeMsgPacket(DOWNLOAD_RES_START_CODE, ErrCode::NONE, msg),
                         sizeof(MsgPacket));
    } else {
        std::cout << "file open fail!! [" << file << "]\n";
        this->asyncWrite((char *) packetManager::makeMsgPacket(DOWNLOAD_RES_START_CODE, ErrCode::FILE_OPEN_FAIL, ""),
                         sizeof(MsgPacket));
        this->socketClose();
    }
}

void session::recvDownloadData(MsgBody *recvData) {
    if (this->sourceFile.is_open()) {
        if (!this->sourceFile.eof()) {
            DataPacket *sendData = new DataPacket();
            sendData->header.setCode(DOWNLOAD_RES_DATA_CODE);
            sendData->header.setErrCode(ErrCode::NONE);
            sendData->header.setSize(sizeof(DataPacket) - sizeof(HeaderPacket));
            sendData->header.setCrc(sendData->header.getCode()^sendData->header.getErrCode()^sendData->header.getSize());

            this->sourceFile.read(sendData->body.data_, SIZE_32KB);
            if (this->sourceFile.gcount() <= 0) {
                this->asyncWrite(
                        (char *) packetManager::makeMsgPacket(DOWNLOAD_RES_STOP_CODE, ErrCode::FILE_READ_FAIL, ""),
                        sizeof(DataPacket));
                delete sendData;
                this->socketClose();
                return;
            }
            sendData->body.setLength(this->sourceFile.gcount());
            sendData->body.setPacketNum(this->packetNum);
            this->packetNum++;
            this->asyncWrite((char *) sendData, sizeof(DataPacket));
        } else {
            std::cout << "num : " << this->packetNum << "\n";
            std::cout << "파일 전송 완료!!\n";
            this->asyncWrite((char *) packetManager::makeMsgPacket(DOWNLOAD_RES_STOP_CODE, ErrCode::NONE, ""),
                             sizeof(MsgPacket));
        }
    } else {
        this->asyncWrite((char *) packetManager::makeMsgPacket(DOWNLOAD_RES_STOP_CODE, ErrCode::FILE_OPEN_FAIL, ""),
                         sizeof(MsgPacket));
        this->socketClose();
    }
}
void session::setFilePath(const std::string& path) {
    this->filepath = path;
}

void session::start() {
    std::string clientIp = this->socket().remote_endpoint().address().to_string();
    unsigned short clientPort = this->socket().remote_endpoint().port();
    std::cout << "connect session [" << clientIp << ":" << clientPort << "]\n";
    this->asyncReadHeader();
}

void session::asyncReadHeader() {
    boost::asio::async_read(this->socket_, boost::asio::buffer(this->data_, sizeof(HeaderPacket)),
                            boost::bind(&session::handleReadHeader, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
}

void session::asyncReadBody(int code, int size) {
    boost::asio::async_read(this->socket_, boost::asio::buffer(this->data_, size),
                            boost::bind(&session::handleReadBody, this,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred,
                                        code));
}

void session::asyncWrite(const char *dataByte, int dataLen) {
    boost::asio::async_write(socket_, boost::asio::buffer(dataByte, dataLen),
                             boost::bind(&session::handle_write, this,
                                         boost::asio::placeholders::error,
                                         dataByte));
}

void session::handle_write(const boost::system::error_code &error, const char *sendData) {
    if (sendData) {
        delete sendData;
    }
    if (!error) {
        this->asyncReadHeader();
    } else {
        std::cout << "handle_write err!\n";
        this->socketClose();
    }
}

void session::onClosed() {
    delete this;
}

void session::socketClose() {
    std::string clientIp = this->socket().remote_endpoint().address().to_string();
    unsigned short clientPort = this->socket().remote_endpoint().port();
    std::cout << "session close [" << clientIp << ":" << clientPort << "]\n";
    this->socket().close();
    delete this;
}