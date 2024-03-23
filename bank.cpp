// bank.cpp

/*******************************************/
#include "bank.h"


using namespace std;
//creating the log file with read and append permissions
fstream logFile("log.txt", ios::in | ios::app);
//creating the mutex for the log file
//pthread_mutex_t logFile_mutex;


/************************************************/
/****************WRITERS READERS*****************/
/************************************************/

//// ALL WRITERS READERS METHODS ////
//// Implemented with mutex and a counter //
//// so it's basically a semaphore /////////
void readers_lock(pthread_mutex_t* read_mutex, int &counter, pthread_mutex_t* write_mutex){
    if(pthread_mutex_lock(read_mutex)){
        perror("Bank error : pthread_mutex_lock failed");
        logFile.close();
        exit(0);
    }
    counter++;
    if(counter == 1)
        writers_lock(write_mutex);
    if(pthread_mutex_unlock(read_mutex)){
        perror("Bank error : pthread_mutex_unlock failed");
        logFile.close();
        exit(0);
    }
}

void readers_unlock(pthread_mutex_t* read_mutex, int &counter, pthread_mutex_t* write_mutex){
    if(pthread_mutex_lock(read_mutex)){
        perror("Bank error : pthread_mutex_lock failed");
        logFile.close();
        exit(0);
    }
    counter--;
    if(counter == 0)
       writers_unlock(write_mutex);
    if(pthread_mutex_unlock(read_mutex)){
        perror("Bank error : pthread_mutex_unlock failed");
        logFile.close();
        exit(0);
    }
}

void writers_lock(pthread_mutex_t* write_mutex){
   if(pthread_mutex_lock(write_mutex)){
        perror("Bank error : pthread_mutex_lock failed");
        logFile.close();
        exit(0);
    } 
}

void writers_unlock(pthread_mutex_t* write_mutex){
   if(pthread_mutex_unlock(write_mutex)){
        perror("Bank error : pthread_mutex_unlock failed");
        logFile.close();
        exit(0);
    } 
}


/************************************************/
/*******************Bank*************************/
/************************************************/

Bank::Bank() {
    private_balance = 0;
    bank_num_readers = 0;
    if (pthread_mutex_init(&bank_read_mutex, NULL) != 0) {
        perror("Bank error : pthread_mutex_init failed");
        exit(0);
    }
    if (pthread_mutex_init(&bank_write_mutex, NULL) != 0) {
        perror("Bank error : pthread_mutex_init failed");
        exit(0);
    }
}

Bank::~Bank() {
    if (pthread_mutex_destroy(&bank_read_mutex) != 0) {
        perror("Bank error : pthread_mutex_destroy failed");
        exit(0);
    }
    int err = pthread_mutex_destroy(&bank_write_mutex);
    if (err != 0) {
        cout << "the error is :" << err << endl;
        perror("Bank error : pthread_mutex_destroy failed");
        exit(0);
    }
    
}

void Bank::open_account(int acc_num, int password, int initial_balance, int atm_id) {
    writers_lock(&bank_write_mutex);     // Lock the Bank for Writing
    activate_sleep_milli(1000);   //sleep for 1 second
    if (accounts.find(acc_num) != accounts.end()) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - account with the same id exists" << endl;
        //unlock log
        writers_unlock(&bank_write_mutex);   // UnLock the Bank for Writing
        return;
    }
    Account new_acc = Account(password, initial_balance);
    accounts[acc_num] = new_acc;

    //lock log
    logFile << atm_id << ": New account id is " << acc_num << " with password "
        << password <<" and initial balance "<< initial_balance << endl;
    //unlock log
    writers_unlock(&bank_write_mutex);   // UnLock the Bank for Writing
}

void Bank::close_account(int acc_num, int password, int atm_id) {
    writers_lock(&bank_write_mutex);     // Lock the Bank for Writing
    activate_sleep_milli(1000);   //sleep for 1 second
    map<int, Account>::iterator it = accounts.find(acc_num);
    if (it == accounts.end()) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - account id " << acc_num << " does not exist" << endl;
        //unlock log
        writers_unlock(&bank_write_mutex);   // UnLock the Bank for Writing
        return;
    }
    if (it->second.password != password) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - password for account id " << acc_num << " is incorrect" << endl;
        //unlock log
        writers_unlock(&bank_write_mutex);   // UnLock the Bank for Writing
        return;
    }
    int balance = accounts[acc_num].balance;
    accounts.erase(acc_num);

    //lock log
    logFile << atm_id << ": Account " << acc_num << " is now closed. Balance was " << balance << endl;
    // unlock log
    writers_unlock(&bank_write_mutex);   // UnLock the Bank for Writing
}

