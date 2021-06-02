#include "../inc/packetManager.hpp"

MsgPacket* packetManager::makeMsgPacket(int code, int errCode, const std::string& msg) {
    MsgPacket* sendData = new MsgPacket();
    sendData->header.setCode(code);
    sendData->header.setErrCode(errCode);
    sendData->header.setSize(sizeof(MsgPacket) - sizeof(HeaderPacket));
    sendData->header.setCrc(sendData->header.getCode()^sendData->header.getErrCode()^sendData->header.getSize());

    sendData->body.setLength(msg.length());
    //strcpy(sendData->data, loginInfo.c_str());
    sendData->body.setMsg(msg.c_str(), msg.length());

    return sendData;
}

bool packetManager::dataCrcCheck(DataPacket* recvData) {
//    unsigned short crc = crcChecker::calculateCRC2((unsigned char *)recvData->data, recvData->getLength());
//    //std::cout << crc << " / " << recvData->getCrc() << " / " << recvData->getLength() << "\n";
//    if (recvData->getCrc() == crc) {
//        return true;
//    }
    return false;
}
