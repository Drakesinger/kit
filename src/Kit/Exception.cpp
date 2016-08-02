#include "Kit/Exception.hpp"

kit::Exception::Exception(std::string msg, std::string file, std::string method, int line){
	this->m_what = msg;
	this->m_file = file;
	this->m_method = method;
	this->m_line = line;
}

kit::Exception::~Exception(){

}

std::string kit::Exception::what(){
	return this->m_what;
}

std::string kit::Exception::file(){
	return this->m_file;
}

std::string kit::Exception::method(){
	return this->m_method;
}

int kit::Exception::line(){
	return this->m_line;
}
