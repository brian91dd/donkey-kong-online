#include "fileManager.hpp"

FileManager::FileManager(string filePath) {
	this->filePath = filePath;
	//c_str() convierte la cadena al tipo char*
	this->file.open(filePath.c_str(),ios::out | ios::trunc);
}

void FileManager::saveMessage(string message) {
	this->file << message << "\n";
}
void FileManager::close() {
	this->file.close();
	cout << "archivo cerrado totalmente"<<endl;
}