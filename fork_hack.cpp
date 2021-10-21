
#include <memory>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <queue>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdio>

#include "unistd.h"

#include <dlfcn.h>

#include <thread>
#include <mutex>
#include <filesystem> //cross-platform filesystem tools, since c++17
namespace fs = std::filesystem;

using namespace std;

namespace fork_hack{

fs::path fork_hack_dir;
//
size_t counter = 0;
std::mutex mtx;

std::string fileToString(const std::string& filename){
	std::ifstream in(filename);
	std::stringstream buff;
	buff << in.rdbuf();
	return(buff.str());
}
static void writeFile(std::string path, std::string content){
	std::ofstream ofs(path);
	ofs << content;
}

//doesn't seem to work
//int original_system(const char* command){
	//int(*func)(const char*);
	//func = (int(*)(const char*)) dlsym(RTLD_NEXT, "system");
	//return (*func)(command);
//}

void fork_hack_init(){
	pid_t fork_hack_pid = getpid();
	fork_hack_dir = "/tmp/fork_hack_" + to_string(fork_hack_pid);
	fs::create_directory(fork_hack_dir);
}

std::pair<int,std::string> system_return(std::string command, bool stringResultAsFile){
	if(fork_hack_dir.u8string()==""){
		std::cerr << "fork_hack system_return: did you forget to call fork_hack_init?" << std::endl;
		throw std::runtime_error("fork_hack system_return: did you forget to call fork_hack_init?");
	}
	//
	mtx.lock();
	size_t uniqueNum = counter++;
	mtx.unlock();
	//
	fs::path tmpFileCommandPath = fork_hack_dir / (to_string(uniqueNum) + "_in");
	fs::path tmpFileCommandOutPath = fork_hack_dir / (to_string(uniqueNum) + "_out");
	//
	writeFile(tmpFileCommandPath, command + "\n");
	while(true){
		if(fs::exists(tmpFileCommandOutPath)){
			string content = fileToString(tmpFileCommandOutPath);
			size_t firstNewline = content.find('\n');
			if(firstNewline != -1){
				string firstLine = content.substr(0,firstNewline);
				size_t spaceIdx = firstLine.find(' ');
				int returnCode = stoi(firstLine.substr(0,spaceIdx));
				size_t numChars = stoi(firstLine.substr(spaceIdx+1));
				if(content.length() == (firstNewline+1)+numChars){
					fs::remove(tmpFileCommandPath);
					if(stringResultAsFile){
						writeFile(tmpFileCommandOutPath, content.substr(firstNewline+1));
						return make_pair(returnCode, tmpFileCommandOutPath);
					} else {
						fs::remove(tmpFileCommandOutPath);
						return make_pair(returnCode, content.substr(firstNewline+1));
					}
				}
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
}

}


int system(const char* command){
	pair<int,string> result = fork_hack::system_return(string(command), false);
	return result.first;
}

FILE* popen(const char* command, const char* type){
	if(string(type) == "r"){
		pair<int,string> result = fork_hack::system_return(string(command), true);
		return fopen(result.second.c_str(), "r");
	} else {
		cerr << "fork_hack: popen: unexpected mode: " + string(type) << endl;
		throw std::runtime_error("fork_hack: popen: unexpected mode");
	}
}

