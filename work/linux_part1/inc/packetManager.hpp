
#ifndef ASIO_ASYNC_PACKETMANAGER_HPP
#define ASIO_ASYNC_PACKETMANAGER_HPP

#include "global.hpp"

namespace packetManager {
    MsgPacket* makeMsgPacket(int code, int errCode, const std::string& msg) {
        MsgPacket* sendData = new MsgPacket();
        sendData->header.setCode(code);
        sendData->header.setErrCode(errCode);
        sendData->header.setSize(sizeof(MsgPacket) - sizeof(HeaderPacket));
        sendData->header.setCrc(sendData->header.getCode()^sendData->header.getErrCode()^sendData->header.getSize());

        sendData->body.setLength(msg.length());
        sendData->body.setMsg(msg.c_str(), msg.length());

        return sendData;
    }
}

#endif //ASIO_ASYNC_PACKETMANAGER_HPP