void Bank::deposit_account(int acc_num, int password, int amount, int atm_id){
    activate_sleep_milli(1000);   //sleep for 1 second
    readers_lock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);    // Lock the Bank for Reading 
    map<int, Account>::iterator it = accounts.find(acc_num);
    if (it == accounts.end()) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - account id " << acc_num << " does not exist" << endl;
        //unlock log
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);  // UnLock the Bank for Reading
        return;
    }
    if (it->second.password != password) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - password for account id " << acc_num << " is incorrect" << endl;
        //unlock log
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);  // UnLock the Bank for Reading
        return;
    }
    writers_lock(&(it->second.acc_write_mutex));      //lock specific account balance mutex - WRITERv
    it->second.balance += amount;
    //lock log
    logFile << atm_id << ": Account " << acc_num << " new balance is " << it->second.balance 
    << " after " << amount << " $ was deposited" <<endl;
    // unlock log
    writers_unlock(&(it->second.acc_write_mutex));     //unlock specific account balance mutex - WRITER
    readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
}

void Bank::withdraw_account(int acc_num, int password, int amount, int atm_id){    
    readers_lock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);    // Lock the Bank for Reading
    activate_sleep_milli(1000);   //sleep for 1 second
    map<int, Account>::iterator it = accounts.find(acc_num);
    if (it == accounts.end()) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - account id " << acc_num << " does not exist" << endl;
        //unlock log
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
        return;
    }
    if (it->second.password != password) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - password for account id " << acc_num << " is incorrect" << endl;
        //unlock log
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
        return;
    }
    
    writers_lock(&(it->second.acc_write_mutex));      //lock specific account balance mutex - WRITERS
    if(amount > it->second.balance){
        // lock log
        logFile << "Error " << atm_id << ": Your transaction failed - account id " << acc_num 
        << " balance is lower than " << amount << endl;
        // unlock log
    writers_unlock(&(it->second.acc_write_mutex));     //unlock specific account balance mutex - WRITER   - to check
    readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
        return;
    }
    it->second.balance -= amount;
    //lock log
    logFile << atm_id << ": Account " << acc_num << " new balance is " << it->second.balance 
    << " after " << amount << " $ was withdrew" <<endl;
    // unlock log
    writers_unlock(&(it->second.acc_write_mutex));     //unlock specific account balance mutex - WRITER   - to check
    readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
}

void Bank::check_balance_account(int acc_num, int password, int atm_id){
    readers_lock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);    // Lock the Bank for Reading
    activate_sleep_milli(1000);   //sleep for 1 second
    map<int, Account>::iterator it = accounts.find(acc_num);
    if (it == accounts.end()) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - account id " << acc_num << " does not exist" << endl;
        //unlock log
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
        return;
    }
    if (it->second.password != password) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - password for account id " << acc_num << " is incorrect" << endl;
        //unlock log
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
        return;
    }
    readers_lock(&(it->second.acc_read_mutex), it->second.acc_num_readers, &(it->second.acc_write_mutex));     //lock specific account balance mutex - READER  
    // lock log
    logFile << atm_id << ": Account " << acc_num << " balance is " << it->second.balance << endl; 
    // unlock log
    readers_unlock(&(it->second.acc_read_mutex), it->second.acc_num_readers, &(it->second.acc_write_mutex)); //unlock specific account balance mutex - READER
    readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
}

