/*	main.cpp

*******************************************************************/
#include <fstream>
#include <iostream>

#include "bank.h"

using namespace std;
void* atm_routine(void* args);
void* take_fees_routine(void* args);
void* print_accounts_routine(void* args);

Bank bank;

int main(int argc, char *argv[]){	

	ofstream clearLogFile("log.txt", std::ios::out);
	clearLogFile << "" << std::endl;
	clearLogFile.close(); // Close the file after truncating it
	
	// thread 1
	pthread_t thread_id;
	pair<string, int> path_id = make_pair("atm_in.txt", 0);
	pthread_create(&thread_id, NULL, atm_routine, (void*)&path_id);
	pthread_join(thread_id, NULL);

	// thread 2
	pthread_t thread_id1;
	pair<string, int> path_id1 = make_pair("atm_in1.txt", 1);
	pthread_create(&thread_id1, NULL, atm_routine, (void*)&path_id1);
	pthread_join(thread_id1, NULL);
	
	//thread 3
	pthread_t thread_id2;
	pthread_create(&thread_id2, NULL, take_fees_routine, NULL);
	pthread_join(thread_id2, NULL);

	// thread 4
	pthread_t thread_id3;
	pthread_create(&thread_id3, NULL, print_accounts_routine, NULL);
	pthread_join(thread_id3, NULL);

	logFile.close();
	return 0;
}

void* atm_routine(void* args) {
	pair<string, int>* path_id = reinterpret_cast<pair<string, int>*>(args);
	vector<string> input_lines = convert_file_to_vec(path_id->first);
	for (auto it = input_lines.begin(); it != input_lines.end(); it++) {
		vector<int> cmd_args;
		char cmd = parse_line(*it, cmd_args);
		bank.exe_command(cmd, cmd_args, path_id->second);
	}
	pthread_exit(NULL);
}

void* take_fees_routine(void* args){
	// needs to add a check that there are still some running atm's
	bank.take_fees_account();
	pthread_exit(NULL);
}

void* print_accounts_routine(void* args){
	// needs to add a check that there are still some running atm's
	bank.print_accounts();
	pthread_exit(NULL);
}























































/*
	bank.open_account(10, 1234, 1000, 5);
	bank.open_account(10, 4567, 400, 17);
	bank.open_account(20, 3333, 5000, 5);
	bank.close_account(11, 1234, 11);//not exist
	bank.close_account(10, 2222, 11);//wrong pass
	map<int, Account>::iterator it = bank.accounts.begin();
	while (it != bank.accounts.end()) {
		cout << it->first << "   "<< it->second.password <<"   " << it->second.balance << endl;
		it++;
	}
	cout << endl;
	bank.close_account(10, 1234, 5);
	bank.deposit_account(20,3333,23,7);
	bank.open_account(21,1133,5757,12);
	bank.withdraw_account(21,1133,9000,10);
	bank.withdraw_account(21,1133,2000,19);
	bank.check_balance_account(21,1000,2);
	bank.check_balance_account(23,1000,2);
	bank.check_balance_account(20,3333,2);
	bank.transfer_funds_account(21,1133,20,6000,2);
	it = bank.accounts.begin();
	while (it != bank.accounts.end()) {
		cout << it->first << "   "<< it->second.password <<"   " << it->second.balance << endl;
		it++;
	}

	cout << endl;
	bank.take_fees_account();
	bank.print_accounts();
	*/