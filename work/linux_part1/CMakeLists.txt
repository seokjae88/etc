cmake_minimum_required(VERSION 3.0)
project(asio_async)

find_package(Boost COMPONENTS REQUIRED system filesystem thread)

include_directories(${Boost_INCLUDE_DIR} inc/)
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

add_compile_options(-DASIO_STANDALONE)

add_executable(_client
		src/cliMain.cpp
		)

add_executable(_server
		src/svrMain.cpp
		)

target_link_libraries(_client
		${Boost_LIBRARIES}
		)

target_link_libraries(_server
		${Boost_LIBRARIES}
		)