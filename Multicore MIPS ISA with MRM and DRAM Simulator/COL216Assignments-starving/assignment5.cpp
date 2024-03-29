// TODO: Making the request manager is left
// TODO: Capping the simulation time with M is left
#include<bits/stdc++.h>
#include "core.hpp"
using namespace std;

// For getting a reqquest to run in the DRAM, the returned command is non-redundant
tuple <int, int, int, string, int> getCommand(){
	if (waitingList.empty()) return {-1,-1,-1,"",-1};
	int row = currRow;
	if (waitingList.find(currRow)== waitingList.end()) row = (*waitingList.begin()).first;
	int col = (*waitingList[row].begin()).first;
	tuple<int, string, int> comm = waitingList[row][col].front();
	int count = get<0>(comm);
	string reg = get<1>(comm);
	int i = get<2>(comm);
	bool check_lw = (!check_number(reg));	// true if "lw" operation
	bool check1 = check_lw && (cores[i]->registerUpdate.find(reg)== cores[i]->registerUpdate.end());	// means load on that register is not required, hence redundant operation
	bool check2 = (check_lw && (!check1)) && (cores[i]->registerUpdate[reg].first != count);	// Update on register is required but not here, hence redundant operation
	if (!check_lw){
		if (count == get<2>(cores[i]->last_sw) ) cores[i]->last_sw = {-1,0,-1};
	}
	if (check_lw && (check1 || check2)){
		waitingList[row][col].pop();
		queueSize--;
		if (waitingList[row][col].empty() ) waitingList[row].erase(col);
		if (waitingList[row].empty()) waitingList.erase(row);
		return getCommand();
	}
	else return {row, col, count, reg, i};
}

// This should never be passed any redundant instruction
void processCommand(tuple <int, int, int, string, int> command){
	// As you process, don't forget to remove from the thing from Register Update
	int row = get<0>(command);
	int col = get<1>(command);
	int count = get<2>(command);
	string reg = get<3>(command);
	int i = get<4>(command);
	if (row==-1 && col==-1 && count==-1 && reg=="" && i==-1) return;

	int address = 1024*row + 4*col;

	time_req+=DRAMclock;

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
		store = {"sw",to_string(DRAMclock),to_string(address) + "-" + to_string(address+3),to_string(stoi(reg)), count, i};
	}
	else{
		// means this is a lw operation
		cores[i]->registers[reg] = buffer[col];
		cores[i]->registerUpdate.erase(reg);
		time_req+= col_access_delay;
		store  = {"lw", to_string(DRAMclock), reg, to_string(buffer[col]), count, i};
	}
	waitingList[row][col].pop();
	queueSize--;
	if (waitingList[row][col].empty() ) waitingList[row].erase(col);
	if (waitingList[row].empty()) waitingList.erase(row);
}

// This marks the end of a process running in the DRAM 
void processCompletion(){
	stringstream buffercout;
    int count = get<4>(store);
	int i  = get<5>(store);
	string reg = get<2>(store);
	buffercout << to_string(cores[i]->insCounter[count-1]+1) +" => " +cores[i]->instructions[cores[i]->insCounter[count-1]]+"; ";
	if (get<0>(store)=="lw"){
		if (cores[i]->forRefusing[reg] > count) buffercout << "Rejected by processor, "<< reg<< " already updated in procesor\n";
		else  buffercout << reg + "= " + get<3>(store) + "\n";
	}
	else{
		buffercout << "Memory address " + reg + "= " + get<3>(store) + "\n";
	}
	print[{stoi(to_string(time_req)),-1*stoi(get<1>(store))}][i] = buffercout.str();
	time_req =-1;
	store = tuple <string ,string, string, string, int, int>();
}

