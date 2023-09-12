#include "json_reader.h"

#include <fstream>
#include <iostream>
#include <string_view>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    TransportCatalogue catalogue;
    MapRenderer map_renderer(catalogue);
    TransportRouter transport_router(catalogue);
    JsonReader json_reader(std::cin, std::cout, catalogue, map_renderer, transport_router);

    if (mode == "make_base"sv) {
        json_reader.make_base();
    } else if (mode == "process_requests"sv) {
        json_reader.process_requests();
    } else {
        PrintUsage();
        return 1;
    }
}

/*========FOR TESTING PURPOSES ONLY========*/
//int main() {
    //{//make_base
    //    std::ifstream makebase_in(R"(C:\Users\qwert\source\repos\CMake\05___TransportCatalogue\IO\makebase1.json)");
    //    if (!makebase_in) {
    //        throw std::invalid_argument("Failed to open file");
    //    }

    //    TransportCatalogue catalogue;
    //    MapRenderer map_renderer(catalogue);//создал, чтобы не менять в json_reader ссылки на указатели
    //    TransportRouter transport_router(catalogue);//аналогично
    //    JsonReader json_reader(makebase_in, std::cout, catalogue, map_renderer, transport_router);
    //    json_reader.make_base();
    //}
    //{//process_requests
    //    std::ifstream process_requests_in(R"(C:\Users\qwert\source\repos\CMake\05___TransportCatalogue\IO\process_requests1.json)");
    //    if (!process_requests_in) {
    //        throw std::invalid_argument("Failed to open file");
    //    }
    //    std::ofstream file_out(R"(C:\Users\qwert\source\repos\CMake\05___TransportCatalogue\IO\output.txt)");

    //    TransportCatalogue catalogue;
    //    MapRenderer map_renderer(catalogue);
    //    TransportRouter transport_router(catalogue);
    //    JsonReader json_reader(process_requests_in, file_out, catalogue, map_renderer, transport_router);
    //    json_reader.process_requests();
    //}
//}