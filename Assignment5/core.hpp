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

map<int, map<int, queue<pair<int, string>>>> waitingList;
//[row][col]= {counter, reg_name/value}
//value denotes that it was a sw instruction and reg_name(register name) denotes it was an lw instruction

int queueSize = 0;
const int MAX_SIZE = 64;

/*************************** DRAM *******************************/

vector<vector<int>> DRAM(1024, vector<int>(256, 0)); //because every column in itself represents 4 bytes so the column size is only 256
vector<int> buffer(256, 0);
tuple<string, string, string, string, int> store;
// {sw, clockCycles, address- address+3, value, count}
// {lw, clockCycles, register name, value, count}

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
bool check_address(string reg);
int locate_address(string reg);
void print_stats();