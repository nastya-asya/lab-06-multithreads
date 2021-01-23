// Copyright 2020 Anastasiya Smirnova nastya.asya08@yandex.ru

#include <picosha2.h>

#include <hash.hpp>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <ctime>
#include <thread>

std::atomic<bool> continueProcess = true;
std::atomic<bool> addToJson = false;

void stopProcess(int param) {
    if (param == SIGINT) {
        continueProcess = false;
    }
}

void hashGenerator(JsonFiler& j) {
    while (continueProcess) {
        std::string genstring = std::to_string(std::rand());
        std::string hash = picosha2::hash256_hex_string(genstring);
        std::time_t timestamp(std::time(nullptr));
        std::string lastChar = hash.substr(hash.size() - NumZeroes);

        if (lastChar == Ending) {
            BOOST_LOG_TRIVIAL(info) << "Necessary ending found in hash |" << hash
                                    << "| generarted from string |" << genstring << "|";
            if (addToJson) {
                j.NewElement(genstring, hash, timestamp);
            }
        } else {
            BOOST_LOG_TRIVIAL(trace)
                    << "Hash |" << hash << "| generated from string|" << genstring << "|";
        }
    }
}
void StartProcess(const int& argc, char **argv){
    unsigned int numberOfThreads;
    std::string json_path;
    std::srand(time(nullptr));
    switch (argc) {
        case 1:
            numberOfThreads = std::thread::hardware_concurrency();
            break;
        case 2:
            numberOfThreads = std::atoi(argv[1]);
            if (numberOfThreads == 0 ||
                numberOfThreads > std::thread::hardware_concurrency()) {
                throw std::out_of_range(" Invalid number of threads!!!");
            }
            break;
        case 3:
            numberOfThreads = std::atoi(argv[1]);
            if (numberOfThreads == 0 ||
                numberOfThreads > std::thread::hardware_concurrency()) {
                throw std::out_of_range(" Invalid number of threads!!!");
            }
            json_path = argv[2];
            addToJson = true;
            break;
        default:
            throw std::out_of_range("Invalid number of arguments!!!");
    }
    SetUpLogging();
    boost::log::add_common_attributes();
    std::vector<std::thread> threads;
    JsonFiler json_obj;
    threads.reserve(numberOfThreads);
    std::signal(SIGINT, stopProcess);

    for (size_t i = 0; i < numberOfThreads; i++) {
        threads.emplace_back(hashGenerator, std::ref(json_obj));
    }
    for (auto& thread : threads) {
        thread.join();
    }
    if (addToJson) {
        std::ofstream fout{json_path};
        fout << json_obj;
    }
}

