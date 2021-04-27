#include<bits/stdc++.h>
#include "core.hpp"
using namespace std;

Core::Core(string fileName){
    itr = 0;
    counter = 0;
    fillRegs();
    fillOpers();
    fillIns(fileName);
}
 
void Core::fillIns(string fileName){
    // For filling the instructions vector
    string line;
    ifstream myFile(fileName);
    while(getline(myFile,line)){
        //Ignoring inline comments which start with #
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
    
    // For updating the labels Map
    for(int i=0;i<n;i++){
        string currentLine = instructions[i];
        vector<string> strings;

        strings=lexer(currentLine);

        if(strings.size()==1){
            int l=strings[0].size();
            if(strings[0][l-1]==':'){
                // Making sure the label name isn't the name of any operation or register
                if (operations.find(strings[0].substr(0,l-1))!=operations.end() || registers.find(strings[0].substr(0,l-1))!=registers.end()){
                    cout << "A label name can't be reserved keyword on line "<<(++i)<<endl;
                    throwError =1;
                    return;
                }
                // Making sure that the same name is not given to more than one labels
                if (labels.find(strings[0].substr(0,l-1))!=labels.end()){
                    cout << "You cannot provide more than 1 set of instructions for same label on line "<<(++i)<<endl;
                    throwError =1;
                    return;
                }
                labels[strings[0].substr(0,l-1)]=i;
            }
            else{
                cout<<"Colon required at the end of label on line "<<(++i)<<endl;
                throwError=1;
                return;
            }
        }

        if(strings.size()==2 && strings[1]==":"){
            // Making sure the label name isn't the name of any operation or register
            if (operations.find(strings[0])!=operations.end() || registers.find(strings[0])!=registers.end()){
                cout << "A label name can't be reserved keyword on line "<<(++i)<<endl;
                throwError =1;
                return;
            }
            // Making sure that the same name is not given to more than one labels
            if (labels.find(strings[0])!=labels.end()){
                cout << "You cannot provide more than 1 set of instructions for same label on line "<<(++i)<<endl;
                throwError =1;
                return;
            }
            labels[strings[0]]=i;
            instructions[i]=strings[0]+":";
        }
    }
}

// Filling the 32 registers and $zero in the registers map.
void Core::fillRegs()
{
    registers["$r0"] = 0;
    registers["$at"] = 0;
    registers["$v0"] = 0;
    registers["$v1"] = 0;
    registers["$zero"] = 0;

    string c;
    int m;
    for (int i = 0; i < 3; i++)
    {
        switch (i)
        {
        case 0:
        {
            c = "$a";
            m = 4;
            break;
        }
        case 1:
        {
            c = "$t";
            m = 10;
            break;
        }
        case 2:
        {
            c = "$s";
            m = 9;
            break;
        }
        }
        for (int j = 0; j < m; j++)
        {
            registers[c + to_string(j)] = 0;
        }
    }
    for (int i = 0; i <= 32; i++)
    {
        c = "$" + to_string(i);
        registers[c] = 0;
    }
    registers["$k0"] = 0;
    registers["$k1"] = 0;
    registers["$gp"] = 0;
    registers["$sp"] = 2147479548;
    registers["$ra"] = 0;
}

// Filling operations in the operations map and initialised to zero.
void Core::fillOpers()
{
    operations["add"] = 0;
    operations["sub"] = 0;
    operations["mul"] = 0;
    operations["beq"] = 0;
    operations["bne"] = 0;
    operations["slt"] = 0;
    operations["j"] = 0;
    operations["li"] = 0;
    operations["lw"] = 0;
    operations["sw"] = 0;
    operations["addi"] = 0;
}

// Function to print the statistics at the end
void Core::print_stats()
{

    cout << "\nNo. of times each operation was executed:\n\n";
    for (auto u : operations)
    {
        if (u.second != 0)
        {
            cout << u.first << ": " << u.second << "\n";
        }
    }

    cout << "\nNon-zero values in the memory at the end of the execution:\n\n";
    int addr;
    for (int i = 0; i < 1024; i++)
    {
        for (int j = 0; j < 256; j++)
        {
            if (DRAM[i][j] != 0)
            {
                addr = 1024 * i + 4 * j;
                cout << addr << "-" << addr + 3 << ": " << DRAM[i][j] << "\n";
            }
        }
    }

    cout << "\nNon-zero values in registers at the end of execution:\n\n";
    for (auto u : registers)
    {
        if (u.second != 0)
        {
            cout << u.first << ": " << u.second << "\n";
        }
    }

    cout << "\nTotal number of row buffer updates: " << row_buffer_updates << "\n";
    if (currRow != -1)
        cout << "Total number of cycles: " << --clockCycles << " + " << row_access_delay << " (cycles taken for code execution + final writeback delay)\n";
    else
        cout << "Total number of cycles: " << --clockCycles << " (cycles taken for code execution)\n";
}