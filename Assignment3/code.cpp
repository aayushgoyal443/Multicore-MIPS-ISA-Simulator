#include<bits/stdc++.h>
#include<fstream>
using namespace std;

map<string,int> registers;
map<string,int> operations;
map<string,int> labels;
map<string,string> variables;
vector<string> instructions;
int itr = 0;
int throwError = 0;

bool check_number(string str) {
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

void printRegisters(){
	for (auto i: registers){
		cout<<(i.first).substr(1)<<": "<<i.second<<endl;
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

void process(vector<string> tokens){
	int m=tokens.size();
	string s0=tokens[0];
	if(m>4){
		cout<<"Syntax Error\n";
		throwError =1;
		return;
	}
	else{
		if(operations.find(s0)!=operations.end()){
			if(m==2){
				string s1=tokens[1];
				if(labels.find(s1)!=labels.end()){
					if(s0=="j"){
						itr=labels[s1];
					}
				}else{
					cout<<"Invalid register"<<endl;
					throwError=1;
					return;
				}
			}else if(m==3){
				string s1=tokens[1];
				string s2=tokens[2];
				if(s0=="lw"){
					registers[s1]=stoi(variables[s2]);
				}else if(s0=="sw"){
					return; 		 //edit
				}else{
					registers[s1]=stoi(s2);
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
							cout<<"Invalid register\n";
							throwError=1;
							return;
						}
					}else if(s0=="sub"){
						if(registers.find(s3)!=registers.end()){
							registers[s1]=registers[s2]-registers[s3];
						}else{
							cout<<"Invalid register\n";
							throwError=1;
							return;
						}
					}else if(s0=="mul"){
						if(registers.find(s3)!=registers.end()){
							registers[s1]=registers[s2]*registers[s3];
						}else{
							cout<<"Invalid register\n";
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
							cout<<"Invalid register\n";
							throwError=1;
							return;
						}
					}else if(s0=="addi"){
						if(check_number(s3)){
							registers[s1]=registers[s2]+stoi(s3);
						}else{
							cout<<"Immediate value is not an integer\n";
							throwError=1;
							return;
						}
					}else if(s0=="beq"){
						if(registers[s1]==registers[s2]){
							if(labels.find(s3)!=labels.end()){
								itr=labels[s3];
							}else{
								cout<<"Invalid label\n";
								throwError=1;
								return;
							}
						}
					}else if(s0=="bne"){
						if(registers[s1]!=registers[s2]){
							if(labels.find(s3)!=labels.end()){
								itr=labels[s3];
							}else{
								cout<<"Invalid label\n";
								throwError=1;
								return;
							}
						}
					}
				}else{
					cout<<"Invalid register\n";
					throwError=1;
					return;
				}
			}
			itr++;
			if(m!=1){
				operations[s0]++;
			}
		}else{
			cout<<"Invalid Instruction"<<endl;
			throwError=1;
			return;
		}
	}
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
		if(line!=""){
			instructions.push_back(line);
		}
	}
	int n=instructions.size();

	for(int i=0;i<n;i++){
		string currentLine = instructions[i];
		istringstream f(currentLine);
		vector<string> strings;
		string c;

		while(getline(f,c,' ')){
			strings.push_back(c);		
		}

		if(strings.size()==1){
			labels[strings[0]]=i;
		}
	}

	while(itr<n){
		string currentLine = instructions[itr];
		istringstream f(currentLine);
		vector<string> strings;
		string c;

		while(getline(f,c,' ')){
			strings.push_back(c);		
		}

		process(strings);
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