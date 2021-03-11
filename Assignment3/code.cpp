#include<bits/stdc++.h>
#include<fstream>
using namespace std;

void printRegisters(unordered_map<string,int> registers){
	for (auto i: registers){
		cout<<i.first<<": "<<i.second<<endl;
	}
	cout<<"\n";
}

void fill(unordered_map<string,int> &registers){
	registers.insert(make_pair("r0",0));
	registers.insert(make_pair("at",0));
	registers.insert(make_pair("v0",0));
	registers.insert(make_pair("v1",0));
	
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
			registers.insert(make_pair(c+to_string(j),0));
		}
	}
	
	registers.insert(make_pair("k0",0));
	registers.insert(make_pair("k1",0));
	registers.insert(make_pair("gp",0));
	registers.insert(make_pair("sp",0));
	registers.insert(make_pair("ra",0));
}

void fill2(unordered_map<string,int> &operations)
{
	operations.insert(make_pair("add",0));
	operations.insert(make_pair("sub",0));
	operations.insert(make_pair("mul",0));
	operations.insert(make_pair("mflo",0));
	operations.insert(make_pair("beq",0));
	operations.insert(make_pair("bne",0));
	operations.insert(make_pair("slt",0));
	operations.insert(make_pair("j",0));
	operations.insert(make_pair("li",0));
	operations.insert(make_pair("sw",0));
	operations.insert(make_pair("addi",0));
}

void process(vector<string> tokens,unordered_map<string,int> &registers,unordered_map<string,int> &operations){
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
    	unordered_map<string,int> registers;
		unordered_map<string,int> operations;
    	fill(registers);
		fill2(operations);
    
    	while(getline(myFile,line)){
    		instructions.push_back(line);
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

			process(strings,registers,operations);

    	
    		clockCycles++;
    		printRegisters(registers);
			printRegisters(operations);
    	}

    	cout<<"No. of clock cycles: "<<clockCycles<<endl;
    
    	return 0;
}