// Whenever we encounter an unsafe instruction, we use this function to complete the value of register then and there.
void completeRegister(string name, int i){
	if (cores[i]->registerUpdate.find(name)== cores[i]->registerUpdate.end()){
		if (get<2>(store) == name && get<5>(store) == i){
			// Means we were currently processing that required register only
			DRAMclock = time_req+1;
			processCompletion();
		}
		return;
	}
	// else we have unfinished business

	if (time_req != -1){	// Need to complete the ongoing work forcefully
		DRAMclock = time_req+1;
		processCompletion();
	}
	else if (just_did) ++DRAMclock;

	// Complete that [row][col]
	int count_req = cores[i]->registerUpdate[name].first;
	int address  = cores[i]->registerUpdate[name].second;
	int row = address/1024;
	int col = (address%1024)/4;

	tuple<int, string, int> p;
	string reg="";
	int count=-1;
	while(true){
		p = waitingList[row][col].front();
		count = get<0>(p);
		reg = get<1>(p);
		bool check_lw = (!check_number(reg));	// true if "lw" operation
		bool check1 = check_lw &&  (cores[i]->registerUpdate.find(reg)== cores[i]->registerUpdate.end());	// means load on that register is not required, hence redundant operation
		bool check2 = (check_lw && (!check1)) && (cores[i]->registerUpdate[reg].first != count);	// Update on register is required but not here, hence redundant operation
		if (check_lw && (check1 || check2)){
			waitingList[row][col].pop();
			queueSize--;
			if (waitingList[row][col].empty() ) waitingList[row].erase(col);
			if (waitingList[row].empty()) waitingList.erase(row);
			continue;
		}
		processCommand({row,col,count, reg,i});
		DRAMclock = time_req+1;
		processCompletion();

		if (get<0>(p) == count_req) break;
	}
}

