#pragma once

#include <iostream>
#include <fstream> 
#include <stdexcept>
#include <string>

#define __u16 uint16_t
#define __u32 uint32_t

#define swab16(x) ((__u16)(				\
	(((__u16)(x) & (__u16)0x00ffU) << 8) |			\
	(((__u16)(x) & (__u16)0xff00U) >> 8)))

#define swab32(x) ((__u32)(				\
	(((__u32)(x) & (__u32)0x000000ffUL) << 24) |		\
	(((__u32)(x) & (__u32)0x0000ff00UL) <<  8) |		\
	(((__u32)(x) & (__u32)0x00ff0000UL) >>  8) |		\
	(((__u32)(x) & (__u32)0xff000000UL) >> 24)))

/* BIG ENDIAN */
#pragma pack(push,1)
struct YmHeader {
	char fileId[4];
	char checkString[8];
	uint32_t frameNumber;
	uint32_t songAttributes;
	uint16_t digidrumSamples;
	uint32_t masterClock;
	uint16_t frameRate;
	uint32_t loopFrame;
	uint16_t additionalData;
};

struct YmFrame {
	uint8_t R[16];
};
#pragma pack(pop)

class YmStream
{
protected:
	std::ifstream is;
	YmHeader header;
	std::string songName;
	std::string authorName;
	std::string songComment;
	YmFrame* data;
	tm duration;
	void readHeader(void);
	void readInterleaved(void);
	void readSequential(void);
	void readEndMarker(void);
public:
	YmStream(const wchar_t* fileName);
	~YmStream();
	uint32_t getFrameCount();
	uint16_t getFrameRate();
	uint32_t getLoopFrame();
	tm* getDuration();
	std::string& getSongName();
	std::string& getAuthorName();
	std::string& getSongComment();
	std::string getVersion();
	std::string getDurationStr(const char* format = "%H:%M:%S");
	YmFrame getFrame(uint32_t frameNumber);
};

