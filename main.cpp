/*	main.cpp

*******************************************************************/
#include <fstream>
#include <iostream>

#include "bank.h"

using namespace std;
void* atm_routine(void* args);
void* take_fees_routine(void* args);
void* print_accounts_routine(void* args);

bool finished_flag = false;
Bank bank;


int main(int argc, char *argv[]){	

	ofstream clearLogFile("log.txt", std::ios::out);
	clearLogFile << "" << std::endl;
	clearLogFile.close(); // Close the file after truncating it

	// creating print and take-fees threads
	pthread_t print_thread, take_fees_thread;
	pthread_create(&print_thread, NULL, print_accounts_routine, NULL);
	pthread_create(&take_fees_thread, NULL, take_fees_routine, NULL);

	// creating atm threads
	pthread_t* atm_threads = new pthread_t[argc-1];
	vector<pair<string,int>> atm_threads_input(argc-1);
	for(int i=0; i<argc-1; i++){
		atm_threads_input[i] = make_pair(argv[i+1], i+1);
		pthread_create(&atm_threads[i], NULL, atm_routine, (void*)&atm_threads_input[i]);
	}
	// now joining all threads
	for(int i=0; i<argc-1; i++){
		pthread_join(atm_threads[i], NULL);
	}
	finished_flag = true;
	// after we join all threads!!
	pthread_join(print_thread, NULL);
	pthread_join(take_fees_thread, NULL);
	

	logFile.close();
	return 0;
}

void* atm_routine(void* args) {
	pair<string, int>* path_id = reinterpret_cast<pair<string, int>*>(args);
	vector<string> input_lines = convert_file_to_vec(path_id->first);
	for (auto it = input_lines.begin(); it != input_lines.end(); it++) {
		activate_sleep_milli(1);	//sleep for 1 millisecond between two operations
		vector<int> cmd_args;
		char cmd = parse_line(*it, cmd_args);
		bank.exe_command(cmd, cmd_args, path_id->second);
	}
	pthread_exit(NULL);
}

void* take_fees_routine(void* args){
	// needs to add a check that there are still some running atm's
	while(!finished_flag){
		activate_sleep_milli(3000);		//sleep for 3 seconds
		bank.take_fees_account();
	}
	pthread_exit(NULL);
}

void* print_accounts_routine(void* args){
	// needs to add a check that there are still some running atm's
	while(!finished_flag){
		activate_sleep_milli(500);	//sleep for half a second
		bank.print_accounts();
	}
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