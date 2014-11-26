#include <iostream>
#include <fstream>
#ifndef _H_FILE_MANAGER
	#define _H_FILE_MANAGER
	using namespace std;
	class FileManager {
		private:
			string filePath;
			ofstream file;
		public:
			FileManager(string);
			void saveMessage(string);
			void close();
	};
#endif