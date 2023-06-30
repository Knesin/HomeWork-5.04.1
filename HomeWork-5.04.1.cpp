#include <iostream>
#include <pqxx/pqxx>
#include "ClientDB.h"
#include <string>
#include <Windows.h>

int main()
{
    //setlocale(LC_ALL, "Russian");
    setlocale(LC_ALL, "ru_RU.urf-8");
    //system("chcp 1251");
    std::string initialize_connection_db(   "host=localhost "
                                            "port=5432 "
                                            "dbname=homework0504 "
                                            "user= "
                                            "password="); 
    try
    {
        ClientDB cdb(initialize_connection_db);//Структура БД создаётся в конмтрукторе
        ///Заполняемм БД 3я полями
        std::cout << "Создаем таблищы и пользователей" << std::endl;
        if (!cdb.add_client("Ivan", "Ivanov", "fdgvvvv@ml.ru")) {
            std::cout << cdb.get_last_error() << std::endl;
        };
        if (!cdb.add_client("Mark", "", "adfsd@mml.ru")) {
            std::cout << cdb.get_last_error() << std::endl;
        };

        if (!cdb.add_client_and_telephones("Jon", "Smith", "ss@mm.mc", { "11" , "22"})) {
            std::cout << cdb.get_last_error() << std::endl;
        };
        if (!cdb.add_client_and_telephones("Jon", "Konor", "Jom@mm.mc", { "54663" , "3457777", "112233" })) {
            std::cout << cdb.get_last_error() << std::endl;
        };

        cdb.print_all(); 

        //Добавляем 2у клиенту телефон
        std::cout << "Добавляем 2у клиенту телефон" << std::endl;
        if (!cdb.add_telephone(2, "123456")) {
            std::cout << cdb.get_last_error() << std::endl;
        }
        cdb.print_id(2);
        //Изменения клиента 
        std::cout << "Изменяем 2ого клиента" << std::endl;
        cdb.change_first_name(2, "Markus");
        cdb.change_last_name(2, "Mak");
        if (!cdb.change_email(2, "MMoak@mm.com")) {
            std::cout << cdb.get_last_error() << std::endl;
        }
        cdb.print_id(2);
        //Удаляем телефон
        std::cout << "Удаляем телефон" << std::endl;
        if (!cdb.delete_telephone(1)) {
            std::cout << cdb.get_last_error() << std::endl;
        }
        cdb.print_id(3);

        //Поиск клиента
        std::cout << "Ищем Jon" << std::endl;
        cdb.print_id(cdb.search_client("Jon"));

        //Удаляем клиента
        std::cout << "Удаляем клиента" << std::endl;
        if (!cdb.delete_client(4)) {
            std::cout << cdb.get_last_error() << std::endl;
        }
        cdb.print_all();
    }
    catch(const std::exception e){
        std::cout << "Error: " << e.what() << std::endl;

    }
}
