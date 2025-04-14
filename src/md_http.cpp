#include "md_http.hpp"

#ifdef linux

#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#elif defined(_WIN32) || defined(WIN32)

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#endif // linux

#include "interpreter.hpp"
#include "semantic_analysis.hpp"
#include "utils.hpp"
#include "constants.hpp"

using namespace core::modules;
using namespace core::runtime;
using namespace core::analysis;

ModuleHTTP::ModuleHTTP() {}

ModuleHTTP::~ModuleHTTP() = default;

void ModuleHTTP::register_functions(SemanticAnalyser* visitor) {
	visitor->builtin_functions["request"] = nullptr;
}

void ModuleHTTP::register_functions(Interpreter* visitor) {

	visitor->builtin_functions["request"] = [this, visitor]() {
		auto& scope = visitor->scopes[Constants::STD_NAMESPACE].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("req"))->get_value();

		RuntimeValue* config_value = val;
		if (TypeUtils::is_void(config_value->type)) {
			throw std::runtime_error("'req' is null");
		}
		flx_struct config_str = config_value->get_str();
		std::string hostname = config_str["hostname"]->get_s();
		std::string path = config_str["path"]->get_s();
		std::string method = config_str["method"]->get_s();
		std::string port = "80";
		std::string headers = "";
		std::string parameters = "";
		std::string data = config_str["data"]->get_s();

		// check mandatory parameters
		if (hostname.empty()) {
			throw std::runtime_error("Hostname must be informed.");
		}
		else if (method.empty()) {
			throw std::runtime_error("Method must be informed.");
		}

		// check if has path
		if (path.empty()) {
			path = "/";
		}

		// get port
		int param_port = config_str["port"]->get_i();
		if (param_port  != 0) {
			port = std::to_string(param_port);
		}

		// build parameters
		flx_struct str_parameters = config_str["parameters"]->get_str();
		for (auto parameter : str_parameters) {
			if (parameters.empty()) {
				parameters = "?";
			}
			else {
				parameters += "&";
			}
			parameters += parameter.first + "=" + parameter.second->get_s();
		}

		// build headers
		flx_struct str_headers = config_str["headers"]->get_str();
		for (auto header : str_headers) {
			headers += header.first + ": " + header.second->get_s() + "\r\n";
		}

#ifdef linux

		int sock;
		struct addrinfo hints;
		struct addrinfo* result = nullptr;

		// set hints to DNS resolution
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET; // IPv4
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// resolve DNS
		if (getaddrinfo(hostname.c_str(), port.c_str(), &hints, &result) != 0) {
			throw std::runtime_error("Failed to resolve hostname.");
		}

		// create socket
		sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (sock < 0) {
			freeaddrinfo(result);
			throw std::runtime_error("Socket creation failed.");
		}

		// connect to server
		if (connect(sock, result->ai_addr, result->ai_addrlen) < 0) {
			close(sock);
			freeaddrinfo(result);
			throw std::runtime_error("Connection failed.");
		}
		
#elif defined(_WIN32) || defined(WIN32)

		WSADATA wsa;
		SOCKET sock;
		struct sockaddr_in server;
		struct addrinfo* result = NULL;
		struct addrinfo hints;

		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
			throw std::runtime_error("Failed. Error Code: " + WSAGetLastError());
		}

		// set hints to DNS resolution
		ZeroMemory(&hints, sizeof(hints));
		hints.ai_family = AF_INET; // IPv4
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;

		// resolve DNS
		if (getaddrinfo(hostname.c_str(), port.c_str(), &hints, &result) != 0) {
			WSACleanup();
			throw std::runtime_error("Failed to resolve hostname.");
		}

		// create socket
		sock = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if (sock == INVALID_SOCKET) {
			freeaddrinfo(result);
			WSACleanup();
			throw std::runtime_error("Socket creation failed. Error: " + WSAGetLastError());
		}

		// connect to server
		if (connect(sock, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
			closesocket(sock);
			freeaddrinfo(result);
			WSACleanup();
			throw std::runtime_error("Connection failed. Error: " + WSAGetLastError());
		}
		
#endif // linux

		// prepare HTTP request
		std::string request = method + " " + path + parameters + " HTTP/1.1\r\n";
		request += "Host: " + hostname + "\r\n";

		// adds headers to request
		if (!headers.empty()) {
			request += headers;
		}
		else {
			request += "\r\n";
		}

		// adds body to request
		if (!data.empty()) {
			request += "Content-Length: " + std::to_string(data.length()) + "\r\n";
			request += "\r\n";
			request += data;
		}
		else {
			request += "\r\n";
		}

		// send HTTP request
		send(sock, request.c_str(), request.length(), 0);

		// receive responce
		char buffer[8192];
		int bytes_received = recv(sock, buffer, sizeof(buffer), 0);

#ifdef linux

		close(sock);
		
#elif defined(_WIN32) || defined(WIN32)
		
		closesocket(sock);
		freeaddrinfo(result);
		WSACleanup();
		
#endif // linux

		std::string raw_response(buffer, bytes_received);
		std::vector<std::string> response_lines = utils::StringUtils::split(raw_response, "\r\n");
		bool is_body = false;
		std::string res_body;

		// prepare flx instructions

		// set new scope
		const auto& current_program = visitor->current_program_stack.top();
		visitor->scopes[Constants::STD_NAMESPACE].push_back(std::make_shared<Scope>(current_program));
		auto& curr_scope = visitor->scopes[Constants::STD_NAMESPACE].back();

		// dictionary struct
		flx_struct res_headers_str;
		res_headers_str["root"] = visitor->allocate_value(new RuntimeValue(Type::T_VOID));
		res_headers_str["size"] = visitor->allocate_value(new RuntimeValue(flx_int(0)));
		auto headers_value = visitor->allocate_value(new RuntimeValue(res_headers_str, "Dictionary", Constants::STD_NAMESPACE));
		// dict identifier
		auto header_identifier = std::make_shared<ASTIdentifierNode>(std::vector<Identifier>{ Identifier("headers_value") }, Constants::STD_NAMESPACE, 0, 0);
		
		// create dict expr
		auto dict_expr = std::make_shared<ASTValueNode>(headers_value, 0, 0);

		// declare dict
		(std::make_shared<ASTDeclarationNode>("headers_value", Type::T_STRUCT, Type::T_UNDEFINED, std::vector<std::shared_ptr<ASTExprNode>>(),
			"Dictionary", Constants::STD_NAMESPACE, dict_expr, false, 0, 0))->accept(visitor);

		// dictionary emplace function declaration
		auto identifier_vector = std::vector<Identifier>{ Identifier("emplace") };
		auto fcall = std::make_shared<ASTFunctionCallNode>(Constants::STD_NAMESPACE, identifier_vector,
			std::vector<std::shared_ptr<ASTExprNode>>(), std::vector<Identifier>(), nullptr, 0, 0);

		for (size_t i = 1; i < response_lines.size(); ++i) {
			auto& line = response_lines[i];
			if (is_body) {
				res_body += line;
			}
			else {
				if (line == "") {
					is_body = true;
					continue;
				}

				// setup emplace parameters
				auto header = utils::StringUtils::split(line, ": ");
				auto parameters = std::vector<std::shared_ptr<ASTExprNode>> {
					// dictionary
					header_identifier,
					// key
					std::make_shared<ASTLiteralNode<flx_string>>(header[0], 0, 0),
					// value
					std::make_shared<ASTLiteralNode<flx_string>>(header[1], 0, 0)
				};

				// call emplace
				fcall->parameters = parameters;
				fcall->accept(visitor);

			}
		}

		// create response struct
		flx_struct res_str;
		auto status = utils::StringUtils::split(response_lines[0], ' ');
		res_str["http_version"] = visitor->allocate_value(new RuntimeValue(flx_string(status[0])));
		res_str["status"] = visitor->allocate_value(new RuntimeValue(flx_int(stoll(status[1]))));
		res_str["status_description"] = visitor->allocate_value(new RuntimeValue(flx_string(status[2])));
		res_str["headers"] = headers_value;
		res_str["data"] = visitor->allocate_value(new RuntimeValue(flx_string(res_body)));
		res_str["raw"] = visitor->allocate_value(new RuntimeValue(flx_string(raw_response)));

		visitor->current_expression_value = visitor->allocate_value(new RuntimeValue(res_str, "HttpResponse", Constants::STD_NAMESPACE));

		// remove scope
		visitor->scopes[Constants::STD_NAMESPACE].pop_back();

		};

}