void Bank::transfer_funds_account(int src_acc_num, int src_acc_password, int trg_acc_num, int amount, int atm_id){
    readers_lock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);    // Lock the Bank for Reading
    activate_sleep_milli(1000);   //sleep for 1 second
    map<int, Account>::iterator it_src = accounts.find(src_acc_num);
    if (it_src == accounts.end()) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - account id " << src_acc_num << " does not exist" << endl;
        //unlock log
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
        return;
    }
    map<int, Account>::iterator it_trg = accounts.find(trg_acc_num);
    if (it_trg == accounts.end()) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - account id " << trg_acc_num << " does not exist" << endl;
        //unlock log
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
        return;
    }
    if (it_src->second.password != src_acc_password) {
        //lock log
        logFile << "Error " << atm_id << ": Your transaction failed - password for account id " << src_acc_num << " is incorrect" << endl;
        //unlock log
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
        return;
    }
    // Lock the accounts in lex order
    if (src_acc_num < trg_acc_num){
        //  LOCK SRC THAN TRG - WRITER  - to check if readers lock is needed
        writers_lock(&(it_src->second.acc_write_mutex));
        writers_lock(&(it_trg->second.acc_write_mutex));
    }
    else{
        // LOCK TRG THAN SRC  -WRITER
        writers_lock(&(it_trg->second.acc_write_mutex));
        writers_lock(&(it_src->second.acc_write_mutex));
    }
    if(amount > it_src->second.balance){
        // lock log
        logFile << "Error " << atm_id << ": Your transaction failed - account id " << src_acc_num 
        << " balance is lower than " << amount << endl;
        // unlock log
        // unlock in reverse order
        if (src_acc_num < trg_acc_num){
            //  UnLOCK TRG THAN SRC - WRITER  
            writers_unlock(&(it_trg->second.acc_write_mutex));
            writers_unlock(&(it_src->second.acc_write_mutex));
        }
        else{
            //  UnLOCK SRC THAN TRG - WRITER 
            writers_unlock(&(it_src->second.acc_write_mutex));
            writers_unlock(&(it_trg->second.acc_write_mutex));
        }
        readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
        return;
    }
    it_src->second.balance -= amount;
    it_trg->second.balance += amount;
    // lock log
    logFile << atm_id << ": Transfer "<< amount << " from account " << src_acc_num 
    << " to account " << trg_acc_num << " new account balance is " << it_src->second.balance 
    << " new target account balance is " << it_trg->second.balance << endl; 
    // unlock log
    // unlock in reverse order
    if (src_acc_num < trg_acc_num){
        //  UnLOCK TRG THAN SRC - WRITER  
        writers_unlock(&(it_trg->second.acc_write_mutex));
        writers_unlock(&(it_src->second.acc_write_mutex));
    }
    else{
        //  UnLOCK SRC THAN TRG - WRITER 
        writers_unlock(&(it_src->second.acc_write_mutex));
        writers_unlock(&(it_trg->second.acc_write_mutex));
    }
    readers_unlock(&bank_read_mutex,bank_num_readers,&bank_write_mutex);       // UnLock the Bank for Reading
}

void Bank::take_fees_account(){
    int total_fees = 0;
    int int_fee = randomize_fee();
    double fee_rate = ((double)int_fee)/100;
    writers_lock(&bank_write_mutex);     // lock bank writer 
    map<int, Account>::iterator it = accounts.begin();
    while(it!=accounts.end()){
        writers_lock(&(it->second.acc_write_mutex));       // Lock specific account - writer
        int acc_fee = round((it->second.balance)*fee_rate);     
        it->second.balance -= acc_fee;  // deduct a fee from account
        total_fees += acc_fee;
        // lock log
        logFile << "Bank: commissions of " << int_fee << " % were charged, the bank gained "
        << acc_fee << " $ from account " << it->first << endl;
        // unlock log
        writers_unlock(&(it->second.acc_write_mutex));     // UnLock specific account - writer
        it++;
    }
    // Now add the fees to the bank manager
    private_balance += total_fees;
    writers_unlock(&bank_write_mutex);               // Unlock bank writer
}

