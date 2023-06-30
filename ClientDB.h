#pragma once
#include <pqxx/pqxx>
#include <string>
#include <iostream>
#include <vector>

class ClientDB {
public:
	ClientDB(const std::string &idb);
	
	bool add_client(std::string first_name, std::string last_name, std::string email);
	bool add_client_and_telephones(std::string first_name, std::string last_name, std::string email, std::vector<std::string> telephones);
	bool add_telephone(int id_client, std::string telephone_number);
	bool change_first_name(int id_client, std::string first_name);
	bool change_last_name(int id_client, std::string last_name);
	bool change_email(int id_client, std::string email);
	bool delete_telephone(int id_telephone);
	bool delete_client(int id_client);
	std::vector<int> search_client(std::string key);
	int search_client_email(std::string email);
	int search_client_telephone(std::string telephone_number);
	std::string get_last_error();
	void create_db_();
	void print_all();
	void print_id(const int id_client);
	void print_id(const std::vector<int>& id_client);
private:
	pqxx::connection conn_db_;
	void print_(const int id_client);
	std::string last_error;
	bool check_id_client_(int id_client);
	bool check_id_telephone_(int id_telephone);
};