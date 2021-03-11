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
    
    	while(getline(myFile,line)){
    		instructions.push_back(line);
    	}
    	int n=instructions.size();
    
    	for(int i=0;i<n;i++){
    		string currentLine = instructions[i];
    	
    		//process currentLine and update registers
    	
    		clockCycles++;
    		printRegisters(registers);
    	}

    	cout<<"No. of clock cycles: "<<clockCycles<<endl;
    
    	return 0;
}
