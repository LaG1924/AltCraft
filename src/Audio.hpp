#pragma once

#include "Vector.hpp"

namespace Audio {
	void Init();
	void Deinit() noexcept;

	void UpdateListener(Vector3<float> direction, Vector3<float> up, Vector3<float> pos, Vector3<float> speed) noexcept;

	void LoadOGG(void *mapping, size_t mlen, void **bufferPtr, int *channels, int *freq, size_t *blen) noexcept;
}
