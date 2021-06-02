
#ifndef ASIO_ASYNC_PACKETMANAGER_HPP
#define ASIO_ASYNC_PACKETMANAGER_HPP

#include "global.hpp"

namespace packetManager {
    MsgPacket* makeMsgPacket(int code, int errCode, const std::string& msg);
    //bool msgCrcCheck(DataPacket* recvData);
    bool dataCrcCheck(DataPacket* recvData);
};


#endif //ASIO_ASYNC_PACKETMANAGER_HPP
