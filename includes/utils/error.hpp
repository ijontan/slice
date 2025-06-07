
#include <exception>
#include <sstream>
#include <string>

class Error : public std::exception
{
public:
	Error(std::string info, std::string extraInfo)
	{
		static const std::string error("Error: ");
		std::stringstream ss;
		ss << error << info << extraInfo;
		ss >> _error;
	};
	~Error() throw() {};
	const char *what() const throw()
	{
		return _error.c_str();
	}

private:
	std::string _error;
};
