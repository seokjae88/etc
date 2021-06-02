cmake_minimum_required(VERSION 3.0)
project(asio_async)

find_package(Boost COMPONENTS REQUIRED system filesystem thread)

include_directories(${Boost_INCLUDE_DIR} server/inc/ client/inc/ common/inc/)
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_compile_options(-DASIO_STANDALONE)

add_executable(_client
		client/src/cliMain.cpp
		client/src/client.cpp
		common/src/packetManager.cpp
		common/src/crcChecker.cpp)

add_executable(_server
		server/src/session.cpp
		server/src/server.cpp
		server/src/svrMain.cpp
		common/src/packetManager.cpp
		common/src/crcChecker.cpp)

target_link_libraries(_client
		${Boost_LIBRARIES}
		)

target_link_libraries(_server
		${Boost_LIBRARIES}
		)