#include<bits/stdc++.h>
using namespace std;

int throwError = 0;

class Core{

    public:
        map<string, int> registers;
        unordered_map<string, int> operations;
        unordered_map<string, int> labels;
        vector<string> instructions;
        vector<int> insCounter;
        unordered_map<string, int> forRefusing; // this will store the last count at which a register was updated in the *Processor*
        map<string, pair<int, int>> registerUpdate; // [reg_name] = {counter, address}; the counter and the last address at which it was updated
        
        int itr=0;
        int counter=0;
        int last_updated_address = -1;
        int last_stored_value = 0;
        int clockCycles = 1;
        int error=0;
        Core();
        Core(string fileName);

    private:
        void fillIns(string fileName);
        void fillRegs();
        void fillOpers();

};

int N = 1;	// N cores
long long MAX_TIME = 2000000000;	// M time to run
vector<Core*> cores;
int currentCore = 0;

/*********************** Request Manager *************************/

map<int, map<int, queue<tuple<int, string, int>>>> waitingList;
//[row][col]= {counter, reg_name/value, core}
//value denotes that it was a sw instruction and reg_name(register name) denotes it was an lw instruction

int queueSize = 0;
const int MAX_SIZE = 64;

/*************************** DRAM *******************************/

vector<vector<int>> DRAM(1024, vector<int>(256, 0)); //because every column in itself represents 4 bytes so the column size is only 256
vector<int> buffer(256, 0);
tuple<string, string, string, string, int, int> store;
// {sw, clockCycles, address- address+3, value, count, core}
// {lw, clockCycles, register name, value, count, core}

int DRAMclock=1;
pair<bool, int> didlw ={false, 1};  // This will store, whether lw was last performed and also in which core
int currRow = -1; //row number of current row buffer
int row_buffer_updates = 0;
int time_req = -1; // This marks the clock cycle at which the DRAM request will complete
int row_access_delay = 10;
int col_access_delay = 2;

/*********************** Helper functions ***********************/
void initialize(int argc, char** argv);
bool check_number(string str);
string extract_reg(string reg);
vector<string> lexer(string line);
bool checkAddress(string reg);
int locateAddress(string reg);
void print_stats();

/*********************** Function Definitions ***********************/

Core::Core(){
    fillRegs();
    fillOpers();
}

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
                    error =1;
                    return;
                }
                // Making sure that the same name is not given to more than one labels
                if (labels.find(strings[0].substr(0,l-1))!=labels.end()){
                    cout << "You cannot provide more than 1 set of instructions for same label on line "<<(++i)<<endl;
                    error =1;
                    return;
                }
                labels[strings[0].substr(0,l-1)]=i;
            }
            else{
                cout<<"Colon required at the end of label on line "<<(++i)<<endl;
                error=1;
                return;
            }
        }

        if(strings.size()==2 && strings[1]==":"){
            // Making sure the label name isn't the name of any operation or register
            if (operations.find(strings[0])!=operations.end() || registers.find(strings[0])!=registers.end()){
                cout << "A label name can't be reserved keyword on line "<<(++i)<<endl;
                error =1;
                return;
            }
            // Making sure that the same name is not given to more than one labels
            if (labels.find(strings[0])!=labels.end()){
                cout << "You cannot provide more than 1 set of instructions for same label on line "<<(++i)<<endl;
                error =1;
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
void print_stats()
{

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
    for (int i=0 ;i<N;i++){
        cout <<"For "<<i<<"th core:\n";
        for (auto u : cores[i]->registers)
        {
            if (u.second != 0)
            {
                cout << u.first << ": " << u.second << "\n";
            }
        }
        cout <<"\n";
    }

    cout << "\nTotal number of row buffer updates: " << row_buffer_updates << "\n";
    if (currRow != -1)
        cout << "Total number of cycles: " << --DRAMclock << " + " << row_access_delay << " (cycles taken for code execution + final writeback delay)\n";
    else
        cout << "Total number of cycles: " << --DRAMclock << " (cycles taken for code execution)\n";
}

void initialize_short(int argc, char** argv){
    N = stoi(argv[1]);
    MAX_TIME = stoll(argv[2]);
    string folder  = argv[3];
    for (int i=0;i<N;i++){
        string filename = folder +"/"+to_string(i);
        cores.push_back(new Core(filename));
    }
}

// To initialize data according to N cores.
void initialize(int argc, char** argv)
{
    if (argc <3){
        cout<<"Provide appropriate number of arguments.\n";
		throwError=1;
		return;
    }
	if(!check_number(argv[1])){
		"Invalid value for number of cores.\n";
		throwError = 1;
		return;
	}
	N = stoi(argv[1]);
	MAX_TIME = stoll(argv[2]);

    if (argc == N+5){
		if(!check_number(argv[N+3]) || !check_number(argv[N+4])){
			cout<<"Invalid value of delays.\n";
			throwError = 1;
			return;
		}
		row_access_delay = stoi(argv[N+3]);
		col_access_delay = stoi(argv[N+4]);
	}
    else if (argc != N+3){
		cout<<"Provide appropriate number of arguments.\n";
		throwError=1;
		return;
	}

	for(int i=3;i<N+3;i++){
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
    Core dummy = Core();
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
        if (dummy.registers.find(s) != dummy.registers.end())
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
            if (dummy.registers.find(s) != dummy.registers.end())
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
    Core dummy = Core();
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
        addr = (num + dummy.registers[second]);
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

