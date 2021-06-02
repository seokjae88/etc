#include "../inc/global.hpp"
#include "../inc/client.hpp"

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
    try
    {
        boost::property_tree::ptree pt;
        std::string iniFile = "../conf/test.ini";
        boost::property_tree::ini_parser::read_ini(iniFile, pt);
        //boost::property_tree::ini_parser::read_ini( "../conf/test.ini", pt );
        std::string svrIp = pt.get<std::string>("client.server_ip");
        std::string svrPort = pt.get<std::string>("client.server_port");
        std::string filename = pt.get<std::string>("client.filename");
        std::string outpath = pt.get<std::string>("client.outpath");

        std::cout << "[" << iniFile << "] read!!\n";
        std::cout << "server_ip [" << svrIp << "]\n";
        std::cout << "server_port [" << svrPort << "]\n";
        std::cout << "filename [" << filename << "]\n";
        std::cout << "outpath [" << outpath << "]\n";

        std::cout << "server start (ip:" << svrIp << ", port:" << svrPort << ")\n";

        boost::asio::io_context io_context;
        //boost::asio:io_context io_context;
        client c(io_context, svrIp, svrPort, filename);
        c.setOutPath(outpath);
        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}