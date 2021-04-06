#include<bits/stdc++.h>
using namespace std;

unordered_map<string,int> registers;
unordered_map<string,int> operations;
unordered_map<string,int> labels;
vector <string> regPrint;
vector<string> instructions;
int itr = 0;
int counter =0;
int throwError = 0;
int clockCycles=1;
int row_buffer_updates=1;

vector <vector<int>> DRAM(1024,vector<int>(256,0));	// becuase every column in itself represents 4 Bytes so the column size is only 256
vector <int> buffer(256,0);
int currRow=-1;
int row_access_delay=0;
int col_access_delay=0;

map <int, set<tuple<int, string, string, int>>>waitingList;
map<string, int> registerUpdate;
map<int, int> memoryUpdate;
map<int, int> counterLocation;

// Helper functions:-
bool check_number(string str);
bool checkAddress(string reg);
int locateAddress(string reg);
void fillRegs();
void fillOpers();
vector<string> lexer(string line);
void print_stats();

// Parser executes the required functions
void parser(vector<string> tokens){
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
		cout<<"Invalid Instruction on line "<<(++itr)<<endl;
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
		if(labels.find(s1)!=labels.end()){
			itr=labels[s1];
			cout <<"cycle "<<clockCycles++<<": Jumping to "<<s1<<"\n";
		}
		else{
			cout<<"Such a label doesn't exist on line "<<(++itr)<<endl;
			throwError=1;
			return;
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
			cout<<"Invalid register\n";
			throwError=1;
			return;
		}
		if (s1== "$zero" && s0!="sw"){
			cout << "value of $zero cannot be changed on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
		if(!checkAddress(s2)){
			cout<<"Invalid format of memory address. on line "<<(++itr)<<endl;
			throwError=1;
			return;
		}
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

		if(s0=="lw"){
			cout << "cycle "<< clockCycles++ << ": DRAM request issued\n";

			if (row==currRow){
				registers[s1] = buffer[col];
				cout <<"cycle "<< clockCycles <<"-"<<clockCycles+col_access_delay-1<<": "<< s1<<"= "<<buffer[col]<<"\n";
				clockCycles+= col_access_delay;
			}
			else{
				if (currRow==-1){
					currRow =row;
					buffer = DRAM[row];
					registers[s1] = buffer[col];
					cout <<"cycle "<< clockCycles <<"-"<<clockCycles+row_access_delay+col_access_delay-1<<": "<< s1<<"= "<<buffer[col]<<"\n";
					clockCycles+= (row_access_delay+col_access_delay);
				}
				else{
					row_buffer_updates++;
					DRAM[currRow] = buffer;
					currRow =row;
					buffer = DRAM[row];
					registers[s1] = buffer[col];
					cout <<"cycle "<< clockCycles <<"-"<<clockCycles+2*row_access_delay+col_access_delay-1<<": "<< s1<<"= "<<buffer[col]<<"\n";
					clockCycles+= (2*row_access_delay+col_access_delay);
				}
			}
			waitingList[row].insert({counter, s0, s1, address});
			registerUpdate[s1] = counter;
			counterLocation[counter] = row;
		}
		else if(s0=="sw"){
			cout << "cycle "<< clockCycles++ << ": DRAM request issued\n";
			row_buffer_updates++;
			if (row==currRow){
				buffer[col] = registers[s1];
				cout <<"cycle "<< clockCycles <<"-"<<clockCycles+col_access_delay-1<<": Memory address "<<address<<"-"<<address+3<<"= "<<registers[s1]<<"\n";
				clockCycles+= col_access_delay;
			}
			else{
				if (currRow==-1){
					currRow =row;
					buffer = DRAM[row];
					buffer[col] = registers[s1];
					cout <<"cycle "<< clockCycles <<"-"<<clockCycles+row_access_delay+col_access_delay-1<<": Memory address "<<address<<"-"<<address+3<<"= "<<registers[s1]<<"\n";
					clockCycles+= (row_access_delay+col_access_delay);
				}
				else{
					row_buffer_updates++;
					DRAM[currRow] = buffer;
					currRow =row;
					buffer = DRAM[row];
					buffer[col] = registers[s1];
					cout <<"cycle "<< clockCycles <<"-"<<clockCycles+2*row_access_delay+col_access_delay-1<<": Memory address "<<address<<"-"<<address+3<<"= "<<registers[s1]<<"\n";
					clockCycles+= (2*row_access_delay+col_access_delay);
				}
			}
			waitingList[row].insert({counter, s0, s1, address});
			memoryUpdate[address] = counter;
			counterLocation[counter] = row;
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
			int toJump = 0;
			if (s0 == "beq" && registers[s1]==registers[s2]){
				toJump = 1;
			}
			else if (s0 == "bne" && registers[s1]!=registers[s2]){
				toJump = 1;
			}
			if(toJump==1){
				itr=labels[s3];
				cout <<"cycle "<<clockCycles++<<": Jumping to "<<s3<<"\n";
			}
			else{
				cout <<"cycle "<<clockCycles++<<": No jump required to "<< s3 <<"\n";
			}
		}
		else if (s0=="add" || s0=="sub" || s0=="mul" || s0 == "slt" || s0=="addi"){

			if (s1== "$zero"){
				cout << "value of $zero cannot be changed on line "<<(++itr)<<endl;
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

			if (s0 == "add") registers[s1]=registers[s2]+registers[s3];
			else if (s0=="sub") registers[s1]=registers[s2]-registers[s3];
			else if (s0 == "mul") registers[s1]=registers[s2]*registers[s3];
			else if (s0 == "slt") registers[s1]= (registers[s2]<registers[s3]);
			else if (s0=="addi") registers[s1]=registers[s2]+stoi(s3);
			
			cout <<"cycle "<<clockCycles++<<": "<<s1<<"= "<<registers[s1]<<"\n";
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

	if (argc!=4){
		cout <<"You have to pass 4 parameters: ./assignment4.exe <file_name> <row_access_delay> <col_access_delay>\n";
		return 0;
	}
	string fileName = argv[1];
	ifstream myFile(fileName);
	if (!check_number(argv[2]) || !check_number(argv[3])){
		cout << "Invalid Row and Column access delay values\n";
		return 0;
	}
	row_access_delay = stoi(argv[2]);
	col_access_delay = stoi(argv[3]);
	string line;
	fillRegs();
	fillOpers();

	while(getline(myFile,line)){
		line = line.substr(0, line.find('#'));
		vector <string> strings;
		strings = lexer(line);
		if(strings.size()!=0){
			int x = line.find(':');
			if (x!=string::npos){
				instructions.push_back(line.substr(0,x+1));
				line = line.substr(x+1);
				strings = lexer(line);
			}
			if (strings.size()!=0) instructions.push_back(line);
		}
	}
	int n=instructions.size();

	for(int i=0;i<n;i++){
		string currentLine = instructions[i];
		vector<string> strings;

		strings=lexer(currentLine);

		if(strings.size()==1){
			int l=strings[0].size();
			if(strings[0][l-1]==':'){
				// make sure it ain't the name of some operation or register
				if (operations.find(strings[0].substr(0,l-1))!=operations.end() || registers.find(strings[0].substr(0,l-1))!=registers.end()){
					cout << "A label name can't be reserved keyword on line "<<(++i)<<endl;
					throwError =1;
					return 0;
				}
				// Make sure we are not re-definig it
				if (labels.find(strings[0].substr(0,l-1))!=labels.end()){
					cout << "You cannot provide more than 1 set of instructions for same label on line "<<(++i)<<endl;
					throwError =1;
					return 0;
				}
				labels[strings[0].substr(0,l-1)]=i;
			}
			else{
				cout<<"Colon required at the end of label on line "<<(++i)<<endl;
				throwError=1;
				return 0;
			}
		}

		if(strings.size()==2 && strings[1]==":"){
			// make sure it ain't the name of some operation or register
			if (operations.find(strings[0])!=operations.end() || registers.find(strings[0])!=registers.end()){
				cout << "A label name can't be reserved keyword on line "<<(++i)<<endl;
				throwError =1;
				return 0;
			}
			// Make sure we are not re-definig it
			if (labels.find(strings[0])!=labels.end()){
				cout << "You cannot provide more than 1 set of instructions for same label on line "<<(++i)<<endl;
				throwError =1;
				return 0;
			}
			labels[strings[0]]=i;
			instructions[i]=strings[0]+":";
		}
	}
	cout << "Every cycle descripiton\n\n";
	while(itr<n){
		string currentLine = instructions[itr];
		vector<string> strings;

		strings=lexer(currentLine);
		parser(strings);
		if(throwError==1){
			return 0;
		}
	}

	if (currRow!=-1) DRAM[currRow] = buffer;
	else row_buffer_updates=0;

	print_stats();

	return 0;
}

// To check if a string denotes a integer or not
bool check_number(string str){
	if(str.length()==0)
		return true;
   	if(!isdigit(str[0])){
	   	if(str[0]!='-' && str[0]!='+'){
		   return false;
	   	}
   	}
   	for (int i = 1;i < str.length(); i++)
   		if (isdigit(str[i]) == false)
      		return false;
    return true;
}

// To check whether somethig is a valid memory 
bool checkAddress(string reg){
	int n=reg.length();
	if (check_number(reg)) return true;

	if (n>=7 && reg.substr(n-7)=="($zero)") return true;
	
	if(n<4) return false;

	if (reg[n-4]=='(' && reg[n-1]==')'){
		string s = reg.substr(n-3,2);
		if(registers.find(s)!=registers.end()){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		if (n<5) return false;
		if(reg[n-5]=='(' && reg[n-1]==')'){
			string s = reg.substr(n-4,3);
			if(registers.find(s)!=registers.end()){
				return true;
			}
			else{
				return false;
			}
		}
		else{
			return false;
		}
	}
}

// To get the memory address from a string
int locateAddress(string reg){
	int addr;
	int n=reg.length();
	if(check_number(reg)){
		addr=stoi(reg);
	}
	else{
		int num=0;
		string first = reg.substr(0,n-5);
		string second = reg.substr(n-4,3);
		if (n>=7 && reg.substr(n-7)=="($zero)"){
			first = reg.substr(0,n-7);
			second = reg.substr(n-6,5);
		}
		else if (reg[n-4]=='(' && reg[n-1]==')'){
			first = reg.substr(0,n-4);
			second = reg.substr(n-3,2);
		}
		if(first!=""){
			num = stoi(first);
		}
		addr=(num+registers[second]);
	}
	if(addr%4!=0){
		// because we are handling just lw and sw, in them it must be multiple of 4, in lb it could be anything
		addr=-1;
	}
	if(addr>=(1<<20)){
		// Memory out of limits must be thrown
		addr=-2;
	}
	return addr;
}

// Filling the 32 registers and $zero in the registrs map.
void fillRegs(){
	registers["$r0"]=0;
	registers["$at"]=0;
	registers["$v0"]=0;
	registers["$v1"]=0;
	registers["$zero"] =0;
	
	string c;
	int m;
	for(int i=0;i<3;i++){
		switch(i)
		{
			case 0: {c="$a";m=4;break;}
			case 1: {c="$t";m=10;break;}
			case 2: {c="$s";m=9;break;}
		}
		for(int j=0;j<m;j++){
			registers[c+to_string(j)]=0;
		}
	}
	for (int i=0;i<=32;i++){
		c= "$"+to_string(i);
		registers[c] =0;
	}
	registers["$k0"]=0;
	registers["$k1"]=0;
	registers["$gp"]=0;
	registers["$sp"]=2147479548;
	registers["$ra"]=0;
}

// Filling operations in the Operations map and initialised to zero. 
void fillOpers(){
	operations["add"]=0;
	operations["sub"]=0;
	operations["mul"]=0;
	operations["beq"]=0;
	operations["bne"]=0;
	operations["slt"]=0;
	operations["j"]=0;
	operations["li"]=0; 
	operations["lw"]=0;
	operations["sw"]=0;
	operations["addi"]=0;
}

// Lexer splits the string into tokens such that first 2 were space delimited in the input string and the remaining were space or tab delimited
vector<string> lexer(string line){
	int n=line.length();
	vector<string> v;
	bool first=false;
	bool second=false;
	string s="";
	int i=0;
	while(i<n){
		if(first){
			if(second){
				if(line[i]==','){
					v.push_back(s);
					s="";
					i++;
					while(line[i]==' ' || line[i]=='\t'){
						if(i<n){
							i++;
						}
						else{
							break;
						}
					}
				}
				else{
					if (line[i]!=' ' && line[i]!='\t') s+=line[i];
					i++;
				}
			}
			else{
				if(line[i]==' '||line[i]=='\t'){
					second=true;
					v.push_back(s);
					s="";
					while(line[i]==' ' || line[i]=='\t'){
						if(i<n){
							i++;
						}
						else{
							break;
						}
					}
				}
				else{
					s+=line[i];
					i++;    
				}
			}
		}
		else{
			if(line[i]!=' ' && line[i]!='\t'){
				first=true;
				s+=line[i];
			}
			i++;
		}
	}
	if (s!="") v.push_back(s);
	return v;
}

// Function to print the statistics at the end
void print_stats(){

	cout <<"\nNo. of times each operation was executed:\n\n";
	for (auto u: operations){
		if (u.second!=0){
			cout <<u.first<<": "<<u.second<<"\n";
		}
	}

	cout <<"\nNon-zero values in the memory at the end of the execution:\n\n";
	int addr;
	for (int i=0;i<1024;i++){
		for (int j=0;j<256;j++){
			if (DRAM[i][j]!=0){
				addr = 1024*i+4*j;
				cout << addr<<"-"<<addr+3<<": "<<DRAM[i][j]<<"\n";
			}
		}
	}

	cout <<"\nNon-zero values in registers at the end of execution:\n\n";
	for (auto u: registers){
		if (u.second!=0){
			cout <<u.first<<": "<<u.second<<"\n";
		}
	}

	cout <<"\nTotal number of row buffer updates: "<< row_buffer_updates<<"\n";
	if (currRow!=-1)
		cout<<"Total number of cycles: "<<--clockCycles<<" + "<<row_access_delay<<" (cycles taken for code execution + final writeback delay)\n";
	else
		cout<<"Total number of cycles: "<<--clockCycles<<" (cycles taken for code execution)\n";
}