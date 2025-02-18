#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#include "md_http.hpp"

#include "interpreter.hpp"
#include "semantic_analysis.hpp"
#include "utils.hpp"

#pragma comment(lib, "ws2_32.lib")

using namespace modules;
using namespace visitor;

ModuleHTTP::ModuleHTTP() {}

ModuleHTTP::~ModuleHTTP() = default;

void ModuleHTTP::register_functions(visitor::SemanticAnalyser* visitor) {
	visitor->builtin_functions["request"] = nullptr;
}

void ModuleHTTP::register_functions(visitor::Interpreter* visitor) {

	visitor->builtin_functions["request"] = [this, visitor]() {
		auto& scope = visitor->scopes[language_namespace].back();
		auto val = std::dynamic_pointer_cast<RuntimeVariable>(scope->find_declared_variable("config"))->value;

		RuntimeValue* config_value = val;
		if (parser::is_void(config_value->type)) {
			throw std::exception("Config is null");
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

		closesocket(sock);
		freeaddrinfo(result);
		WSACleanup();

		std::string raw_response(buffer, bytes_received);
		std::vector<std::string> response_lines = utils::StringUtils::split(raw_response, "\r\n");
		bool is_body = false;
		std::string res_body;

		// create response struct
		flx_struct res_str;
		auto status = utils::StringUtils::split(response_lines[0], ' ');
		res_str["http_version"] = visitor->alocate_value(new RuntimeValue(flx_string(status[0])));
		res_str["status"] = visitor->alocate_value(new RuntimeValue(flx_int(stoll(status[1]))));
		res_str["status_description"] = visitor->alocate_value(new RuntimeValue(flx_string(status[2])));
		// prepare flx instructions
		flx_struct res_headers_str;
		res_headers_str["root"] = visitor->alocate_value(new RuntimeValue(Type::T_VOID));
		res_headers_str["size"] = visitor->alocate_value(new RuntimeValue(flx_int(0)));
		auto headers_value = visitor->alocate_value(new RuntimeValue(res_headers_str, "Dictionary", language_namespace));
		auto header_identifier = std::make_shared<ASTIdentifierNode>(std::vector<Identifier>{ Identifier("headers_value") }, language_namespace, 0, 0);
		auto identifier_vector = std::vector<Identifier>{ Identifier("emplace") };
		auto fcall = std::make_shared<ASTFunctionCallNode>(language_namespace, identifier_vector, std::vector<std::shared_ptr<ASTExprNode>>(), 0, 0);

		const auto& prg = visitor->current_program.top();
		visitor->scopes[language_namespace].push_back(std::make_shared<Scope>(prg));
		auto& curr_scope = visitor->scopes[language_namespace].back();
		(std::make_shared<ASTDeclarationNode>("headers_value", Type::T_STRUCT, Type::T_UNDEFINED, std::vector<std::shared_ptr<ASTExprNode>>(),
			"Dictionary", language_namespace, std::make_shared<ASTNullNode>(0, 0), false, 0, 0))->accept(visitor);
		auto var = std::dynamic_pointer_cast<RuntimeVariable>(curr_scope->find_declared_variable("headers_value"));
		var->set_value(headers_value);

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

				auto header = utils::StringUtils::split(line, ": ");
				auto parameters = std::vector<std::shared_ptr<ASTExprNode>> {
					header_identifier,
					std::make_shared<ASTLiteralNode<flx_string>>(header[0], 0, 0),
					std::make_shared<ASTLiteralNode<flx_string>>(header[1], 0, 0)
				};
				fcall->parameters = parameters;
				fcall->accept(visitor);

			}
		}

		visitor->scopes[language_namespace].pop_back();

		res_str["headers"] = headers_value;
		res_str["data"] = visitor->alocate_value(new RuntimeValue(flx_string(res_body)));
		res_str["raw"] = visitor->alocate_value(new RuntimeValue(flx_string(raw_response)));

		visitor->current_expression_value = visitor->alocate_value(new RuntimeValue(res_str, "HttpResponse", language_namespace));

		};

}

void ModuleHTTP::register_functions(visitor::Compiler* visitor) {}

void ModuleHTTP::register_functions(vm::VirtualMachine* vm) {}