// Parser executes the required functions (tokens and core_num as arguments)
void parser(vector<string> tokens, int i){
	cores[i]->insCounter.push_back(cores[i]->itr);
	cores[i]->counter++;
	int m=tokens.size();
	string s0=tokens[0];
	if(m>4){
		cout<<"Core "<<i+1<<": Syntax Error on line "<<(++cores[i]->itr)<<endl;
		cores[i]->error =1;
		return;
	}
	if (m==1){
		cores[i]->itr++;
        if (cores[i]->itr < cores[i]->instructions.size()) parser(lexer(cores[i]->instructions[cores[i]->itr]), i);
		return;
	}
	if(cores[i]->operations.find(s0)==cores[i]->operations.end()){
		cout<<"Core "<<i+1<<": Invalid instruction on line "<<(++cores[i]->itr)<<endl;
		cores[i]->error=1;
		return;
	}

	if(m==2){
		string s1=tokens[1];
		if (s0!="j"){
			cout<<"Core "<<i+1<<": Invalid instruction on line "<<(++cores[i]->itr)<<endl;
			cores[i]->error =1;
			return;
		}
		if(cores[i]->labels.find(s1)==cores[i]->labels.end()){
			cout<<"Core "<<i+1<<": Invalid label on line "<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
		cores[i]->itr=cores[i]->labels[s1];
		stringstream buffercout;
		buffercout << cores[i]->itr+1<<" => "<<cores[i]->instructions[cores[i]->itr]<<"; ";
		buffercout <<"Jumping to "<<s1<<"\n";
		print[{cores[i]->clockCycles, -1*cores[i]->clockCycles}][i] = buffercout.str();
		cores[i]->clockCycles++;
	}
	else if(m==3){
		if (s0 != "lw" && s0!="sw"){
			cout<<"Core "<<i+1<<": Invalid instruction on line "<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
		string s1=tokens[1];
		string s2=tokens[2];
		if(cores[i]->registers.find(s1)==cores[i]->registers.end()){
			cout<<"Core "<<i+1<<": Invalid register on line "<<(++cores[i]->itr)<<"\n";
			cores[i]->error=1;
			return;
		}
		if (s1== "$zero" && s0!="sw"){
			cout<<"Core "<<i+1<<": Value of $zero cannot be changed on line "<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
		if(!checkAddress(s2)){
			cout<<"Core "<<i+1<<": Invalid format of memory address on line "<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
		int clock_initial = DRAMclock;
		completeRegister(extract_reg(s2), i);
		int address=locateAddress(s2, i);
		if (address==-2){
			cout<<"Core "<<i+1<<": Only 2^20 Bytes of memory could be used on line "<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
		if(address<0){
			cout<<"Core "<<i+1<<": Unaligned memory address on line "<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
		int row = address/1024;
		int col = (address%1024)/4;
		if ( address_core.find(address)!= address_core.end() && address_core[address] != i+1){
			cout<<"Core "<<i+1<<": Memory address "<<address<<" already accessed in core "<<address_core[address]<<", error on line"<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
		address_core[address] =i+1;

		if ( address_core.find(address)!= address_core.end() && address_core[address] != i+1){
			cout<<"Core "<<i+1<<": Memory address "<<address<<" already accessed in core "<<address_core[address]<<", error on line"<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
		address_core[address] =i+1;

		if (s0 =="sw") completeRegister(s1, i);
		if (clock_initial!= DRAMclock){	// Means we stopped from completing some register
            cores[i]->clockCycles = DRAMclock;
			didlw.first = false;
			processCommand(getCommand());
		}

		// Incase the lw gets handled by forwarding
		if (s0=="lw" && address == get<0>(cores[i]->last_sw)){
			if (didlw.first && didlw.second==i && cores[i]->clockCycles == DRAMclock){
				cores[i]->clockCycles++;	// because we don't have 2 ports to update a register
				DRAMclock++;
				processCommand(getCommand());				
			}
			cores[i]->registers[s1] = get<1>(cores[i]->last_sw);
			cores[i]->forRefusing[s1] = cores[i]->counter;
			cores[i]->registerUpdate.erase(s1);
			stringstream buffercout;
			buffercout << cores[i]->itr+1<<" => "<<cores[i]->instructions[cores[i]->itr]<<"; ";
			buffercout << s1<<"= "<<get<1>(cores[i]->last_sw)<<"; Due to forwarding"<<"\n";
			print[{cores[i]->clockCycles, -1*cores[i]->clockCycles}][i] = buffercout.str();
			cores[i]->clockCycles++;
		}
		// Normal lw and sw instruction 
		else{
			if (queueSize == MAX_SIZE){
				// We have to process one more command from waiting list, we don't have to finsih it, just need to start it's execution 
				if (time_req != -1){	// Need to complete the ongoing work forcefully
                    DRAMclock = time_req+1;
					processCompletion();
					cores[i]->clockCycles = DRAMclock;
					didlw.first = false;
				}
				else if (just_did) DRAMclock++;
				processCommand(getCommand());
			}
			if (queueSize>=MAX_SIZE){
				cout <<"No space in waiting list\n";
				throwError=1;
				return ;
			}
			stringstream buffercout;
			buffercout << cores[i]->itr+1<<" => "<<cores[i]->instructions[cores[i]->itr]<<"; ";
			buffercout << "DRAM request issued"<<"\n";
			print[{cores[i]->clockCycles, -1*cores[i]->clockCycles}][i] = buffercout.str();
			cores[i]->clockCycles++;
			queueSize++;
			if(s0=="lw"){
				waitingList[row][col].push({cores[i]->counter, s1, i});
				cores[i]->registerUpdate[s1] = {cores[i]->counter, address};
			}
			else if(s0=="sw"){
				waitingList[row][col].push({cores[i]->counter, to_string(cores[i]->registers[s1]), i});
				cores[i]->last_sw = {address, cores[i]->registers[s1], cores[i]->counter};
			}
		}
	}
	else if(m==4){
		string s1=tokens[1];
		string s2=tokens[2];
		string s3=tokens[3];
		if (cores[i]->registers.find(s1)==cores[i]->registers.end() || cores[i]->registers.find(s2)==cores[i]->registers.end()){
			cout<<"Core "<<i+1<<": Invalid register on line "<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
		if(s0=="beq" || s0=="bne"){
			if(cores[i]->labels.find(s3)==cores[i]->labels.end()){
				cout<<"Core "<<i+1<<": Invalid label on line "<<(++cores[i]->itr)<<endl;
				cores[i]->error=1;
				return;
			}
			int clock_initial = DRAMclock;
			completeRegister(s1, i);
			completeRegister(s2, i);
			if (clock_initial!= DRAMclock){
				cores[i]->clockCycles = DRAMclock;
				processCommand(getCommand());
			}
			int toJump = 0;
			if (s0 == "beq" && cores[i]->registers[s1]==cores[i]->registers[s2]){
				toJump = 1;
			}
			else if (s0 == "bne" && cores[i]->registers[s1]!=cores[i]->registers[s2]){
				toJump = 1;
			}
			stringstream buffercout;
			buffercout << cores[i]->itr+1<<" => "<<cores[i]->instructions[cores[i]->itr]<<"; ";
			if(toJump==1){
				cores[i]->itr=cores[i]->labels[s3];
				buffercout <<"Jumping to "<<s3<<"\n";
			}
			else{
				buffercout <<"No jump required to "<< s3 <<"\n";
			}
			print[{cores[i]->clockCycles, -1*cores[i]->clockCycles}][i] = buffercout.str();
			cores[i]->clockCycles++;
		}
		else if (s0=="add" || s0=="sub" || s0=="mul" || s0 == "slt" || s0=="addi"){

			if (s1== "$zero"){
				cout<<"Core "<<i+1<<": Value of $zero cannot be changed on line "<<(++cores[i]->itr)<<endl;
				cores[i]->error=1;
				return;
			}
			if(s0!= "addi" && cores[i]->registers.find(s3)==cores[i]->registers.end()){
				cout<<"Core "<<i+1<<": Invalid register on line "<<(++cores[i]->itr)<<endl;
				cores[i]->error=1;
				return;
			}
			if(s0=="addi" && !check_number(s3)){
				cout<<"Core "<<i+1<<": Immediate value is not an integer on line "<<(++cores[i]->itr)<<endl;
				cores[i]->error=1;
				return;
			}
			
			int clock_initial = DRAMclock;
			completeRegister(s2, i);
			if (s0 != "addi") completeRegister(s3, i);
			if (clock_initial!= DRAMclock){
				didlw.first = false;
				cores[i]->clockCycles = DRAMclock;
				processCommand(getCommand());
			}

			if (didlw.first && didlw.second ==i && cores[i]->clockCycles ==DRAMclock){
				cores[i]->clockCycles++;	// because we don't have 2 ports to update a register
				DRAMclock++;
				processCommand(getCommand());
			}

			if (s0 == "add") cores[i]->registers[s1]=cores[i]->registers[s2]+cores[i]->registers[s3];
			else if (s0=="sub") cores[i]->registers[s1]=cores[i]->registers[s2]-cores[i]->registers[s3];
			else if (s0 == "mul") cores[i]->registers[s1]=cores[i]->registers[s2]*cores[i]->registers[s3];
			else if (s0 == "slt") cores[i]->registers[s1]= (cores[i]->registers[s2]<cores[i]->registers[s3]);
			else if (s0=="addi") cores[i]->registers[s1]=cores[i]->registers[s2]+stoi(s3);

			cores[i]->forRefusing[s1] = cores[i]->counter;
			cores[i]->registerUpdate.erase(s1);	
			stringstream buffercout;
			buffercout << cores[i]->itr+1<<" => "<<cores[i]->instructions[cores[i]->itr]<<"; ";
			buffercout <<s1<<"= "<<cores[i]->registers[s1]<<"\n";
			print[{cores[i]->clockCycles, -1*cores[i]->clockCycles}][i] = buffercout.str();
			cores[i]->clockCycles++;
		}
		
		else{
			cout<<"Core "<<i+1<<": Invalid instruction on line "<<(++cores[i]->itr)<<endl;
			cores[i]->error=1;
			return;
		}
	}
	cores[i]->itr++;
	if(m!=1){
		cores[i]->operations[s0]++;
	}
	
}

int main(int argc, char** argv){
	initialize(argc, argv);
	if(throwError==1){
		return 0;
	}

	// Start running the file
	while(true){
		didlw = {false, 1};
		just_did = false;
        if (time_req == DRAMclock){
			if (get<0>(store) == "lw"){
				didlw.first = true;
				didlw.second = get<5>(store);
			}
			just_did = true;
            processCompletion();
        }

		bool all_error = true;
		bool all_done = true;
		for (int i=0;i<N;i++){
			if (cores[i]->error ==0) all_error= false;
		}
		for (int i=0;i<N;i++){
			if (cores[i]->error ==0 && cores[i]->itr < cores[i]->instructions.size()) all_done = false;
		}
		if (all_error || throwError==1){
			if (all_error) cout <<"\nError in all the files\n";
			return 0;
		}
		if (all_done){
			break;
		}

		for (int i=0;i<N;i++){
			if (cores[i]->error==0 && cores[i]->itr < cores[i]->instructions.size()) parser(lexer(cores[i]->instructions[cores[i]->itr]), i);
		}
		
		DRAMclock++;
		if (time_req==-1){
			processCommand(getCommand());
		}
	}
	DRAMclock++;

	// Complete any remaining requests in the DRAM
	if (time_req != -1){
		DRAMclock = time_req+1;
		processCompletion();
	}
	while (!waitingList.empty()){
		processCommand(getCommand());
		if(time_req==(-1)) break;
		DRAMclock = time_req+1;
		processCompletion();
	}

	if (currRow!=-1) DRAM[currRow] = buffer;

	print_stats();

	return 0;
}