#include "../inc/global.hpp"
#include "../inc/server.hpp"

int main() {
    try {
        std::cout << BOOST_LIB_VERSION << '\n';
        boost::property_tree::ptree pt;
        std::string iniFile = "../conf/test.ini";
        boost::property_tree::ini_parser::read_ini(iniFile, pt);
        //boost::property_tree::ini_parser::read_ini( "../conf/test.ini", pt );
        int port = pt.get<int>("server.port");
        std::string filepath = pt.get<std::string>("server.filepath");

        std::cout << "[" << iniFile << "] read!!\n";
        std::cout << "port [" << port << "]\n";
        std::cout << "filepath [" << filepath << "]\n";

        std::cout << "server start (port:" << port << ")\n";
        boost::asio::io_context io_service;
        server svr(io_service, port, filepath);
        //asio 통신을 시작한다.
        size_t cpuc = (boost::thread::hardware_concurrency() * 2) + 2;
        boost::thread_group tg;
        for (int i=0; i < (int)cpuc; ++i) {
            tg.create_thread(boost::bind(&boost::asio::io_context::run, &io_service));
        }

            //tg.create_thread([&]{ io_service.run(); });

        tg.join_all();
        //io_service.run();
    }
    catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}