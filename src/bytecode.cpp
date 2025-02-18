#include <fstream>
#include <iomanip>

#include "bytecode.hpp"

using namespace vm;

BytecodeInstruction::BytecodeInstruction()
	: opcode(OpCode::OP_RES), operand(nullptr) {}

BytecodeInstruction::BytecodeInstruction(OpCode opcode, uint8_t* operand)
	: opcode(opcode), operand(operand) {}

BytecodeInstruction::BytecodeInstruction(OpCode opcode, uint8_t operand)
	: opcode(opcode), operand(to_byteopnd(operand)) {}

BytecodeInstruction::BytecodeInstruction(OpCode opcode, size_t operand)
	: opcode(opcode), operand(to_byteopnd(operand)) {}

BytecodeInstruction::BytecodeInstruction(OpCode opcode, flx_bool operand)
	: opcode(opcode), operand(to_byteopnd(operand)) {}

BytecodeInstruction::BytecodeInstruction(OpCode opcode, flx_int operand)
	: opcode(opcode), operand(to_byteopnd(operand)) {}

BytecodeInstruction::BytecodeInstruction(OpCode opcode, flx_float operand)
	: opcode(opcode), operand(to_byteopnd(operand)) {}

BytecodeInstruction::BytecodeInstruction(OpCode opcode, flx_char operand)
	: opcode(opcode), operand(to_byteopnd(operand)) {}

BytecodeInstruction::BytecodeInstruction(OpCode opcode, const flx_string& operand)
	: opcode(opcode), operand(to_byteopnd(operand)) {}

uint8_t BytecodeInstruction::get_uint8_operand() const {
	return *operand;
}

size_t BytecodeInstruction::get_size_operand() const {
	return *reinterpret_cast<size_t*>(operand);
}

flx_bool BytecodeInstruction::get_bool_operand() const {
	return *reinterpret_cast<flx_bool*>(operand);
}

flx_int BytecodeInstruction::get_int_operand() const {
	return *reinterpret_cast<flx_int*>(operand);
}

flx_float BytecodeInstruction::get_float_operand() const {
	return *reinterpret_cast<flx_float*>(operand);
}

flx_char BytecodeInstruction::get_char_operand() const {
	return *reinterpret_cast<flx_char*>(operand);
}

flx_string BytecodeInstruction::get_string_operand() const {
	uint64_t size;
	std::memcpy(&size, operand, sizeof(uint64_t));

	std::string result(reinterpret_cast<char*>(operand + sizeof(uint64_t)), size);
	return result;
}

template <typename T>
uint8_t* BytecodeInstruction::to_byteopnd(T value) {
	static_assert(std::is_arithmetic<T>::value, "Only arithmetic types are supported");

	uint8_t* buffer = new uint8_t[sizeof(T)];

	std::memcpy(buffer, &value, sizeof(T));

	return buffer;
}

uint8_t* BytecodeInstruction::to_byteopnd(const std::string& str) {
	uint64_t size = static_cast<uint64_t>(str.size());
	size_t total_size = sizeof(size) + str.size();

	uint8_t* buffer = new uint8_t[total_size];

	std::memcpy(buffer, &size, sizeof(size));
	std::memcpy(buffer + sizeof(size), str.data(), str.size());

	return buffer;
}

void BytecodeInstruction::write_bytecode_table(const std::vector<BytecodeInstruction>& instructions, const std::string& filename) {
	std::ofstream file(filename);

	if (!file.is_open()) {
		std::cerr << "Error opening file: " << filename << std::endl;
		return;
	}

	for (const auto& instruction : instructions) {
		file << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(instruction.opcode) << "\t";

		file << OP_NAMES.at(instruction.opcode);
		
		file << std::setw(22 - OP_NAMES.at(instruction.opcode).size()) << std::setfill(' ') << "\t" << std::dec;

		switch (instruction.opcode)
		{
		case OP_SET_TYPE:
		case OP_TYPE_PARSE:
		case OP_IS_TYPE:
			file << int(instruction.get_uint8_operand());
			break;
		case OP_SET_ELEMENT:
		case OP_JUMP:
		case OP_JUMP_IF_FALSE:
		case OP_JUMP_IF_FALSE_OR_NEXT:
		case OP_JUMP_IF_TRUE:
		case OP_JUMP_IF_TRUE_OR_NEXT:
			file << instruction.get_size_operand();
			break;
		case OP_PUSH_BOOL:
		case OP_INIT_ARRAY:
			file << instruction.get_bool_operand();
			break;
		case OP_PUSH_INT:
			file << instruction.get_int_operand();
			break;
		case OP_PUSH_FLOAT:
			file << instruction.get_float_operand();
			break;
		case OP_PUSH_CHAR:
			file << instruction.get_char_operand();
			break;
		case OP_PUSH_STRING:
		case OP_PUSH_FUNCTION:
		case OP_INIT_STRUCT:
		case OP_SET_FIELD:
		case OP_SET_TYPE_NAME:
		case OP_SET_TYPE_NAME_SPACE:
		case OP_STRUCT_START:
		case OP_LOAD_VAR:
		case OP_STORE_VAR:
		case OP_FUN_START:
		case OP_CALL:
		case OP_LOAD_SUB_ID:
			file << instruction.get_string_operand();
			break;
		default:
			if (instruction.operand) {
				file << "[UNHANDLED]\t" << instruction.operand;
			}
			break;
		}

		file << std::endl;
	}

	file.close();
}
