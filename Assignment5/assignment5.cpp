#include <bits/stdc++.h>
#include "core.hpp"
using namespace std;

// For getting a reqquest to run in the DRAM, the returned command is non-redundant
tuple <int, int, int, string> getCommand(){
	if (waitingList.empty()) return {-1,-1,-1,""};
	int row = currRow;
	if (waitingList.find(currRow)== waitingList.end()) row = (*waitingList.begin()).first;
	int col = (*waitingList[row].begin()).first;
	int count = waitingList[row][col].front().first;
	string reg = waitingList[row][col].front().second;
	bool check_lw = (!check_number(reg));	// true if "lw" operation
	bool check1 = check_lw && (registerUpdate.find(reg)== registerUpdate.end());	// means load on that register is not required, hence redundant operation
	bool check2 = (check_lw && (!check1)) && (registerUpdate[reg].first != count);	// Update on register is required but not here, hence redundant operation
	if (check_lw && (check1 || check2)){
		waitingList[row][col].pop();
		queueSize--;
		if (waitingList[row][col].empty() ) waitingList[row].erase(col);
		if (waitingList[row].empty()) waitingList.erase(row);
		return getCommand();
	}
	else return {row, col, count, reg};
}

// This should never be passed any redundant instruction
void processCommand(tuple <int, int, int, string> command){
	// As you process, don't forget to remove from the thing from Register Update
	int row = get<0>(command);
	int col = get<1>(command);
	int count = get<2>(command);
	string reg = get<3>(command);
	if (row==-1 && col==-1 && count==-1 && reg=="") return;

	int address = 1024*row + 4*col;

	time_req+=clockCycles;

	if (currRow!= row){
		if (currRow != -1){
			DRAM[currRow]  = buffer;
			time_req+= row_access_delay;
		}
		currRow = row;
		buffer = DRAM[currRow];
		row_buffer_updates++;
		time_req+= row_access_delay;
	}

	if (check_number(reg)){
		// Means this was a sw operation, and we are not ignoring any sw operation
		buffer[col] = stoi(reg);
		row_buffer_updates++;
		time_req+= col_access_delay;
		store = {"sw",to_string(clockCycles),to_string(address) + "-" + to_string(address+3),to_string(stoi(reg)), count };
	}
	else{
		// means this is a lw operation
		registers[reg] = buffer[col];
		registerUpdate.erase(reg);
		time_req+= col_access_delay;
		store  = {"lw", to_string(clockCycles), reg, to_string(buffer[col]), count};
	}
	waitingList[row][col].pop();
	queueSize--;
	if (waitingList[row][col].empty() ) waitingList[row].erase(col);
	if (waitingList[row].empty()) waitingList.erase(row);
}

// This marks the end of a process running in the DRAM 
void processCompletion(){
	int count = get<4>(store);
	string reg = get<2>(store);
	cout << to_string(insCounter[count-1]+1) +" => " +instructions[insCounter[count-1]]+"\n";
	cout << "cycle " + get<1>(store) + "-" + to_string(time_req)+": ";
	if (get<0>(store)=="lw"){
		if (forRefusing[reg] > count) cout << "Rejected by processor, "<< reg<< " already updated in procesor\n\n";
		else  cout << reg + "= " + get<3>(store) + "\n\n";
	}
	else{
		cout << "Memory address " + reg + "= " + get<3>(store) + "\n\n";
	}
	time_req =-1;
	store = tuple <string ,string, string, string, int>();
}

// Whenever we encounter an unsafe instruction, we use this function to complete the value of register then and there.
void completeRegister(string name){
	if (registerUpdate.find(name)== registerUpdate.end()){
		if (get<2>(store) == name){
			// Means we were currently processing that required register only
			clockCycles = time_req+1;
			processCompletion();
		}
		return;
	}
	// else we have unfinished business

	if (time_req != -1){	// Need to complete the ongoing work forcefully
		clockCycles = time_req+1;
		processCompletion();
	}

	// Complete that [row][col]
	int count_req = registerUpdate[name].first;
	int address  = registerUpdate[name].second;
	int row = address/1024;
	int col = (address%1024)/4;

	pair<int, string> p;
	string reg="";
	int count=-1;
	while(true){
		p = waitingList[row][col].front();
		count = p.first;
		reg = p.second;
		bool check_lw = (!check_number(reg));	// true if "lw" operation
		bool check1 = check_lw &&  (registerUpdate.find(reg)== registerUpdate.end());	// means load on that register is not required, hence redundant operation
		bool check2 = (check_lw && (!check1)) && (registerUpdate[reg].first != count);	// Update on register is required but not here, hence redundant operation
		if (check_lw && (check1 || check2)){
			waitingList[row][col].pop();
			queueSize--;
			if (waitingList[row][col].empty() ) waitingList[row].erase(col);
			if (waitingList[row].empty()) waitingList.erase(row);
			continue;
		}
		processCommand({row,col,count, reg});
		clockCycles = time_req+1;
		processCompletion();

		if (p.first == count_req) break;
	}
}

