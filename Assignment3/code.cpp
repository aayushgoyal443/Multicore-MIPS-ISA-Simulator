#include<bits/stdc++.h>
#include<fstream>
using namespace std;

unordered_map<string,int> registers;
unordered_map<string,int> operations;
unordered_map<string,int> labels;
int memory[1<<18] = {0};
vector<string> instructions;
int itr = 0;
int throwError = 0;
int newlines = 0;

string int_to_hex(int n){
	string binary;
	int np = abs(n);
	for (int i=0;i<32;i++){
		binary += '0';
	}
	int i=31;
	while(np!=0){
		if (np%2==1) binary[i] = '1';
		np/=2;
		i--;
	}
	i=31;
	while(n<0 && i>=0){
		if (binary[i]=='0') binary[i] = '1';
		else binary[i] = '0';
		i--;
	}
	i=31;
	while(n<0 && i>=0){
		if (binary[i]=='0'){
			binary[i]='1';
			break;
		}
		else{
			binary[i] = '0';
			i--;
		}
	}
	string hex;
	for (int i=0;i<8;i++){
		int val=0;
		val = (binary[31-4*i]-'0')+(binary[31-4*i-1]-'0')*2+(binary[31-4*i-2]-'0')*4+(binary[31-4*i-3]-'0')*8;
		if (val>=0 && val<=9) hex = char('0'+val) + hex;
		else{
			hex = char('a'+val-10) + hex;
		}
	}

	return hex;
}

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

bool checkReg(string reg){
	int n=reg.length();
	if(!check_number(reg)){
		if(n<5){
			return false;
		}else{
			if(reg[n-5]=='(' && reg[n-1]==')'){
				string s = reg.substr(n-4,3);
				if(registers.find(s)!=registers.end()){
					return true;
				}else{
					return false;
				}
			}else{
				return false;
			}
		}
	}else{
		return true;
	}
}

int locateAddress(string reg){
	int addr;
	int n=reg.length();
	if(check_number(reg)){
		addr=stoi(reg);
	}else{
		int num=0;
		string first = reg.substr(0,n-4);
		string second = reg.substr(n-4,3);
		if(first!=""){
			num = stoi(first);
		}
		addr=(num+registers[second]);
		if(addr%4==0){
			addr=addr/4;
		}else{
			addr=-1;
		}
	}
	if(addr>(1<<18)){
		addr=-1;
	}
	return addr;
}

void printRegisters(){
	for (auto i: registers){
		cout<<(i.first).substr(1)<<": "<<int_to_hex(i.second)<<endl;
	}
	cout<<"\n";
}

void printOperations(){
	for (auto i: operations){
		cout<<i.first<<": "<<i.second<<endl;
	}
	cout<<"\n";
}

void fillRegs(){
	registers["$r0"]=0;
	registers["$at"]=0;
	registers["$v0"]=0;
	registers["$v1"]=0;
	
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
	
	registers["$k0"]=0;
	registers["$k1"]=0;
	registers["$gp"]=0;
	registers["$sp"]=2147479548;
	registers["$ra"]=0;
}

