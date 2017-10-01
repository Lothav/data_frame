//
// Created by luizorv on 10/1/17.
//

#ifndef DATA_FRAME_UTILS_H
#define DATA_FRAME_UTILS_H

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <netinet/in.h>

#define FR_CHECKSUM_OFFSET  8
#define FR_CHECKSUM_SIZE    2

#define	DF_SOCKET_TYPE_RECEIVER "passivo"
#define	DF_SOCKET_TYPE_SENDER 	"ativo"

#define FR_SYNC_EVAL 	    0xDCC023C2
#define FR_ST_SIZE_PAD 	    112

struct Frame {
	uint32_t 	__sync_1;
	uint32_t 	__sync_2;
	uint16_t 	chksum;
	uint16_t 	length;
	uint16_t    resvr;
}__attribute__((packed));

namespace DataFrame
{
	class Utils
	{
	public:
		static uint16_t ip_checksum(void* vdata, size_t length) {
			// Cast the data pointer to one that can be indexed.
			char* data = (char *) vdata;

			// Initialise the accumulator.
			uint16_t acc = 0xffff;

			// Handle complete 16-bit blocks.
			for (size_t i = 0; (i+1) < length; i += 2) {
				uint16_t word;
				memcpy(&word, data+i, 2);
				acc += ntohs(word);
				if (acc>0xffff) {
					acc-=0xffff;
				}
			}

			// Handle any partial block at the end of the data.
			if (length & 1) {
				uint16_t word = 0;
				memcpy(&word, data+length-1, 1);
				acc += ntohs(word);
				if (acc > 0xffff) {
					acc -= 0xffff;
				}
			}

			// Return the checksum in network byte order.
			return htons(~acc);
		}


		static const bool checkChecksum(void* buffer, struct Frame header, ssize_t rec_size)
		{
			// clear checksum field
			memset(((uint8_t *)buffer)+FR_CHECKSUM_OFFSET, 0, FR_CHECKSUM_SIZE);

			// calc checksum
			uint16_t checksum16 = Utils::ip_checksum(buffer, rec_size);
			return checksum16 == ntohs(header.chksum);
		}

		void static prettyBytes(char* buf, ssize_t bytes)
		{
			const char* suffixes[7] = {"B", "KB", "MB", "GB", "TB", "PB", "EB"};
			uint s = 0;
			double count = bytes;
			while (count >= 1024 && s < 7) {
				s++;
				count /= 1024;
			}
			if (count - floor(count) == 0.0)
				sprintf(buf, "%d %s", (int)count, suffixes[s]);
			else
				sprintf(buf, "%.1f %s", count, suffixes[s]);
		}

	private:
		Utils() {}
	};
}

#endif //DATA_FRAME_UTILS_H
