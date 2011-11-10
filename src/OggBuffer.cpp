#include "OggBuffer.h"
#include "Endianness.h"
#include <iostream>
using std::cout;
using std::endl;

OggBuffer::OggBuffer() 
:buffer(NULL)
,size(0)
,published(0)
,consumed(0)
,min_chunk_size(5)
{
	setup(); // should we do this? or its up to the user (?)
}

OggBuffer::~OggBuffer() {
	cleanup();
}


bool OggBuffer::loadFromFile(string path) {
	ifstream ifs(path.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
	if(!ifs.is_open()) {
		printf("OggBuffer error: cannot read file\n");
		return false;
	}
	// get size and go back to start.
	uint32_t file_size = ifs.tellg();
 	ifs.seekg(0, std::ios::beg);
	
	// read bytes into buffer.
 	char* tmp_buffer = new char [file_size];
	ifs.read (tmp_buffer, file_size);
 	ifs.close();
	
	// and copy the buffer.
	storeBytes(tmp_buffer, file_size);
	delete[] tmp_buffer;

	return true;
}

// http://www.cplusplus.com/reference/iostream/ofstream/ofstream/
bool OggBuffer::saveToFile(string path) {
	ofstream ofs(path.c_str(), std::ios::out | std::ios::binary);
	if(!ofs.is_open()) {
		return false;
	}
	
	//ofs.open(filename, ios::out | ios::binary);
	ofs.write((char*)buffer, published);
	if(!ofs.good()) {
		ofs.close();
		return false;
	}
	ofs.close();
	return true;
}


void OggBuffer::setup() {
	setup(min_chunk_size);	
}

void OggBuffer::setup(uint32_t expectedSize) {
	if( (buffer != NULL)
		|| (size != 0)
		|| (published != 0)
		|| (consumed != 0)
	)
	{
		printf("OggBuffer error: buffer has been setup already\n");
		return;
	}
	
	ensureSize(expectedSize);
	//memset(buffer, 0, expectedSize);
}

bool OggBuffer::ensureSize(uint32_t expectedSize) {
	
	// 1. we have enough space
	if(published + expectedSize <= size) {
		return true;
	}
	
	// 2. not enough space, try to move it.
	moveData();
	
	// 3. check if we have enough space after moving..
	if(published + expectedSize <= size) {
		return true;
	}
	
	// 4. not enough space. 
	// first allocate at least 1/3 of what we have and no less then min_chunk_size
	if( (published + expectedSize - size) < (size / 3)) {
		expectedSize = size + size / 3 - published;
	}
	if(expectedSize < min_chunk_size) {
		expectedSize = min_chunk_size;
	}
	
	// 5. create a new buffer.
	uint8_t* tmp_buffer = new uint8_t[published + expectedSize];
	
	// 6. copy exising data to tmp buffer
	if(buffer != NULL) {
		memcpy(tmp_buffer, buffer, published);
		delete[] buffer;
	}
	buffer = tmp_buffer;
	
	// 7. update our size.
	size = published + expectedSize;
	return true;
}

uint8_t* OggBuffer::getPtr() {
	return buffer;
}

uint8_t* OggBuffer::getStorePtr() {
	return buffer+published;
}

uint8_t* OggBuffer::getConsumePtr() {
	return buffer+consumed;
}

bool OggBuffer::moveData() {
	if(published - consumed <= consumed) {
		memcpy(buffer, buffer+consumed, published - consumed);
		published = published - consumed;
		consumed = 0;
	}
	return true;
}

void OggBuffer::cleanup() {
	if(buffer != NULL) {
		//printf("* need to free memory in OggBuffer \n");
		//delete[] buffer;
		buffer = NULL;
	}	
	size = 0;
	consumed = 0;
	published = 0;
}

void OggBuffer::setMinChunkSize(uint32_t minSize) {
	//assert(minSize > 0 && minSize < 16 * 1024 * 1024);
	min_chunk_size = minSize;
}

uint32_t OggBuffer::getMinChunkSize() {
	return min_chunk_size;
}

void OggBuffer::storeByte(uint8_t byte) {
	ensureSize(1);
	buffer[published] = byte;
	published++;
}

bool OggBuffer::storeBytes(const uint8_t* someData, const uint32_t numBytes) {
	//printf("storeBytes: %d\n", numBytes);
	if(!ensureSize(numBytes)) {
		return false;
	}
	memcpy(buffer+published, someData, numBytes);
	published += numBytes;
	return true;
}

bool OggBuffer::storeBytes(const char* someData, const uint32_t numBytes) {
	if(!ensureSize(numBytes)) {
		return false;
	}
	memcpy(buffer+published, someData, numBytes);
	published += numBytes;
	return true;
}


void OggBuffer::storeRepeat(uint8_t byte, uint32_t numBytes) {
	ensureSize(numBytes);
	memset(buffer + published, byte, numBytes);
	published += numBytes;
}

void OggBuffer::storeUI8(uint8_t byte) {
	storeByte(byte);
}

void OggBuffer::storeUI16(uint16_t data) {
	ensureSize(16);
	memcpy(buffer+published, &data, 2);
	published += 2;
}

void OggBuffer::storeUI32(uint32_t data) {
	ensureSize(32);
	memcpy(buffer+published, &data, 4);
	published += 4;
}

void OggBuffer::storeBigEndianUI16(uint16_t data) {
	ensureSize(16);
	data = ToBE16(data);
	memcpy(buffer+published, &data, 2);
	published += 2;
}

void OggBuffer::storeBigEndianUI32(uint32_t data) {
	ensureSize(4);
	data = ToBE32(data);
	memcpy(buffer+published, &data, 4);
	published += 4;
}

void OggBuffer::storeBigEndianUI64(uint64_t data) {
	ensureSize(8);
	data = ToBE64(data);
	memcpy(buffer+published, &data, 8);
	published += 8;
}

void OggBuffer::storeBigEndianDouble(double data) {
	ensureSize(8);
	uint64_t val = 0;
	memcpy(&val, &data, 8);
	val = ToBE64(val);
	memcpy(buffer+published, &val, 8);
	published += 8;
}


void OggBuffer::storeString(string data) {
	uint32_t len = (uint32_t)data.length();
	ensureSize(len);
	memcpy(buffer+published, (uint8_t *)data.c_str(), len);
	published += len;
}

// stores a uint16_t (big endian) + string
void OggBuffer::storeStringWithSize(string data) {
	storeBigEndianUI16(data.size());
	storeString(data);
}

// copy data from another buffer.
void OggBuffer::storeBuffer(OggBuffer& other) {
	storeBuffer(other, other.getNumBytesStored());	
}

int OggBuffer::storeBuffer(OggBuffer& other, uint32_t numBytes) {
	/*
	cout << "Store from other buffer. Bytes:" << numBytes << endl;
	cout << "this.published: " << published << endl;
	cout << "this.consumed: " << consumed << endl;
	cout << "other.published: " << other.published << endl;
	cout << "other.consumed: " << other.consumed << endl;
	*/
	
	// check if we can read this many bytes from other buffer.
	numBytes = other.getMostNumberOfBytesWeCanConsume(numBytes);
	if(numBytes == 0) {
		return 0;
	}
	ensureSize(numBytes);
	memcpy(buffer+published, other.buffer+other.consumed, numBytes);
	published += numBytes;
//	printf("consumed after copying from other buffer: %d\n", consumed);
	other.published += numBytes;
	return numBytes;
}

uint32_t OggBuffer::getMostNumberOfBytesWeCanConsume(uint32_t tryToRead) {
//	printf("@ published: %d consumed:%d trying to read: %d\n", published, consumed, tryToRead);
	int space = published - consumed;
	if(space <= 0) {
//		printf("@ space is zero %d?\n", space);
//		printf("@--------------\n");
		return 0;
	}

//		printf("@--------------\n");	
	if(tryToRead > space) {
		tryToRead = space;
	}
//	printf("@ tell we can read all!: %d\n", tryToRead);
//	printf("@--------------\n");
	return tryToRead;
}


bool OggBuffer::ignore(uint32_t numBytes) {
	consumed += numBytes;
	recycle();	
	return true;
}
bool OggBuffer::reuse(uint32_t numBytes) {
	consumed -= numBytes;
	return true;
}

void OggBuffer::setNumBytesStored(uint32_t numBytes) {
	published = numBytes;
}

void OggBuffer::addNumBytesStored(uint32_t numBytes) {
	published += numBytes;
}

void OggBuffer::addNumBytesConsumed(uint32_t numBytes) {
	consumed += numBytes;
}

uint32_t OggBuffer::getNumBytesStored() {
	return published;
}



void OggBuffer::recycle() {
	if(consumed != published) {
		return;
	}
	consumed = 0;
	published = 0;
}

void OggBuffer::reset() {
	consumed = 0; 
	published = 0;
}
void OggBuffer::resetConsumed() {
	consumed = 0;
}
void OggBuffer::resetStored() {
	published = 0;
}

// Retrieve from buffer and return data
//------------------------------------------------------------------------------
int OggBuffer::consumeBytes(uint8_t* buf, uint32_t numBytes) {
	int32_t left = (published - consumed);
	if(left < numBytes) {
		numBytes = left;
	}
	int32_t available = left - numBytes;

	if(available <= 0) {
		printf("published: %d consumed: %d asking: %d, left:%d\n", published, consumed, numBytes, left);
		return 0;
	}
	//printf("# still available: %d\n", available);
	if(available < numBytes) {
	///	printf("Resetting: %d\n", available);
	//	printf("Current info is now: %d and published: %d want to read: %d available: %d\n", consumed, published, numBytes, available);

		numBytes = available;
	}

	printf("# going to read %d\n", numBytes);
	memcpy(buf,buffer+consumed, numBytes);
	consumed += numBytes;

	printf(">>> consumed: %d, this: %p, %c %c %c\n", consumed, this, buffer[consumed], buffer[consumed+1], buffer[consumed+2]);
	printf("Consumed is now: %d and published: %d want to read: %d\n", consumed, published, numBytes);
	return numBytes;
}

uint8_t OggBuffer::consumeByte() {
	return consumeUI8();
}

uint8_t OggBuffer::consumeUI8() {
	uint8_t val = 0;
	memcpy(&val, buffer+consumed, 1);
	consumed += 1;
	return val;
}

uint16_t OggBuffer::consumeUI16() {
	uint16_t val = 0;
	memcpy(&val, buffer+consumed, 2);
	consumed += 2;
	return val;

}

uint32_t OggBuffer::consumeUI32() {
	uint32_t val = 0;
	memcpy(&val, buffer+consumed, 4);
	consumed += 4;
	return val;
}

uint64_t OggBuffer::consumeUI64() {
	uint64_t val = 0;
	memcpy(&val, buffer+consumed, 8);
	consumed += 8;
	return val;
}

int8_t  OggBuffer::consumeI8() {
	int8_t val = 0;
	memcpy(&val, buffer+consumed, 1);
	consumed += 1;
	return val;
}

int16_t OggBuffer::consumeI16() {
	int16_t val = 0;
	memcpy(&val, buffer+consumed, 2);
	consumed += 2;
	return val;
}

int32_t OggBuffer::consumeI32() {
	int32_t val = 0;
	memcpy(&val, buffer+consumed, 4);
	consumed += 4;
	return val;
}

int64_t OggBuffer::consumeI64() {
	int64_t val = 0;
	memcpy(&val, buffer+consumed, 8);
	consumed += 8;
	return val;
}

double OggBuffer::consumeDouble() {
	double val = 0;
	memcpy(&val, buffer+consumed, 8);
	consumed += 8;
	return val;
}

// when you assume the data is big endian, convert it to system 
// -----------------------------------------------------------------------------
uint16_t OggBuffer::consumeBigEndianUI16() {
	uint16_t val = 0;
	memcpy(&val, buffer+consumed, 2);
	consumed += 2;
	val = FromBE16(val);
	return val;
}

uint32_t OggBuffer::consumeBigEndianUI32() {
	uint32_t val = 0;
	memcpy(&val, buffer+consumed, 4);
	consumed += 4;
	val = FromBE32(val);
	return val;
}

uint64_t OggBuffer::consumeBigEndianUI64() {
	uint64_t val = 0;
	memcpy(&val, buffer+consumed, 8);
	val = FromBE64(val);
	consumed += 8;
	return val;
}

double OggBuffer::consumeBigEndianDouble() {
	uint64_t val = 0;
	memcpy(&val, buffer+consumed, 8);
	val = FromBE64(val);

	double d = 0.0;
	memcpy(&d, &val, 8);

	consumed += 8;
	return d;
}

int16_t OggBuffer::consumeBigEndianI16() {
	int16_t val = 0;
	memcpy(&val, buffer+consumed, 2);
	val = FromBE16(val);
	consumed += 2;
	return val;
}

int32_t OggBuffer::consumeBigEndianI32() {
	int32_t val = 0;
	memcpy(&val, buffer+consumed, 4);
	val = FromBE32(val);
	consumed += 4;
	return val;
}

int64_t OggBuffer::consumeBigEndianI64() {
	int64_t val = 0;
	memcpy(&val, buffer+consumed, 8);
	val = FromBE64(val);
	consumed += 8;
	return val;
}

// Searching for bytes in buffer and returning strings
// -----------------------------------------------------------------------------

// returns where we found the string, or 0 when not found.
int OggBuffer::consumeUntil(uint8_t until, string& found) {
	for(int i = consumed; i < published; ++i) {
		found.push_back((char)buffer[i]);
		if(buffer[i] == until) {
			consumed += i;
			return i;
		}
	}
	return 0;
}

// consume up to the given string, if not foudn return 0 else bytes consumed
int OggBuffer::consumeUntil(string until, string& found) {
	int search_size = until.size();
	const char* search = until.c_str();
	int k = 0;
	for(int i = consumed; i < published; ++i) {
		//cout << "store:" << (char)buffer[i] << endl;
		found.push_back((char)buffer[i]);
		if(buffer[i] == search[0]) {
			// out of buffer.
			if( (i + search_size) > published) {
				return 0;
			} 
			// check if next couple of chars is the search string.
			for(k = 0; k < search_size; ++k) {
				if(buffer[i+k] != search[k]) {
					break;
				}
			}
			// we found the search string, update consumed!
			if(k == search_size) {
				found.append((char*)buffer+(i+1), search_size-1);
				consumed += (i+search_size);
				return consumed;
			}
		}
	}
	found = "";
	return 0;
}	

string OggBuffer::consumeString(uint32_t upToNumBytes) {
	string str((char*)buffer+consumed, upToNumBytes);
	consumed += upToNumBytes;
	return str;
}

// we assume you used storeStringWithSize 
string OggBuffer::consumeStringWithSize() {
	uint16_t num_bytes_in_string = consumeBigEndianUI16();
	return consumeString(num_bytes_in_string);
}

// operators
//------------------------------------------------------------------------------
uint8_t& OggBuffer::operator[](uint32_t index) const {
	return buffer[index];
}

// helpers
//------------------------------------------------------------------------------
void OggBuffer::printHex(uint32_t start, uint32_t end) {
	if(end == 0) {
		end = published;
	}
	if(end > published) {
		end = published;
	}
	if(start > end) {
		start = end;		
	}
	int c = 0;
	for(int i = start,c=0; i < end; ++i,++c) {
		if(c > 20) {
			printf("\n");
			c = 0;
		}
		printf("%02X ", buffer[i]);
	}
	printf("\n");
}

void OggBuffer::printDoubleAsHex(double d) {
	uint8_t tmp_buf[8];
	memcpy(tmp_buf,&d,8);
	for(int i = 0; i < 8; ++i) {
		printf("%02X ", tmp_buf[i]);
	}
	printf("\n");
}

void OggBuffer::printUI16AsHex(uint16_t toPrint) {
	uint8_t tmp_buf[2];
	memcpy(tmp_buf, &toPrint, 2);
	printf("%02X %02X\n", tmp_buf[0], tmp_buf[1]);
}

bool OggBuffer::hasBytesToRead() {
	return consumed < published;
}