void fillOpers()
{
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

void parser(vector<string> tokens){
	int m=tokens.size();
	string s0=tokens[0];
	if(m>4){
		cout<<"Syntax Error on line "<<++itr<<endl;
		throwError =1;
		return;
	}
	else{
		if(m!=1){
			if(operations.find(s0)!=operations.end()){
				if(m==2){
					string s1=tokens[1];
					if(labels.find(s1)!=labels.end()){
						if(s0=="j"){
							itr=labels[s1];
						}
					}else{
						cout<<"Invalid label on line "<<++itr<<endl;
						throwError=1;
						return;
					}
				}else if(m==3){
					string s1=tokens[1];
					string s2=tokens[2];
					if(registers.find(s1)==registers.end()){
						cout<<"Invalid register on line "<<++itr<<endl;
						throwError=1;
						return;
					}
					if(s0=="li"){
						if(!check_number(s2)){
							cout<<"Please provide a number as a second argument on line "<<++itr<<endl;
							throwError=1;
							return;
						}
						registers[s1]=stoi(s2);
					}else{
						if(!checkReg(s2)){
							cout<<"Invalid format of memory address on line "<<++itr<<endl;
							throwError = 1;
							return;
						}
						int address=locateAddress(s2);
						if(s0=="lw"){
							if(address<0){
								cout<<"Unaligned memory address on "<<++itr<<endl;
								throwError = 1;
								return;
							}else{
								registers[s1]=memory[address];
							}
						}else if(s0=="sw"){
							if(address<0){
								cout<<"Unaligned memory address on "<<++itr<<endl;
								throwError = 1;
								return;
							}else{
								memory[address]=registers[s1];
							}		
						}else{
							cout<<"Invalid instruction on line"<<++itr<<endl;
							throwError=1;
							return;
						}
					}
				}else if(m==4){
					string s1=tokens[1];
					string s2=tokens[2];
					string s3=tokens[3];
					if(registers.find(s1)!=registers.end() && registers.find(s2)!=registers.end()){
						if(s0=="add"){
							if(registers.find(s3)!=registers.end()){
								registers[s1]=registers[s2]+registers[s3];
							}else{
								cout<<"Invalid register on line "<<++itr<<endl;
								throwError=1;
								return;
							}
						}else if(s0=="sub"){
							if(registers.find(s3)!=registers.end()){
								registers[s1]=registers[s2]-registers[s3];
							}else{
								cout<<"Invalid register on line "<<++itr<<endl;
								throwError=1;
								return;
							}
						}else if(s0=="mul"){
							if(registers.find(s3)!=registers.end()){
								registers[s1]=registers[s2]*registers[s3];
							}else{
								cout<<"Invalid register on line "<<++itr<<endl;
								throwError=1;
								return;
							}
						}else if(s0=="slt"){
							if(registers.find(s3)!=registers.end()){
								if(registers[s2]<registers[s3]){
									registers[s1]=1;
								}else{
									registers[s1]=0;
								}
							}else{
								cout<<"Invalid register on line "<<++itr<<endl;
								throwError=1;
								return;
							}
						}else if(s0=="addi"){
							if(check_number(s3)){
								registers[s1]=registers[s2]+stoi(s3);
							}else{
								cout<<"Immediate value is not an integer on line "<<++itr<<endl;
								throwError=1;
								return;
							}
						}else if(s0=="beq"){
							if(registers[s1]==registers[s2]){
								if(labels.find(s3)!=labels.end()){
									itr=labels[s3];
								}else{
									cout<<"Invalid label on line "<<++itr<<endl;
									throwError=1;
									return;
								}
							}
						}else if(s0=="bne"){
							if(registers[s1]!=registers[s2]){
								if(labels.find(s3)!=labels.end()){
									itr=labels[s3];
								}else{
									cout<<"Invalid label on line "<<++itr<<endl;
									throwError=1;
									return;
								}
							}
						}
					}else{
						cout<<"Invalid register on line "<<++itr<<endl;
						throwError=1;
						return;
					}
				}
				itr++;
				if(m!=1){
					operations[s0]++;
				}
			}else{
				cout<<"Invalid Instruction on line "<<++itr<<endl;
				throwError=1;
				return;
			}
		}else{
			itr++;
		}
	}
}

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
						}else{
							break;
						}
					}
				}else{
					if (line[i]!=' ' && line[i]!='\t') s+=line[i];
					i++;
				}
			}else{
				if(line[i]==' '||line[i]=='\t'){
					second=true;
					v.push_back(s);
					s="";
					while(line[i]==' ' || line[i]=='\t'){
						if(i<n){
							i++;
						}else{
							break;
						}
					}
				}else{
					s+=line[i];
					i++;    
				}
			}
		}else{
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

int main(int argc, char** argv)
{
	string fileName = argv[1];
	ifstream myFile(fileName);
	string line;
	int clockCycles=0;
	fillRegs();
	fillOpers();

	while(getline(myFile,line)){
		vector <string> strings;
		strings = lexer(line);
		if(strings.size()!=0){
			instructions.push_back(line);
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
				labels[strings[0].substr(0,l-1)]=i;
			}else{
				cout<<"Colon required at the end of label at line "<<++i<<endl;
				throwError=1;
				return 0;
			}
		}

		if(strings.size()==2 && strings[1]==":"){
			labels[strings[0]]=i;
			instructions[i]=strings[0]+":";
		}
	}

	while(itr<n){
		string currentLine = instructions[itr];
		vector<string> strings;

		strings=lexer(currentLine);
		parser(strings);
		if(throwError==1){
			return 0;
		}
		clockCycles++;
		printRegisters();
	}

	printOperations();
	cout<<"No. of clock cycles: "<<clockCycles<<endl;

	return 0;
}