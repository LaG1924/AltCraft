#include "Audio.hpp"

#include "stb_vorbis.h"

void Audio::LoadOGG(void *mapping, size_t mlen, void **bufferPtr, int *channels, int *freq, size_t *blen) noexcept {
	int stb_errno;
	stb_vorbis *info = stb_vorbis_open_memory(reinterpret_cast<unsigned char*>(mapping), mlen, &stb_errno, NULL);
	*blen = stb_vorbis_decode_memory(reinterpret_cast<uint8_t*>(mapping), mlen, channels, freq, reinterpret_cast<short**>(bufferPtr));
	stb_vorbis_close(info);
}
