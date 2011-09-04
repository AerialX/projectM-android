#include <ios>
#include <streambuf>
#include <string>
#include <unzip.h>

class ZipStreamBuffer : public std::streambuf {
protected:
	char* buffer;
	unzFile file;

public:
	ZipStreamBuffer(const std::string& zipPath, const std::string& filename);
	~ZipStreamBuffer();

	int underflow();

	std::streampos seekpos(std::streampos sp, std::ios_base::openmode which);
	std::streampos sekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which);
};
