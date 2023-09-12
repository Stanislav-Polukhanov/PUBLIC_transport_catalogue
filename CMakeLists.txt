cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)

set(OTHER   geo.h 
            geo.cpp 
            ranges.h)
 
set(TRANSPORT_CATALOGUE transport_catalogue.h 
                        transport_catalogue.cpp 
                        domain.h 
                        domain.cpp 
                        transport_catalogue.proto)
                      
set(ROUTER graph.h 
           router.h        
           transport_router.h 
           transport_router.cpp)
                              
set(JSON json.h 
         json.cpp 
         json_builder.h 
         json_builder.cpp 
         json_reader.h 
         json_reader.cpp)
               
set(SVG svg.h 
        svg.cpp)
        
set(MAP_RENDERER map_renderer.h
                 map_renderer.cpp)
              
set(SERIALIZATION serialization.h 
                  serialization.cpp)

add_executable(transport_catalogue main.cpp 
                                   ${PROTO_SRCS} 
                                   ${PROTO_HDRS} 
                                   ${OTHER}
                                   ${TRANSPORT_CATALOGUE}
                                   ${ROUTER} 
                                   ${JSON}
                                   ${SVG} 
                                   ${MAP_RENDERER} 
                                   ${SERIALIZATION})

target_include_directories(transport_catalogue  PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue  PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue  "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)