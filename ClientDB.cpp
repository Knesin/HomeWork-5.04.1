#include "ClientDB.h"

ClientDB::ClientDB(const std::string& idb) :conn_db_{ pqxx::connection(idb) }
{
	create_db_();
	conn_db_.prepare("add_client", "INSERT INTO client (first_name, last_name, email) VALUES ($1, $2, $3)");
	conn_db_.prepare("add_telephone", "INSERT INTO telephone(id_client, telephone_number) VALUES ($1, $2)");
	conn_db_.prepare("change_first_name", "UPDATE client SET first_name = $2 WHERE id = $1");
	conn_db_.prepare("change_last_name", "UPDATE client SET last_name = $2 WHERE id = $1");
	conn_db_.prepare("change_email", "UPDATE client SET email = $2 WHERE id = $1");
	conn_db_.prepare("delete_telephone", "DELETE FROM telephone WHERE id = $1");
	conn_db_.prepare("delete_all_client_telephone", "DELETE FROM telephone WHERE id_client = $1");
	conn_db_.prepare("delete_client", "DELETE FROM client WHERE id = $1");

};

void ClientDB::create_db_()
{
	pqxx::work tx{ conn_db_ };
	tx.exec("CREATE TABLE IF NOT EXISTS client( "
		"id SERIAL PRIMARY KEY, "
		"first_name TEXT NOT NULL, "
		"last_name TEXT NOT NULL, "
		"email TEXT NOT NULL UNIQUE)"
	);
	tx.exec("CREATE TABLE IF NOT EXISTS telephone( "
		"id SERIAL PRIMARY KEY, "
		"id_client INTEGER NOT NULL REFERENCES client(id), "
		"telephone_number VARCHAR(16) NOT NULL UNIQUE)"
	);
	tx.commit();
}
void ClientDB::print_all()
{
	std::cout << "+----------------+----------------+--------------------+------------------+" << std::endl;
	std::cout << "|   first name   |   last name    |       email        |    telephone     |" << std::endl;
	std::cout << "+----------------+----------------+--------------------+------------------+" << std::endl;
	pqxx::work tx{ conn_db_ };
	for (auto [i] : tx.query<int>("SELECT id FROM client")) {
		tx.abort();
		print_(i);	
	};
	std::cout << "+----------------+----------------+--------------------+------------------+" << std::endl;
};
void ClientDB::print_id(const int id_client)
{
	bool check_id = check_id_client_(id_client);
	if (check_id) {
		std::cout << "+----------------+----------------+--------------------+------------------+" << std::endl;
		std::cout << "|   first name   |   last name    |       email        |    telephone     |" << std::endl;
		std::cout << "+----------------+----------------+--------------------+------------------+" << std::endl;
		print_(id_client);
		std::cout << "+----------------+----------------+--------------------+------------------+" << std::endl;
	}
	else {
		std::cout << "ERROR: Client not found";
	};
}
void ClientDB::print_id(const std::vector<int>& id_client)
{
	std::cout << "+----------------+----------------+--------------------+------------------+" << std::endl;
	std::cout << "|   first name   |   last name    |       email        |    telephone     |" << std::endl;
	std::cout << "+----------------+----------------+--------------------+------------------+" << std::endl;
	for (auto i : id_client) {
		bool check_id = check_id_client_(i);
		if (check_id) {
			print_(i);
		}
	};
	std::cout << "+----------------+----------------+--------------------+------------------+" << std::endl;
}
void ClientDB::print_(const int id_client)
{
	int count = 0;
	pqxx::work tx{ conn_db_ };
	for (auto [f_n, l_n, em] : tx.query<std::string, std::string, std::string>
			("SELECT c.first_name, c.last_name, c.email "
			"FROM client c "
			"WHERE c.id = " + std::to_string(id_client) + ";")) {
		//std::wstring f_n(f_n_);
		//std::wstring l_n(l_n_);
		f_n.insert(0, (16 - f_n.size()) / 2, ' ');
		f_n.insert(f_n.size(), 16 - f_n.size(), ' ');
		l_n.insert(0, (16 - l_n.size()) / 2, ' ');
		l_n.insert(l_n.size(), 16 - l_n.size(), ' ');
		em.insert(0, (20 - em.size()) / 2, ' ');
		em.insert(em.size(), 20 - em.size(), ' ');
		std::cout << "|" << f_n << "|" << l_n << "|" << em << "|";
	}
	for (auto [t] : tx.query<std::string>
		("SELECT t.telephone_number "
			"FROM telephone t "
			"WHERE t.id_client = " + std::to_string(id_client) + ";")) {
		t.insert(0, (18 - t.size()) / 2, ' ');
		t.insert(t.size(), 18 - t.size(), ' ');
		if (t.size() > 0 && count == 0) {
			std::cout << t << "|" << std::endl;
			count++;
		}
		else if (t.size() > 0 && count > 0) {
			std::cout << "|                |                |                    |" << t << "|" << std::endl;
		}
	}
	if (count == 0){
		std::cout << "                  |" << std::endl;
	}
}

bool ClientDB::add_client(std::string first_name, std::string last_name, std::string email) {
	bool result = true;
	int check_email = search_client_email(email);
	if (check_email == 0 ){
		pqxx::work tx{ conn_db_ };
		tx.exec_prepared("add_client", first_name, last_name, email);
		tx.commit();
	}
	else {
		last_error = "ERROR: email already exist";
		result = false;
	}
	return result;
}

