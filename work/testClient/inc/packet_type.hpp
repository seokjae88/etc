
#ifndef ASIO_ASYNC_PACKET_TYPE_HPP
#define ASIO_ASYNC_PACKET_TYPE_HPP

#define SIZE_32KB 32768
#define SIZE_DATA (32768-28)

#define LOGIN_REQ_CODE              101
#define DOWNLOAD_REQ_START_CODE     102
#define DOWNLOAD_REQ_DATA_CODE      103
#define DOWNLOAD_REQ_STOP_CODE      104

#define LOGIN_RES_CODE              201
#define DOWNLOAD_RES_START_CODE     202
#define DOWNLOAD_RES_DATA_CODE      203
#define DOWNLOAD_RES_STOP_CODE      204

enum ErrCode {
    NONE = 0,
    LOGIN_FAIL,
    FILE_OPEN_FAIL,
    FILE_READ_FAIL,
    PACKET_NUM_WRONG,
    PACKET_LENGTH_WRONG,
    UNKNOWN_TYPE,
    UNKNOWN_CODE,
    UNKNOWN_ERR,
};

class HeaderPacket {
private:
    int code_ = 0; // 101:req_login, 102:req_download, 201:res_login, 202:res_download, 999:error
    int size_ = 0; // data size (total - header)
    int errCode_ = 0; // error code
    int crc_ = 0;
public:
    HeaderPacket() {
    }
    void setCode(int code) {
        this->code_ = htonl(code);
    }
    int getCode() {
        return ntohl(this->code_);
    }
    void setSize(int size) {
        this->size_ = htonl(size);
    }
    int getSize() {
        return ntohl(this->size_);
    }
    void setErrCode(int err) {
        this->errCode_ = htonl(err);
    }
    int getErrCode() {
        return ntohl(this->errCode_);
    }
    void setCrc(int crc) {
        this->crc_ = htonl(crc);
    }
    int getCrc() {
        return ntohl(this->crc_);
    }
};

class MsgBody {
private:
    int num_ = 0;
    int length_ = 0;
    char msg_[1024];
public:
    MsgBody() {
        memset(this->msg_, 0x0, 1024);
    }
    void setLength(int length) {
        this->length_ = htonl(length);
    }
    int getLength() {
        return ntohl(this->length_);
    }
    void setPacketNum(int num) {
        this->num_ = htonl(num);
    }
    int getPacketNum() {
        return ntohl(this->num_);
    }
    void setMsg(const char *msg, int length) {
        memset(this->msg_, 0x0, 1024);
        memcpy(this->msg_, msg, length);
    }
    char *getMsg() {
        return this->msg_;
    }
};

class DataBody {
private:
    int num_ = 0;
    int length_ = 0;
public:
    DataBody() {
        memset(this->data_, 0x0, SIZE_32KB);
    }
    void setLength(int length) {
        this->length_ = htonl(length);
    }
    int getLength() {
        return ntohl(this->length_);
    }
    void setPacketNum(int num) {
        this->num_ = htonl(num);
    }
    int getPacketNum() {
        return ntohl(this->num_);
    }
    void setData(const char* data, int length) {
        memset(this->data_, 0x0, SIZE_32KB);
        memcpy(this->data_, data, length);
    }
    char* getData() {
        return this->data_;
    }
    char data_[SIZE_32KB];
};

typedef struct {
    HeaderPacket header;
    DataBody body;
} DataPacket;

typedef struct {
    HeaderPacket header;
    MsgBody body;
} MsgPacket;



#endif //ASIO_ASYNC_PACKET_TYPE_HPP
