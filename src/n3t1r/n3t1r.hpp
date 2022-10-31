#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

namespace N3T1R {
	namespace C_API {
		typedef struct IRCommunicationHandler IRCommunicationHandler;
		typedef struct Result_String Result_String;
		typedef struct Vec_String Vec_String;
		extern "C" {
		void n3t1r_str_free(char *str_ptr);
		bool n3t1r_result_is_error(struct Result_String *result_ptr);
		char *n3t1r_result_get_error_message(struct Result_String *result_ptr);
		void n3t1r_result_free(struct Result_String *result_ptr);
		struct Vec_String *n3t1r_vec_string_new(void);
		void n3t1r_vec_string_free(struct Vec_String *vector_ptr);
		size_t n3t1r_vec_string_len(struct Vec_String *vector_ptr);
		char *n3t1r_vec_string_get(struct Vec_String *vector_ptr, size_t index);
		struct Result_String *n3t1r_get_available_serial_ports(struct Vec_String *names_vector_ptr, struct Vec_String *descriptions_vector_ptr);
		struct Result_String *n3t1r_get_available_rooms(struct Vec_String *vector_ptr);
		struct IRCommunicationHandler *n3t1r_irch_new(void);
		void n3t1r_irch_free(struct IRCommunicationHandler *instance_ptr);
		void n3t1r_irch_select_serial_backend(struct IRCommunicationHandler *instance_ptr, const char *port_name_ptr);
		void n3t1r_irch_select_rendezvous_backend(struct IRCommunicationHandler *instance_ptr, const char *room_name_ptr);
		void n3t1r_irch_select_network_backend(struct IRCommunicationHandler *instance_ptr, uint16_t source_port, const char *destination_host_ptr, uint16_t destination_port);
		struct Result_String *n3t1r_irch_enable(struct IRCommunicationHandler *instance_ptr);
		void n3t1r_irch_disable(struct IRCommunicationHandler *instance_ptr);
		struct Result_String *n3t1r_irch_send(struct IRCommunicationHandler *instance_ptr, const uint8_t *data_ptr, size_t data_len);
		struct Result_String *n3t1r_irch_receive(struct IRCommunicationHandler *instance_ptr, uint8_t *data_ptr, size_t *data_len_ptr);
		} // extern "C"
	}

	class IRCommunicationHandler {
		private:
			static std::string as_string(char* c_str) {
				std::string string = std::string(c_str);
				C_API::n3t1r_str_free(c_str);
				return string;
			}

			static void throw_on_error(C_API::Result_String* result_ptr) {
				if (C_API::n3t1r_result_is_error(result_ptr)) {
					std::string error_message = as_string(C_API::n3t1r_result_get_error_message(result_ptr));
					C_API::n3t1r_result_free(result_ptr);
					throw std::runtime_error(error_message);
				}
				else {
					C_API::n3t1r_result_free(result_ptr);
				}
			}

			C_API::IRCommunicationHandler* irch_ptr;

		public:
			constexpr static const uintptr_t MAXIMUM_DATA_LENGTH = 255;

			static std::map<std::string, std::string> get_available_serial_ports() {
				C_API::Vec_String* names_vector_ptr = C_API::n3t1r_vec_string_new();
				C_API::Vec_String* descriptions_vector_ptr = C_API::n3t1r_vec_string_new();
				
				throw_on_error(C_API::n3t1r_get_available_serial_ports(names_vector_ptr, descriptions_vector_ptr));
				
				size_t vector_len = C_API::n3t1r_vec_string_len(names_vector_ptr);
				std::map<std::string, std::string> serial_ports;
				
				for (size_t i = 0; i < vector_len; ++i) {
					char* name = C_API::n3t1r_vec_string_get(names_vector_ptr, i);
					char* description = C_API::n3t1r_vec_string_get(descriptions_vector_ptr, i);

					serial_ports[name] = description;

					C_API::n3t1r_str_free(name);
					C_API::n3t1r_str_free(description);
				}

				C_API::n3t1r_vec_string_free(names_vector_ptr);
				C_API::n3t1r_vec_string_free(descriptions_vector_ptr);

				return serial_ports;
			}

			static std::vector<std::string> get_available_rooms() {
				C_API::Vec_String* vector_ptr = C_API::n3t1r_vec_string_new();
				
				throw_on_error(C_API::n3t1r_get_available_rooms(vector_ptr));
				
				size_t vector_len = C_API::n3t1r_vec_string_len(vector_ptr);
				std::vector<std::string> rooms(vector_len);
				
				for (size_t i = 0; i < vector_len; ++i) {
					char* room_name = C_API::n3t1r_vec_string_get(vector_ptr, i);
					rooms[i] = room_name;
					C_API::n3t1r_str_free(room_name);
				}

				C_API::n3t1r_vec_string_free(vector_ptr);
				
				return rooms;
			}

			IRCommunicationHandler() : irch_ptr(C_API::n3t1r_irch_new()) {}

			~IRCommunicationHandler() {
				C_API::n3t1r_irch_free(this->irch_ptr);
			}
		
			void reset() {
				C_API::n3t1r_irch_free(this->irch_ptr);
				this->irch_ptr = C_API::n3t1r_irch_new();
			}

			void select_serial_backend(const char *port_name) {
				C_API::n3t1r_irch_select_serial_backend(this->irch_ptr, port_name);
			}

			void select_serial_backend(std::string port_name) {
				this->select_serial_backend(port_name.c_str());
			}

			void select_rendezvous_backend(const char *room_name) {
				C_API::n3t1r_irch_select_rendezvous_backend(this->irch_ptr, room_name);
			}

			void select_rendezvous_backend(std::string room_name) {
				this->select_rendezvous_backend(room_name.c_str());
			}

			void select_network_backend(uint16_t source_port, const char *destination_host, uint16_t destination_port) {
				C_API::n3t1r_irch_select_network_backend(this->irch_ptr, source_port, destination_host, destination_port);
			}

			void select_network_backend(uint16_t source_port, std::string destination_host, uint16_t destination_port) {
				this->select_network_backend(source_port, destination_host.c_str(), destination_port);
			}

			void enable() {
				throw_on_error(C_API::n3t1r_irch_enable(this->irch_ptr));
			}

			void disable() {
				C_API::n3t1r_irch_disable(this->irch_ptr);
			}

			void send(const uint8_t *data_ptr, size_t data_len) {
				throw_on_error(C_API::n3t1r_irch_send(this->irch_ptr, data_ptr, data_len));
			}

			size_t receive(uint8_t *data_ptr, size_t max_data_len) {
				size_t data_sent_len = max_data_len;
				this->throw_on_error(C_API::n3t1r_irch_receive(this->irch_ptr, data_ptr, &data_sent_len));
				return data_sent_len;
			}

	};
}