bool ClientDB::add_client_and_telephones(std::string first_name, std::string last_name, std::string email, std::vector<std::string> telephones)
{
	int count = 0;
	bool result = true;
	int check_email = search_client_email(email);
	for (const std::string& t : telephones) {
		int check_telephon = search_client_telephone(t);
		if (check_telephon > 0) {
			count++;
		}
	}
	if (count > 0) {
		result = false;
		last_error = "ERROR: can not create telephone:" + std::to_string(count) + " telephone(s) already exist";
	}
	if (check_email == 0 && result) {
		add_client(first_name, last_name, email);		
		pqxx::work tx{ conn_db_ };
		int max_id = tx.query_value<int>("SELECT MAX(id) FROM client");
		tx.abort();
		for (const std::string& t : telephones) {
			add_telephone(max_id, t);
		}
	}
	else if (result){
		last_error = "ERROR: email already exist";
		result = false;
	}
	else {
		last_error += " AND email already exist";
	}
	return result;
}

bool ClientDB::add_telephone(int id_client, std::string telephone_number)
{
	bool result = true;
	int check_telephon = search_client_telephone(telephone_number);
	if (check_telephon == 0) {
		pqxx::work tx{ conn_db_ };
		tx.exec_prepared("add_telephone", id_client, telephone_number);
		tx.commit();
	}
	else {
		last_error = "ERROR: telephone number already exist";
		result = false;
	};
	return result;
}

bool ClientDB::change_first_name(int id_client, std::string first_name)
{
	bool result = true;
	bool check_id = check_id_client_(id_client);
	pqxx::work tx{ conn_db_ };
	if (check_id) {
		tx.exec_prepared("change_first_name", id_client, first_name);
		tx.commit();
	}
	else {
		last_error = "ERROR: Client not found";
		result = false;
	}
	return result;
}

bool ClientDB::change_last_name(int id_client, std::string last_name)
{
	bool result = true;
	bool check_id = check_id_client_(id_client);
	pqxx::work tx{ conn_db_ };
	if (check_id) {
		tx.exec_prepared("change_last_name", id_client, last_name);
		tx.commit();
	}
	else {
		last_error = "ERROR: Client not found";
		result = false;
	}
	return result;
}

bool ClientDB::change_email(int id_client, std::string email)
{
	bool result = true;
	int check_email = search_client_email(email);
	if (check_email == 0) {
		bool check_id = check_id_client_(id_client);
		pqxx::work tx{ conn_db_ };
		if (check_id) {
			tx.exec_prepared("change_email", id_client, email);
			tx.commit();
		}
		else {
			last_error = "ERROR: Client not found";
			result = false;
		}
	}
	else {
		last_error = "ERROR: email already exist";
		result = false;
	}
	return result;
}

bool ClientDB::check_id_client_(int id_client)
{
	pqxx::work tx{ conn_db_ };
	bool check_id = false;
	check_id = tx.query_value<bool>("SELECT EXISTS(SELECT id FROM client WHERE id =" + std::to_string(id_client) + ");");
	return check_id;
}

bool ClientDB::check_id_telephone_(int id_telephone)
{
	pqxx::work tx{ conn_db_ };
	bool check_id = false;
	check_id = tx.query_value<bool>("SELECT EXISTS(SELECT id FROM telephone WHERE id =" + std::to_string(id_telephone) + ");");
	return check_id;
}

bool ClientDB::delete_telephone(int id_telephone)
{
	bool result = true;
	bool check_id = check_id_telephone_(id_telephone);
	pqxx::work tx{ conn_db_ };
	if (check_id) {
		tx.exec_prepared("delete_telephone", id_telephone);
		tx.commit();
	}
	else {
		last_error = "ERROR: Telephone not found";
		result = false;
	}
	return result;
}

bool ClientDB::delete_client(int id_client)
{
	bool result = true;
	bool check_id = check_id_client_(id_client);
	pqxx::work tx{ conn_db_ };
	if (check_id) {
		tx.exec_prepared("delete_all_client_telephone", id_client);
		tx.exec_prepared("delete_client", id_client);
		tx.commit();
	}
	else {
		last_error = "ERROR: Client not found";
		result = false;
	}
	return result;
}

std::vector<int> ClientDB::search_client(std::string key)
{
	std::vector<int> result;
	pqxx::work tx{ conn_db_ };
	std::string key_esc = tx.esc(key);
	for (auto [i] : tx.query<int>("SELECT c.id "
								"FROM client c "
								"JOIN telephone t ON c.id = t.id_client "
								"WHERE c.first_name = '"+ key_esc +"' OR c.last_name = '" + key_esc + 
								"' OR c.email = '" + key_esc + "' OR t.telephone_number = '" + key_esc + "' "
								"GROUP BY c.id")) {
		result.push_back(i);
	};
	return result;
}
int ClientDB::search_client_email(std::string email)
{
	int result = 0;
	pqxx::work tx{ conn_db_ };
	std::string key_esc = tx.esc(email);
	result = std::get<0>(*(tx.query<int>("SELECT c.id " // query_value - не работает с пустой таблицей
								 "FROM client c "
								 "WHERE c.email = '" + key_esc + "'").begin()));
	return result;
}
int ClientDB::search_client_telephone(std::string telephone_number)
{
	int result = 0;
	pqxx::work tx{ conn_db_ };
	std::string key_esc = tx.esc(telephone_number);
	result = std::get<0>(*(tx.query<int>("SELECT c.id " // query_value - не работает с пустой таблицей
								 "FROM client c "
								 "JOIN telephone t ON c.id = t.id_client "
								 "WHERE t.telephone_number = '" + key_esc + "'").begin()));
	return result;
}
std::string ClientDB::get_last_error()
{
	return last_error;
}
;

