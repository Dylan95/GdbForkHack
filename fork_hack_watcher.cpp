
#include <memory>
#include <iostream>
#include <chrono>
#include <vector>
#include <algorithm>
#include <queue>
#include <string>
#include <fstream>
#include <sstream>
#include <list>

#include <thread>
#include <mutex>

#include "unistd.h"
#include <sys/stat.h>

#include <filesystem> //cross-platform filesystem tools, since c++17
namespace fs = std::filesystem;

using namespace std;

string fileToString(const std::string& filename){
	std::ifstream in(filename);
	std::stringstream buff;
	buff << in.rdbuf();
	return(buff.str());
}
static void writeFile(std::string path, std::string content){
	std::ofstream ofs(path);
	ofs << content;
}

int main(int argc, char** argv){
	vector<string> args;
	for(int i = 0; i < argc; i++){
		args.push_back(argv[i]);
	}
	//
	if(args.size() != 2){
		cerr << "expected 1 arg" << endl;
		return 1;
	}
	//
	string watchPIDStr = args[1];
	cout << "watch pid: " << watchPIDStr << endl;
	//
	fs::path fork_hack_dir = "/tmp/fork_hack_" + watchPIDStr;
	size_t nextID = 0;
	fs::path tmpFileCommandPath = fork_hack_dir / (to_string(nextID) + "_in");
	fs::path tmpFileCommandOutPath = fork_hack_dir / (to_string(nextID) + "_out");
	fs::path tmpFileCommandOutTmpPath = fork_hack_dir / (to_string(nextID) + "_out.tmp");
	//
	list<unique_ptr<thread> > commandThreads;
	vector<unique_ptr<thread> > commandThreadsDone;
	std::mutex commandThreads_mtx;
	cout << "awaiting command file " << tmpFileCommandPath << endl;
	while(true){
		{
			struct stat sts;
			string procPath = "/proc/" + watchPIDStr;
			if((stat(procPath.c_str(), &sts) == -1) && (errno == ENOENT)) {
				//watched process is dead
				break;
			}
		}
		//
		while(fs::exists(tmpFileCommandPath)){
			size_t id = nextID;
			//
			string command = fileToString(tmpFileCommandPath);
			if(command.back() == '\n'){
				command.pop_back();
				command += " >> " + tmpFileCommandOutTmpPath.u8string();
				//
				commandThreads_mtx.lock();
				auto it = commandThreads.insert(commandThreads.end(), unique_ptr<thread>());
				commandThreads_mtx.unlock();
				it->reset(new std::thread([it,command,tmpFileCommandPath,tmpFileCommandOutPath,tmpFileCommandOutTmpPath,&commandThreads,&commandThreadsDone,&commandThreads_mtx](){
					int result = system(command.c_str());
					string output = fileToString(tmpFileCommandOutTmpPath);
					writeFile(tmpFileCommandOutPath, to_string(result) + " " + to_string(output.length()) + "\n" + output);
					fs::remove(tmpFileCommandOutTmpPath);
					//
					//threads cleanup - remove from list
					commandThreads_mtx.lock();
					commandThreadsDone.emplace_back(it->release());
					commandThreads.erase(it);
					commandThreads_mtx.unlock();
				}));
			}
			//
			//threads cleanup - remove
			commandThreads_mtx.lock();
			for(unique_ptr<thread>& th : commandThreadsDone){
				th->join();
			}
			commandThreadsDone.clear();
			commandThreads_mtx.unlock();
			//
			nextID++;
			tmpFileCommandPath = fork_hack_dir / (to_string(nextID) + "_in");
			tmpFileCommandOutPath = fork_hack_dir / (to_string(nextID) + "_out");
			tmpFileCommandOutTmpPath = fork_hack_dir / (to_string(nextID) + "_out.tmp");
			cout << "awaiting command file " << tmpFileCommandPath << endl;
		}
		//
		std::this_thread::sleep_for(std::chrono::milliseconds(30));
	}
	//
	//cleanup.  kill all processes that may be awaiting completion, nessecary for an immediate exit.  Remove all temp files and temp directory
	commandThreads.clear();
	commandThreadsDone.clear();
	for(fs::path tmpFile : fs::directory_iterator(fork_hack_dir)){
		fs::remove(tmpFile);
	}
	fs::remove(fork_hack_dir);
	return 0;
}