void Bank::print_accounts(){
    writers_lock(&bank_write_mutex);     // lock bank writer 
    map<int, Account>::iterator it = accounts.begin();
    printf("\033[2J");
    printf("\033[1;1H");
    cout << "Current Bank Status" << endl;
    while(it!=accounts.end()){
        readers_lock(&(it->second.acc_read_mutex), it->second.acc_num_readers, &(it->second.acc_write_mutex));    //lock the readers for the specific account
        cout << "Account " << it->first << ": Balance - " << it->second.balance
        <<" $, Account Password - " << it->second.password << endl;
        readers_unlock(&(it->second.acc_read_mutex), it->second.acc_num_readers, &(it->second.acc_write_mutex));    //unlock the readers for the specific account
        it++;
    }
    cout << "The Bank has " << private_balance << " $" << endl;
    writers_unlock(&bank_write_mutex);     // unlock bank writer
}

void Bank::exe_command(char cmd_type, vector<int> cmd_args, int atm_id){
    switch(cmd_type){
        case 'O': 
            open_account(cmd_args[0], cmd_args[1], cmd_args[2], atm_id);
            break;
        
        case 'Q': 
            close_account(cmd_args[0], cmd_args[1], atm_id);
            break;
        
        case 'D': 
            deposit_account(cmd_args[0], cmd_args[1], cmd_args[2], atm_id);
            break;

        case 'W': 
            withdraw_account(cmd_args[0], cmd_args[1], cmd_args[2], atm_id);
            break;

        case 'B': 
            check_balance_account(cmd_args[0], cmd_args[1], atm_id);
            break;  
            
        case 'T': 
            transfer_funds_account(cmd_args[0], cmd_args[1], cmd_args[2], cmd_args[3], atm_id);
            break;
            
        default: 
            break;
    }
    
}
/************************************************/
/*******************Account**********************/
/************************************************/


Account::Account() {
    if (pthread_mutex_init(&acc_read_mutex, NULL) != 0) {
        perror("Bank error : pthread_mutex_init failed");
        exit(0);
    }
    if (pthread_mutex_init(&acc_write_mutex, NULL) != 0) {
        perror("Bank error : pthread_mutex_init failed");
        exit(0);
    }
}	

Account::Account(int password, int balance, int acc_num_readers)
    : password(password), balance(balance), acc_num_readers(acc_num_readers) {
    if (pthread_mutex_init(&acc_read_mutex, NULL) != 0) {
        perror("Bank error : pthread_mutex_init failed");
        exit(0);
    }
    if (pthread_mutex_init(&acc_write_mutex, NULL) != 0) {
        perror("Bank error : pthread_mutex_init failed");
        exit(0);
    }
}

Account::~Account() {
    if (pthread_mutex_destroy(&acc_read_mutex) != 0) {
        perror("Bank error : pthread_mutex_destroy failed acceread");
        exit(0);
    }
    if (pthread_mutex_destroy(&acc_write_mutex) != 0) {
        perror("Bank error : pthread_mutex_destroy failed accewrite");
        exit(0);
    }
}

/************************************************/
/****************help functions******************/
/************************************************/

int randomize_fee(){
    // Initialize random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 5); // Uniform distribution between 1 and 5

    // Generate and return a random integer
    return dis(gen);
}

char parse_line(const std::string& input_line, std::vector<int>& args) {
    
    char first_char = input_line[0];
    int count = 0;
    int num_args = countWords(input_line);
    //cout << "number of words in " << input_line << " is " << num_args << endl; 
    //cout << count << ": " << first_char << endl;
    std::istringstream iss(input_line);
    std::string token;
    // Skip the first token
    iss >> token;
    // Process subsequent tokens
    int tracker = 0;
 
    while (tracker<num_args-1) {
        iss >> token;
        tracker++;
        count++;
        //cout << count << ": " << token << endl;
        args.push_back(std::stoi(token));
    }
    return first_char;
   
}

vector<string> convert_file_to_vec(const string& filePath){
    vector<string> lines;
    ifstream inputFile(filePath);
    
    if (!inputFile.is_open()) {
        cerr << "Bank error: illegal arguments " << endl;
        return lines;
    }
    string line;
    while (getline(inputFile, line)) {
        lines.push_back(line);
    }
    inputFile.close();
    return lines;
}

int countWords(const std::string& str) {
    std::stringstream ss(str);
    std::string word;
    int count = 0;
    
    // Count words using stringstream
    while (ss >> word) {
        count++;
    }

    return count;
}

void activate_sleep_milli(int millis) {
    std::this_thread::sleep_for(std::chrono::milliseconds(millis));
}