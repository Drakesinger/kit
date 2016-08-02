#ifndef KIT_EXCEPTION_HEADER
#define KIT_EXCEPTION_HEADER

#include "Kit/Export.hpp"

#include <string>
#include <iostream>

namespace kit{

	class KITAPI Exception{
		public:
			Exception(std::string msg, std::string file, std::string method, int line);
			~Exception();

			std::string what();
			std::string file();
			std::string method();
			int line();

		private:
			std::string m_what;
			std::string m_file;
			std::string m_method;
			int					m_line;


	};

}


#ifdef _WIN32
	#define KIT_ERR(x) std::cout << "Error: " << __FILE__ << " at " << __LINE__ << " in " << __FUNCTION__ << ": " << x << std::endl;
	#define KIT_THROW(x)	KIT_ERR(x) \
		throw(kit::Exception(x, __FILE__, __FUNCTION__, __LINE__))
#elif __unix__
	#define KIT_ERR(x) std::cout << "Error: " << __FILE__ << " at " << __LINE__ << " in " << __PRETTY_FUNCTION__ << ": " << x << std::endl;
	#define KIT_THROW(x)	KIT_ERR(x) \
		throw(kit::Exception(x, __FILE__, __PRETTY_FUNCTION__, __LINE__))
#endif


#define KIT_ASSERT(x) if( ((x) == 0) ){ KIT_THROW("Assertion failed: " #x); }

#endif // KIT_EXCEPTION_HEADER
