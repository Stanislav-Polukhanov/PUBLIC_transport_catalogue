#pragma once

#include "input_reader.h"
#include "stat_reader.h"

#include <iostream>
#include <string>
#include <fstream>

//read_input_functions from searchserver
/*
#include <iostream>
using namespace std;

string ReadLine() {
	string s;
	getline(cin, s);
	return s;
}

int ReadLineWithNumber() {
	int result;
	cin >> result;
	ReadLine();
	return result;
}
*/

void Test_All_Queries(){
	//D:\\Cpp\\Repository\\cpp-transport-spravochnik\\Input_files\\
	//C:\\Users\\qwert\\source\\repos\\cpp-transport-spravochnik\\Input_files\\
	// input_all.txt   tsA_case1_input.txt   input_falling.txt	tsB_case1_input		tsC_case1_input
	
	std::ifstream filestream("D:\\Cpp\\Repository\\cpp-transport-spravochnik\\Input_files\\tsC_case1_input.txt", std::ios_base::in);

	DataBaseFiller DBfiller;
	TransportCatalogue catalogue;

	int count;
	filestream >> count;
	std::string _;
	std::getline(filestream, _);

	for (int i = 0; i < count; ++i) {
		std::string s;
		std::getline(filestream, s);
		DBfiller.SaveQueries(std::move(s));
	}

	DBfiller.ProcessQueries(catalogue);

	int count0;
	filestream >> count0;
	std::getline(filestream, _);

	for (int i = 0; i < count0; ++i) {
		std::string s;
		if (std::getline(filestream, s)) 
		{
			ParseAndProcessInfoQuery(s, catalogue, std::cout);
		}
	}

	//место для точки останова для просмотра данных каталога
}