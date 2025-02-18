#include "variant.hpp"

using namespace vm;

std::vector<uint8_t> Variant::encode_varint(uint64_t value) {
	std::vector<uint8_t> result;
	while (value > 127) {
		result.push_back(static_cast<uint8_t>((value & 0x7F) | 0x80)); // 7 bits with MSB = 1
		value >>= 7;
	}
	result.push_back(static_cast<uint8_t>(value & 0x7F)); // last byte, MSB = 0
	return result;
}

uint64_t Variant::decode_varint(const std::vector<uint8_t>& bytes, size_t& index) {
	uint64_t value = 0;
	int shift = 0;
	while (true) {
		uint8_t byte = bytes[index++];
		value |= (byte & 0x7F) << shift;
		if (!(byte & 0x80)) break; // if MSB is 0, stops
		shift += 7;
	}
	return value;
}