// Parser executes the required functions
void parser(vector<string> tokens){
	insCounter.push_back(itr);
	counter++;
	int m=tokens.size();
	string s0=tokens[0];
	if(m>4){
		cout<<"Syntax Error on line "<<(++itr)<<endl;
		throwError =1;
		return;
	}
	if (m==1){
		itr++;
		return;
	}
	if(operations.find(s0)==operations.end()){
		cout<<"Invalid instruction on line "<<(++itr)<<endl;
		throwError=1;
		return;
	}

	if(m==2){
		string s1=tokens[1];
		if (s0!="j"){
			cout <<"Invalid instruction on line "<<(++itr)<<endl;
			throwError =1;
			return;
		}
		if(labels.find(s1)==labels.end()){
			cout<<"Invalid label on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
		itr=labels[s1];
		cout << itr+1<<" => "<<instructions[itr]<<"\n";
		cout <<"cycle "<<clockCycles<<": Jumping to "<<s1<<"\n\n";
		if (time_req == clockCycles){
			processCompletion();
		}
		clockCycles++;
		if (time_req ==-1 && !waitingList.empty() ){
			processCommand(getCommand());
		}
	}
	else if(m==3){
		if (s0 != "lw" && s0!="sw"){
			cout<<"Invalid instruction on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
		string s1=tokens[1];
		string s2=tokens[2];
		if(registers.find(s1)==registers.end()){
			cout<<"Invalid register on line "<<(++itr)<<"\n";
			throwError=1;
			return;
		}
		if (s1== "$zero" && s0!="sw"){
			cout << "Value of $zero cannot be changed on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
		if(!checkAddress(s2)){
			cout<<"Invalid format of memory address on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
		int clock_initial = clockCycles;
		completeRegister(extract_reg(s2));
		int address=locateAddress(s2);
		if (address==-2){
			cout <<"Only 2^20 Bytes of memory could be used on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
		if(address<0){
			cout<<"Unaligned memory address on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
		int row = address/1024;
		int col = (address%1024)/4;

		if (s0 =="sw") completeRegister(s1);
		if (clock_initial!= clockCycles){	// Means we stopped from completing some register
			if (time_req ==-1 && !waitingList.empty() ){
				processCommand(getCommand());
			}
		}
		// Incase the lw gets handled by forwarding
		if (s0=="lw" && address == last_updated_address){
			if(time_req==clockCycles){
				if (get<0>(store) == "lw") clockCycles++;
				processCompletion();
				if(!waitingList.empty()){
					processCommand(getCommand());
				}
			}
			registers[s1] = last_stored_value;
			forRefusing[s1] = counter;
			registerUpdate.erase(s1);
			cout << itr+1<<" => "<<instructions[itr]<<"\n";
			cout << "cycle "<< clockCycles << ": "<<s1<<"= "<<last_stored_value<<" | Due to forwarding"<<"\n\n";
			clockCycles++;
		}
		// Normal lw and sw instruction 
		else{
			if (queueSize == MAX_SIZE){
				// We have to process one more command from waiting list, we don't have to finsih it, just need to start it's execution 
				if (time_req != -1){	// Need to complete the ongoing work forcefully
					clockCycles = time_req+1;
					processCompletion();
				}
				processCommand(getCommand());
			}
			if (queueSize>=MAX_SIZE){
				cout <<"No space in waiting list\n";
				throwError=1;
				return ;
			}
			cout << itr+1<<" => "<<instructions[itr]<<"\n";
			cout << "cycle "<< clockCycles << ": DRAM request issued"<<"\n\n";
			if (time_req == clockCycles){
				processCompletion();
			}
			clockCycles++;
			queueSize++;
			if(s0=="lw"){
				waitingList[row][col].push({counter, s1});
				registerUpdate[s1] = {counter, address};
			}
			else if(s0=="sw"){
				waitingList[row][col].push({counter, to_string(registers[s1])});
				last_updated_address = address;
				last_stored_value = registers[s1];
			}
		}
		if (time_req ==-1 && !waitingList.empty() ){
			processCommand(getCommand());
		}
	}
	else if(m==4){
		string s1=tokens[1];
		string s2=tokens[2];
		string s3=tokens[3];
		if (registers.find(s1)==registers.end() || registers.find(s2)==registers.end()){
			cout<<"Invalid register on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
		if(s0=="beq" || s0=="bne"){
			if(labels.find(s3)==labels.end()){
				cout<<"Invalid label on line "<<(++itr)<<endl;
				throwError=1;
				return;
			}
			int clock_initial = clockCycles;
			completeRegister(s1);
			completeRegister(s2);
			if (clock_initial!= clockCycles){
				if (time_req ==-1 && !waitingList.empty() ){
					processCommand(getCommand());
				}
			}
			int toJump = 0;
			if (s0 == "beq" && registers[s1]==registers[s2]){
				toJump = 1;
			}
			else if (s0 == "bne" && registers[s1]!=registers[s2]){
				toJump = 1;
			}
			cout << itr+1<<" => "<<instructions[itr]<<"\n";
			if(toJump==1){
				itr=labels[s3];
				cout <<"cycle "<<clockCycles<<": Jumping to "<<s3<<"\n\n";
			}
			else{
				cout <<"cycle "<<clockCycles<<": No jump required to "<< s3 <<"\n\n";
			}
			if (time_req == clockCycles){
				processCompletion();
			}
			clockCycles++;
			if (time_req ==-1 && !waitingList.empty() ){
				processCommand(getCommand());
			}
		}
		else if (s0=="add" || s0=="sub" || s0=="mul" || s0 == "slt" || s0=="addi"){

			if (s1== "$zero"){
				cout << "Value of $zero cannot be changed on line "<<(++itr)<<endl;
				throwError=1;
				return;
			}
			if(s0!= "addi" && registers.find(s3)==registers.end()){
				cout<<"Invalid register on line "<<(++itr)<<endl;
				throwError=1;
				return;
			}
			if(s0=="addi" && !check_number(s3)){
				cout<<"Immediate value is not an integer on line "<<(++itr)<<endl;
				throwError=1;
				return;
			}
			
			int clock_initial = clockCycles;
			completeRegister(s2);
			if (s0 != "addi") completeRegister(s3);
			if (clock_initial!= clockCycles){
				if (time_req ==-1 && !waitingList.empty() ){
					processCommand(getCommand());
				}
			}

			if (s0 == "add") registers[s1]=registers[s2]+registers[s3];
			else if (s0=="sub") registers[s1]=registers[s2]-registers[s3];
			else if (s0 == "mul") registers[s1]=registers[s2]*registers[s3];
			else if (s0 == "slt") registers[s1]= (registers[s2]<registers[s3]);
			else if (s0=="addi") registers[s1]=registers[s2]+stoi(s3);

			if(time_req==clockCycles){
				if (get<0>(store) == "lw") clockCycles++;
				processCompletion();
				if(!waitingList.empty()){
					processCommand(getCommand());
				}
			}

			forRefusing[s1] = counter;
			registerUpdate.erase(s1);	
			cout << itr+1<<" => "<<instructions[itr]<<"\n";
			cout <<"cycle "<<clockCycles<<": "<<s1<<"= "<<registers[s1]<<"\n\n";
			clockCycles++;
			if (time_req ==-1 && !waitingList.empty() ){
				processCommand(getCommand());
			}
		}
		
		else{
			cout<<"Invalid instruction on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
	}
	itr++;
	if(m!=1){
		operations[s0]++;
	}
	
}

int main(int argc, char** argv){

	initialize(argc, argv);
	if(throwError==1){
		return 0;
	}

	// Start running the file
	cout << "Every cycle description\n\n";
	while(cores[currentCore]->itr< cores[currentCore]->instructions.size() ){
		string currentLine = cores[currentCore]->instructions[cores[currentCore]->itr];
		vector<string> strings;

		strings=lexer(currentLine);
		parser(strings);
		if(throwError==1){
			return 0;
		}
	}

	// Complete any remaining requests in the DRAM
	if (time_req != -1){
		clockCycles = time_req+1;
		processCompletion();
	}
	while (!waitingList.empty()){
		processCommand(getCommand());
		if(time_req==(-1)) break;
		clockCycles = time_req+1;
		processCompletion();
	}

	if (currRow!=-1) DRAM[currRow] = buffer;

	cores[currentCore]->print_stats();

	return 0;
}

/*********************** Function defintions *********************/

// To initialize data according to N cores.
void initialize(int argc, char** argv)
{
	if(!check_number(argv[1])){
		"Invalid value for number of cores.\n";
		throwError = 1;
		return;
	}
	int N = stoi(argv[1]);

	if (argc == N+4){
		if(!check_number(argv[N+2]) || !check_number(argv[N+3])){
			cout<<"Invalid value of delays.\n";
			throwError = 1;
			return;
		}
		row_access_delay = stoi(argv[N+2]);
		col_access_delay = stoi(argv[N+3]);
	}else{
		cout<<"Provide appropriate number of arguments.\n";
		throwError=1;
		return;
	}

	for(int i=2;i<N+2;i++){
		// Reading command line arguments
		string fileName = argv[i];
		cores.push_back(new Core(fileName));
	}

}

// To check if a string denotes a integer or not
bool check_number(string str)
{
	if (str.length() == 0)
		return true;
	if (!isdigit(str[0]))
	{
		if (str[0] != '-' && str[0] != '+')
		{
			return false;
		}
	}
	for (int i = 1; i < str.length(); i++)
		if (isdigit(str[i]) == false)
			return false;
	return true;
}

// To extract a potentially unsafe register from the address in an lw instruction
string extract_reg(string reg)
{
	int n = reg.length();
	string second = reg.substr(n - 4, 3);
	if (n >= 7 && reg.substr(n - 7) == "($zero)")
	{
		second = reg.substr(n - 6, 5);
	}
	else if (reg[n - 4] == '(' && reg[n - 1] == ')')
	{
		second = reg.substr(n - 3, 2);
	}
	return second;
}

// Lexer splits the string into tokens such that first 2 are space delimited in the input string and the remaining are space or tab delimited
vector<string> lexer(string line)
{
	int n = line.length();
	vector<string> v;
	bool first = false;
	bool second = false;
	string s = "";
	int i = 0;
	while (i < n)
	{
		if (first)
		{
			if (second)
			{
				if (line[i] == ',')
				{
					v.push_back(s);
					s = "";
					i++;
					while (line[i] == ' ' || line[i] == '\t')
					{
						if (i < n)
						{
							i++;
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					if (line[i] != ' ' && line[i] != '\t')
						s += line[i];
					i++;
				}
			}
			else
			{
				if (line[i] == ' ' || line[i] == '\t')
				{
					second = true;
					v.push_back(s);
					s = "";
					while (line[i] == ' ' || line[i] == '\t')
					{
						if (i < n)
						{
							i++;
						}
						else
						{
							break;
						}
					}
				}
				else
				{
					s += line[i];
					i++;
				}
			}
		}
		else
		{
			if (line[i] != ' ' && line[i] != '\t')
			{
				first = true;
				s += line[i];
			}
			i++;
		}
	}
	if (s != "")
		v.push_back(s);
	return v;
}

// To check whether something is a valid memory
bool checkAddress(string reg)
{
    int n = reg.length();
    if (check_number(reg))
        return false;

    if (n >= 7 && reg.substr(n - 7) == "($zero)")
    {
        if (!check_number(reg.substr(0, n - 7)))
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    if (n < 4)
        return false;

    if (reg[n - 4] == '(' && reg[n - 1] == ')')
    {
        string s = reg.substr(n - 3, 2);
        if (!check_number(reg.substr(0, n - 4)))
        {
            return false;
        }
        if (registers.find(s) != registers.end())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        if (n < 5)
            return false;
        if (reg[n - 5] == '(' && reg[n - 1] == ')')
        {
            string s = reg.substr(n - 4, 3);
            if (!check_number(reg.substr(0, n - 5)))
            {
                return false;
            }
            if (registers.find(s) != registers.end())
            {
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }
}

// To get the memory address from a string
int locateAddress(string reg)
{
    int addr;
    int n = reg.length();
    if (check_number(reg))
    {
        addr = stoi(reg);
    }
    else
    {
        int num = 0;
        string first = reg.substr(0, n - 5);
        string second = reg.substr(n - 4, 3);
        if (n >= 7 && reg.substr(n - 7) == "($zero)")
        {
            first = reg.substr(0, n - 7);
            second = reg.substr(n - 6, 5);
        }
        else if (reg[n - 4] == '(' && reg[n - 1] == ')')
        {
            first = reg.substr(0, n - 4);
            second = reg.substr(n - 3, 2);
        }
        if (first != "")
        {
            num = stoi(first);
        }
        addr = (num + registers[second]);
    }
    if (addr % 4 != 0)
    {
        // because we are handling just lw and sw, the address must be a multiple of 4, in lb it could be anything
        addr = -1;
    }
    if (addr >= (1 << 20))
    {
        // Memory out of limits
        addr = -2;
    }
    return addr;
}
