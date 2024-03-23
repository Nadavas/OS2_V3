#ifndef _BANK_H
#define _BANK_H
#include <unistd.h> 
#include <stdio.h>
#include <cstdio>
#include <cerrno>
#include <time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string>
#include <cstring>
#include <map>
#include <algorithm>    
#include <iostream>
#include <pthread.h>
#include <fstream>
#include <random>
#include <cmath>
#include <vector>
#include <sstream>

using namespace std;

extern fstream logFile;
//extern pthread_mutex_t logFile_mutex;

// Readers & Writers methods
void readers_lock(pthread_mutex_t* read_mutex, int &counter, pthread_mutex_t* write_mutex);
void readers_unlock(pthread_mutex_t* read_mutex, int &counter, pthread_mutex_t* write_mutex);
void writers_lock(pthread_mutex_t* write_mutex);
void writers_unlock(pthread_mutex_t* write_mutex);

class Account {
public:
	int password;
	int balance;
	int acc_num_readers;
	pthread_mutex_t acc_read_mutex;
	pthread_mutex_t acc_write_mutex;
	
	//constructors and destructor
	Account();
	Account(int password, int balance, int num_readers = 0);
	~Account();

	//methods

};

class Bank {
public:
	int private_balance;
	map<int, Account> accounts;
	int bank_num_readers;
	pthread_mutex_t bank_read_mutex;	
	pthread_mutex_t bank_write_mutex;

	//constructors and destructor
	Bank();
	~Bank();
	
	//methods
	void open_account(int acc_num, int password, int initial_balance, int atm_id);
	void close_account(int acc_num, int password, int atm_id);
	void deposit_account(int acc_num, int password, int amount, int atm_id);
	void withdraw_account(int acc_num, int password, int amount, int atm_id);
	void take_fees_account();
	void print_accounts();
	void exe_command(char cmd_type, vector<int> cmd_args, int atm_id);

};


//help functions
	int randomize_fee();
	char parse_line(const std::string& input_line, std::vector<int>& args);
	vector<string> convert_file_to_vec(const string& filePath);
	int countWords(const std::string& str);
#endif

