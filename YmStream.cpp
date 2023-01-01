#include "YmStream.h"

YmStream::YmStream(const wchar_t* fileName) : 
	header({}) 
{
	is.open(fileName, std::ios::binary);
	if (!is.is_open()) {
		throw std::invalid_argument("Cannot open file");
	}
	readHeader();
	data = (YmFrame*) calloc(header.frameNumber, sizeof(YmFrame));
	if (!data) {
		throw std::runtime_error("Memory allocation failed");
	}
	if (header.songAttributes & 0x01) {
		readInterleaved();
	}
	else {
		readSequential();
	}
	readEndMarker();
	is.close();
}

uint32_t YmStream::getFrameCount() 
{
	return header.frameNumber;
}

uint16_t YmStream::getFrameRate() 
{
	return header.frameRate;
}

uint32_t YmStream::getLoopFrame() 
{
	return header.loopFrame;
}

YmFrame YmStream::getFrame(uint32_t frameNumber) 
{
	if (frameNumber >= header.frameNumber)
		return { 0 };
	return data[frameNumber];
}

std::string YmStream::getDurationStr(const char* format)
{
	char timeStr[32];
	strftime(timeStr, sizeof(timeStr), format, &duration);
	return std::string(timeStr);
}

std::string YmStream::getVersion()
{
	return std::string(header.fileId, 4);
}

std::string& YmStream::getSongName() 
{
	return songName;
}

tm* YmStream::getDuration()
{
	return &duration;
}

std::string& YmStream::getAuthorName() 
{
	return authorName;
}

std::string& YmStream::getSongComment() 
{
	return songComment;
}

void YmStream::readEndMarker(void) 
{
	char endMarker[4];
	is.read(endMarker, 4);
	if (!is.good() || strncmp(endMarker, "End!", 4)) {
		throw std::runtime_error("Invalid file ending");
	}
	is.get();
	if (!is.eof()) {
		throw std::runtime_error("Unexpected data at the end of the file");
	}
}

void YmStream::readInterleaved(void)
{
	for (uint8_t r = 0; r < sizeof(YmFrame); ++r) {
		uint8_t* frames = (uint8_t * )calloc(header.frameNumber, sizeof(uint8_t));
		if (frames) {
			is.read((char*)frames, header.frameNumber);
			if (!is.good()) {
				free(frames);
				throw std::runtime_error("Could not read frame data");
			}
			for (uint32_t f = 0; f < header.frameNumber; f++) {
				data[f].R[r] = frames[f];
			}
			free(frames);
		}
		else {
			throw std::runtime_error("Memory allocation failed");
		}
	}
}

void YmStream::readSequential(void)
{
	for (uint32_t f = 0; f < header.frameNumber; ++f) {
		is.read((char*)&data[f], sizeof(YmFrame));
		if (!is.good()) {
			throw std::invalid_argument("Could not read frame data");
		}
	}
}

void YmStream::readHeader(void) 
{
	is.read((char*)&header, sizeof(header));
	if (is.gcount() != sizeof(header)) {
		throw std::runtime_error("Error reading file header");
	}
	header.frameNumber = swab32(header.frameNumber);
	header.songAttributes = swab32(header.songAttributes);
	header.digidrumSamples = swab16(header.digidrumSamples);
	header.masterClock = swab32(header.masterClock);
	header.frameRate = swab16(header.frameRate);
	header.loopFrame = swab32(header.loopFrame);
	header.additionalData = swab16(header.additionalData);
	if ((strncmp(header.fileId, "YM6!", 4) != 0) && (strncmp(header.fileId, "YM5!", 4) != 0)) {
		throw std::invalid_argument("File ID does not match");
	}
	if (strncmp(header.checkString, "LeOnArD!", 8) != 0) {
		throw std::invalid_argument("Check string does not match");
	}
	is.seekg(header.additionalData, std::ios_base::cur);
	if (!is.good()) {
		throw std::invalid_argument("Could not skip additional data");
	}
	for (int i = 0; i < header.digidrumSamples; i++) {
		uint32_t digidrumSampleSize;
		is.read((char*) &digidrumSampleSize, sizeof(digidrumSampleSize));
		if (!is.good()) {
			throw std::invalid_argument("Could not read digidrum sample size");
		}
		digidrumSampleSize = swab32(digidrumSampleSize);
		is.seekg(digidrumSampleSize, std::ios_base::cur);
		if (!is.good()) {
			throw std::invalid_argument("Could not skip digidrum sample data");
		}
	}
	std::getline(is, songName, '\0');
	std::getline(is, authorName, '\0');
	std::getline(is, songComment, '\0');
	time_t dsec = (getFrameCount() / getFrameRate());
	gmtime_s(&duration, &dsec);
}

YmStream::~YmStream() 
{
	if (data) {
		free(data);
		data = NULL;
	}
}