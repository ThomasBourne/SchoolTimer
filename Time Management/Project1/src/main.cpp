#define _CRT_SECURE_NO_WARNINGS

#define TIME_NULL 0LL

#define MONDAY 1

#define BEEN_RESET "reset"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
#include <chrono>
#include <thread>
#include <atomic>
#include <string>
#include <Windows.h>

#include "json.hpp"

json newTimeTable = {
	{ "DT", 7200 },
	{ "comp sci", 7200 },
	{ "econ", 7200 },
	{ "geog", 7200 },
	{ BEEN_RESET, true }
};

void Write(std::string contents, std::string recordsPath) {
	std::ofstream exit(recordsPath);
	exit << contents;
	exit.close();
}

void DecrementTime(std::atomic<int>* timeRemaining, std::atomic<bool>* ended, json* times, const std::string recordsPath) {
	std::chrono::steady_clock::time_point startTime = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point currTime;
	while (timeRemaining->load() >= 0 && !ended->load()) {
		if (std::chrono::duration_cast<std::chrono::milliseconds>(currTime - startTime).count() >= 1000) {
			*(timeRemaining) -= 1;
			startTime = std::chrono::steady_clock::now();
		}
		currTime = std::chrono::steady_clock::now();
	}
	std::cout << "\a" << std::endl << "\aTimer Ended!" << std::endl;
	std::cout << ">>> ";
	Write(times->dump(), recordsPath);
}

void GetInput(std::atomic<int>* timeRemaining, std::atomic<bool>* ended, json* times, const std::string recordsPath, const std::string option) {
	std::string input;
	while (timeRemaining->load() >= 0 && !ended->load()) {
		std::cout << "Type 'quit'/'exit' to finish (current time: " << timeRemaining->load() << "s):" << std::endl << ">>> ";
		std::cin >> input;
		if (input == "quit" || input == "exit" || input == "end") {
			*(ended) = false;
			times->operator[](option) = timeRemaining->load(); //comment
			Write(times->dump(), recordsPath);
			std::cout << times->dump();
			exit(0);
		}
	}
	Write(times->dump(), recordsPath);
}

int main() {
	std::atomic<bool> ended{ false };
	char* path = new char[_MAX_PATH];
	GetModuleFileNameA(NULL, path, _MAX_PATH);
	std::string recordsPath{ path };
	delete[] path;
	recordsPath = recordsPath.substr(0, recordsPath.find_last_of('\\'));
	recordsPath += "\\records.rec";

	time_t t = time(TIME_NULL);
	tm* timePtr = localtime(&t);

	std::ifstream oldData(recordsPath);
	json times;
	std::string contents;
	oldData >> contents;
	if (contents[0] != '{') {
		times = newTimeTable;
	}
	else {
		oldData.close();
		oldData.open(recordsPath);
		oldData >> times;
	}
	if (timePtr->tm_wday == MONDAY && !times[BEEN_RESET].get<bool>()) {
		times = newTimeTable;
		times[BEEN_RESET] = true;
	}
	else if (timePtr->tm_wday != MONDAY && times[BEEN_RESET].get<bool>()) {
		times[BEEN_RESET] = false;
	}
	oldData.close();
	Write(times.dump(), recordsPath);
	std::string option;
	do {
		std::cout << std::setw(2) << times << std::endl;
		std::cout << "Enter a timer to start ('exit'/'quit' to quit and 'reset' to reset)" << std::endl << ">>> ";
		std::cin >> option;
		if (option == "reset") {
			times = newTimeTable;
			Write(newTimeTable.dump(), recordsPath);
			option = "";
		}
		else if (option == "exit" || option ==  "quit" || option == "end") {
			Write(times.dump(), recordsPath);
			exit(0);
		}
	} while (!times.contains(option) && option != BEEN_RESET);
	std::atomic<int> timeRemaining = times[option].get<int>();
	std::thread timeThread(DecrementTime, &timeRemaining, &ended, &times, recordsPath);
	std::thread inputThread(GetInput, &timeRemaining, &ended, &times, recordsPath, option);
	timeThread.join();
	inputThread.join();
}