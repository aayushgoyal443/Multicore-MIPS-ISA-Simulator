#include<bits/stdc++.h>
#include<fstream>
using namespace std;

map<string,int> registers;
map<string,int> operations;
map<string,int> labels;
int itr = 0;

void printRegisters(){
	for (auto i: registers){
		cout<<i.first<<": "<<i.second<<endl;
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
	registers["r0"]=0;
	registers["at"]=0;
	registers["v0"]=0;
	registers["v1"]=0;
	
	string c;
	int m;
	for(int i=0;i<3;i++){
		switch(i)
		{
			case 0: {c="a";m=4;break;}
			case 1: {c="t";m=10;break;}
			case 2: {c="s";m=9;break;}
		}
		for(int j=0;j<m;j++){
			registers[c+to_string(j)]=0;
		}
	}
	
	registers["k0"]=0;
	registers["k1"]=0;
	registers["gp"]=0;
	registers["sp"]=0;
	registers["ra"]=0;
}

void fillOpers()
{
	operations["add"]=0;
	operations["sub"]=0;
	operations["mul"]=0;
	operations["mflo"]=0;  
	operations["beq"]=0;
	operations["bne"]=0;
	operations["slt"]=0;
	operations["j"]=0;
	operations["li"]=0;	
	operations["sw"]=0;
	operations["addi"]=0;
}

void process(vector<string> tokens){
	int m=tokens.size();
	if(m==4){
		if(tokens[0]=="add"){
			registers[tokens[1]]=registers[tokens[2]]+registers[tokens[3]];
		}
	}else{
		registers[tokens[1]]=stoi(tokens[2]);
	}
	operations[tokens[0]]++;
}

int main(int argc, char** argv)
{
	string fileName = argv[1];
	vector<string> instructions;
	ifstream myFile(fileName);
	string line;
	int clockCycles=0;
	fillRegs();
	fillOpers();

	while(getline(myFile,line)){
		instructions.push_back(line);
	}
	int n=instructions.size();

	while(itr<n){
		string currentLine = instructions[itr];
		istringstream f(currentLine);
		vector<string> strings;
		string c;

		while(getline(f,c,' ')){
			strings.push_back(c);		
		}

		process(strings);
		clockCycles++;
		printRegisters();
	}

	printOperations();
	cout<<"No. of clock cycles: "<<clockCycles<<endl;

	return 0;
}
