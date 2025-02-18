#ifndef VARIANT_HPP
#define VARIANT_HPP

#include <vector>
#include <cstdint>

namespace vm {

	class Variant {

		static std::vector<uint8_t> encode_varint(uint64_t value);
		static uint64_t decode_varint(const std::vector<uint8_t>& bytes, size_t& index);

	};

}

#endif // !VARIANT_HPP
