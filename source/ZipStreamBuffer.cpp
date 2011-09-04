#include "ZipStreamBuffer.hpp"

#define BUFFER_SIZE 0x100

ZipStreamBuffer::ZipStreamBuffer(const std::string& zipPath, const std::string& filename)
{
	file = unzOpen(zipPath.c_str());
	if (!file)
		throw 0;

	int n = unzLocateFile(file, filename.c_str(), 1);
	if (n != UNZ_OK)
		throw 0;

	char filePath[260];
	unz_file_info fileInfo;
	n = unzGetCurrentFileInfo(file, &fileInfo, filePath, sizeof(filePath), NULL, 0, NULL, 0);
	if (n != UNZ_OK)
		throw 0;

	n = unzOpenCurrentFile(file);
	if (n != UNZ_OK)
		throw 0;

	buffer = new char[BUFFER_SIZE];
}

ZipStreamBuffer::~ZipStreamBuffer()
{
	unzCloseCurrentFile(file);
	unzClose(file);
	delete[] buffer;
}

int ZipStreamBuffer::underflow()
{
	int size = unzReadCurrentFile(file, buffer, BUFFER_SIZE);
	if (size <= 0)
		return EOF;
	setg(buffer, buffer, buffer + size);
	return size;
}

std::streampos ZipStreamBuffer::seekpos(std::streampos sp, std::ios_base::openmode which)
{
	throw 0;
}

std::streampos ZipStreamBuffer::sekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
	throw 0;
}
