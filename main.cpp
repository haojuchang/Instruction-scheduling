#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <math.h>
#include <fstream>
#include <sstream>
using namespace std;

struct RS_cell
{
    string name = "";
    bool busy = false;
    string op = "";
    int vj = 0, vk = 0;
    string qj = "", qk = "";
    bool disp = false;
};

struct IQ_cell
{
    // ADDI F1, F2, 1
    // type = "ADDI"
    // v1 = "F1", v2 = "F2", v3 = "1"
    string type = "";
    string v1 = "", v2 = "", v3 = "";
};

struct ALU
{
    map<string, int> cycle_need = {
        {"ADDI", 2},
        {"ADD", 2},
        {"SUBI", 2},
        {"SUB", 2}};
    string dst_name = "";
    string cur_type = "";
    int cur_exe_cycle = 0;
    int v2, v3;
};

struct MLU
{
    map<string, int> cycle_need = {
        {"MULI", 10},
        {"MUL", 10},
        {"DIVI", 20},
        {"DIV", 20}};
    string dst_name = "";
    string cur_type = "";
    int cur_exe_cycle = 0;
    int v2, v3;
};

class Tomasulo
{
public:
    Tomasulo(string case_dir);
    void init_RF(int REG_count); // 定義有幾個 register，與初始值
    void init_RAT(int REG_count);
    void init_RS(int RS_ALU_count, int RS_MLU_count);
    void init_IQ(string case_dir);

    void print_IQ();
    void print_RF();
    void print_RAT();
    void print_RS();
    void print_ALU();
    void print_MLU();

    int get_cycle() { return this->cycle; }

    void issue(vector<RS_cell> &RS);                                                                         // 找空的 RS cell 放進去
    void dispatch(map<string, int> &RF, map<string, string> &RAT, vector<RS_cell> &RS, ALU &alu, MLU &mlu);  // 找一個可以執行的 RS cell 放進 alu/mlu
    void broadcast(vector<RS_cell> &RS, map<string, int> &RF, map<string, string> &RAT, ALU &alu, MLU &mlu); // 把 alu/mlu 運算完的值寫回 RF/ RAT

    bool exe(); // 先把上個 cycle 的值都放進來，再運行這一個 cycle 的值
private:
    string ans_dir = "ans.txt";
    int cycle = 0;
    int pc = 0;
    int MLU_start; // 設定 MLU 是從 RS 的第幾個開始
    vector<IQ_cell> IQ;
    map<string, int> RF;
    map<string, string> RAT;
    vector<RS_cell> RS;
    ALU alu;
    MLU mlu;
};

int main(int argc, char *argv[])
{
    string dir = argv[1];
    Tomasulo to(dir);

    int times = 0;

    while (to.exe())
    {
        cout << "cycle: " << to.get_cycle() << endl;
        ofstream wt("ans.txt", ios::app);
        wt << "cycle: " << to.get_cycle() << endl;
        to.print_IQ();
        to.print_RF();
        to.print_RAT();
        to.print_RS();
        to.print_ALU();
        to.print_MLU();

        // times++;
        // if (times == 6)
        //     break;
    }
    return 0;
}

Tomasulo::Tomasulo(string case_dir)
{
	ofstream wt(this->ans_dir);
    int REG_count = 5;
    int RS_ALU_count = 3;
    int RS_MLU_count = 2;

    this->init_RF(REG_count);
    this->init_RAT(REG_count);
    this->init_RS(RS_ALU_count, RS_MLU_count);
    this->init_IQ(case_dir);
}

void Tomasulo::init_RF(int REG_count)
{
    int REG_value = 0;
    for (int i = 1; i <= REG_count; i++, REG_value++)
    {
        string REG_name = "F" + to_string(i);
        this->RF[REG_name] = pow(2, REG_value);
    }
}

void Tomasulo::init_RAT(int REG_count)
{
    for (int i = 1; i <= REG_count; i++)
    {
        string REG_name = "F" + to_string(i);
        this->RAT[REG_name] = "";
    }
}

void Tomasulo::init_RS(int RS_ALU_count, int RS_MLU_count)
{
    this->MLU_start = RS_ALU_count;
    for (int i = 1; i <= RS_ALU_count; i++)
    {
        RS_cell rs_cell;
        string rs_name = "RS" + to_string(i);
        rs_cell.name = rs_name;
        this->RS.push_back(rs_cell);
    }
    for (int i = RS_ALU_count + 1; i <= RS_ALU_count + RS_MLU_count; i++)
    {
        RS_cell rs_cell;
        string rs_name = "RS" + to_string(i);
        rs_cell.name = rs_name;
        this->RS.push_back(rs_cell);
    }
}

void Tomasulo::init_IQ(string case_dir)
{
    ifstream inputFile;
    inputFile.open(case_dir, ios::in);

    if (!inputFile)
    {
        cerr << "File could not be open at " << case_dir << endl;
        exit(1);
    }

    string str;
    while (!inputFile.eof() && getline(inputFile, str, '\n'))
    {
        stringstream ss;
        ss << str;
        IQ_cell iq_cell;
        ss >> iq_cell.type >> iq_cell.v1 >> iq_cell.v2 >> iq_cell.v3;
        this->IQ.push_back(iq_cell);
    }
}

void Tomasulo::print_IQ()
{
    string output = "";
    for (int i = this->IQ.size() - 1; i >= this->pc; i--)
    {
        output += to_string(i) + " ";
        output += this->IQ[i].type + " ";
        output += this->IQ[i].v1 + " ";
        output += this->IQ[i].v2 + " ";
        output += this->IQ[i].v3 + "\n";
    }
    cout << "===== IQ =====\n"
         << output << endl;

    ofstream wt(this->ans_dir, ios::app);    
    wt << "===== IQ =====\n"
         << output << endl;
}

void Tomasulo::print_RF()
{
    string output = "";
    for (map<string, int>::iterator it = this->RF.begin(); it != this->RF.end(); ++it)
    {
        output += it->first + " ";
        output += to_string(it->second) + "\n";
    }
    cout << "===== RF =====\n"
         << output << endl;
         
    ofstream wt(this->ans_dir, ios::app); 
    wt << "===== RF =====\n"
         << output << endl;
}

void Tomasulo::print_RAT()
{
    string output = "";
    for (map<string, string>::iterator it = this->RAT.begin(); it != this->RAT.end(); ++it)
    {
        output += it->first + " ";
        output += it->second + "\n";
    }
    cout << "===== RAT =====\n"
         << output << endl;

    ofstream wt(this->ans_dir, ios::app); 
    wt << "===== RAT =====\n"
         << output << endl;
}

void Tomasulo::print_RS()
{
    //vector<RS_cell> RS
    RS_cell rs;
    string output = "";
    for (int i = 0; i < this->RS.size(); i++)
    {
        output += this->RS[i].name + " ";
        output += (this->RS[i].busy) ? "T " : "F ";
        output += this->RS[i].op + " ";
        output += (this->RS[i].qj == "") ? to_string(this->RS[i].vj) + " " : "X ";
        output += (this->RS[i].qk == "") ? to_string(this->RS[i].vk) + " " : "X ";
        output += (this->RS[i].qj == "") ? "X " : this->RS[i].qj + " ";
        output += (this->RS[i].qk == "") ? "X " : this->RS[i].qk + " ";
        output += (this->RS[i].disp) ? "T \n" : "F \n";
    }
    cout << "===== RS =====\n"
         << output << endl;
         
    ofstream wt(this->ans_dir, ios::app);
    wt << "===== RS =====\n"
         << output << endl;
}

void Tomasulo::print_ALU()
{
    string output = "";
    output += (this->alu.cur_type == "") ? "X " : this->alu.cur_type + " ";
    output += (this->alu.cur_type == "") ? "X " : to_string(this->alu.v2) + " ";
    output += (this->alu.cur_type == "") ? "X " : to_string(this->alu.v3) + "\n";
    cout << "===== ALU =====\n"
         << output << endl;
         
    ofstream wt(this->ans_dir, ios::app);
    wt << "===== ALU =====\n"
         << output << endl;
}

void Tomasulo::print_MLU()
{
    string output = "";
    output += (this->mlu.cur_type == "") ? "X " : this->mlu.cur_type + " ";
    output += (this->mlu.cur_type == "") ? "X " : to_string(this->mlu.v2) + " ";
    output += (this->mlu.cur_type == "") ? "X " : to_string(this->mlu.v3) + "\n";
    cout << "===== MLU =====\n"
         << output << endl << endl;
         
    ofstream wt(this->ans_dir, ios::app);
    wt << "===== MLU =====\n"
         << output << endl << endl;
}

void Tomasulo::issue(vector<RS_cell> &RS)
{
    if(this->pc == this->IQ.size())
        return;
    string type = this->IQ[this->pc].type;
    string v1 = this->IQ[this->pc].v1;
    string v2 = this->IQ[this->pc].v2;
    string v3 = this->IQ[this->pc].v3;

    if(this->RAT[v1] != "")
        return;

    int st = 0;
    int end = RS.size();
    if (type[0] == 'M' || type[0] == 'D')
        st = this->MLU_start;

    int pos = -1;
    for (int i = st; i < end; i++)
    {
        if (!RS[i].busy)
        {
            pos = i;
            break;
        }
    }

    if (pos != -1)
    {
        this->pc++;
        RS[pos].busy = true;
        RS[pos].op = type;

        if (this->RAT[v2] == "")
            RS[pos].vj = this->RF[v2];
        else
            RS[pos].qj = this->RAT[v2];

        if (type == "ADDI" || type == "SUBI" || type == "MULI" || type == "DIVI")
            RS[pos].vk = stoi(v3);
        else if (this->RAT[v3] == "")
            RS[pos].vk = this->RF[v3];
        else
            RS[pos].qk = this->RAT[v3];

        if (RS[pos].qj == "" && RS[pos].qk == "")
            RS[pos].disp = true;
        this->RAT[v1] = RS[pos].name;
    }
}

void Tomasulo::dispatch(map<string, int> &RF, map<string, string> &RAT, vector<RS_cell> &RS, ALU &alu, MLU &mlu)
{
    // 找原本的RS，改傳進來的 RS
    for (int i = 0; i < this->RS.size(); i++)
    {
        if (this->RS[i].qj != "" && this->RS[i].qk != "")
        {
            this->RS[i].disp = true;
        }
    }

    // 找到 rs.disp == true 的，ALU/MLU 要分開找
    int alu_pos = -1, mlu_pos = -1;
    for (int i = 0; i < MLU_start; i++)
    {
        if (this->RS[i].disp)
        {
            alu_pos = i;
            break;
        }
    }
    for (int i = MLU_start; i < this->RS.size(); i++)
    {
        if (this->RS[i].disp)
        {
            mlu_pos = i;
            break;
        }
    }

    if (alu_pos != -1 && alu.cur_type == "")
    {
        cout << "come" << endl;
        alu.dst_name = this->RS[alu_pos].name;
        alu.cur_type = this->RS[alu_pos].op;
        alu.cur_exe_cycle = alu.cycle_need[alu.cur_type];
        alu.v2 = this->RS[alu_pos].vj;
        alu.v3 = this->RS[alu_pos].vk;

        // RS[alu_pos].busy = false;
        // RS[alu_pos].op = "";
        // RS[alu_pos].vj = 0;
        // RS[alu_pos].vk = 0;
        // RS[alu_pos].qj = "";
        // RS[alu_pos].qk = "";
        // RS[alu_pos].disp = false;
    }

    if (mlu_pos != -1 && mlu.cur_type == "")
    {
        mlu.dst_name = this->RS[mlu_pos].name;
        mlu.cur_type = this->RS[mlu_pos].op;
        mlu.cur_exe_cycle = mlu.cycle_need[mlu.cur_type];
        mlu.v2 = this->RS[mlu_pos].vj;
        mlu.v3 = this->RS[mlu_pos].vk;

        // RS[mlu_pos].busy = false;
        // RS[mlu_pos].op = "";
        // RS[mlu_pos].vj = 0;
        // RS[mlu_pos].vk = 0;
        // RS[mlu_pos].qj = "";
        // RS[mlu_pos].qk = "";
        // RS[mlu_pos].disp = false;
    }
}

void Tomasulo::broadcast(vector<RS_cell> &RS, map<string, int> &RF, map<string, string> &RAT, ALU &alu, MLU &mlu)
{
    if (alu.cur_exe_cycle == 0 && alu.cur_type != "") // alu 是空的
    {
        int alu_ans;
        if (alu.cur_type == "ADD" || alu.cur_type == "ADDI")
        {
            alu_ans = alu.v2 + alu.v3;
        }
        else if (alu.cur_type == "SUB" || alu.cur_type == "SUBI")
        {
            alu_ans = alu.v2 - alu.v3;
        }

        for (map<string, string>::iterator it = RAT.begin(); it != RAT.end(); ++it) // 如果 RAT 有 ALU 計算完的值
        {
            if (it->second == alu.dst_name)
            {
                RF[it->first] = alu_ans; // 更新RF
                it->second = ""; // 把 RAT 清空
                break;
            }
        }

        for(int i = 0; i < RS.size(); i++){ // 把 RS 該欄清空
            if(alu.dst_name == RS[i].name){
                RS[i].busy = false;
                RS[i].op = "";
                RS[i].vj = 0;
                RS[i].vk = 0;
                RS[i].qj = "";
                RS[i].qk = "";
                RS[i].disp = false;
                break;
            }
        }

        // alu 清空
        alu.cur_type = "";
        alu.dst_name = "";
        alu.v2 = 0;
        alu.v3 = 0;
    }
    else if (alu.cur_exe_cycle != 0 && alu.cur_type != "")
    {
        alu.cur_exe_cycle--;
    }

    if (mlu.cur_exe_cycle == 0 && mlu.cur_type != "")
    {
        int mlu_ans;
        if (mlu.cur_type == "MUL" || mlu.cur_type == "MULI")
        {
            mlu_ans = mlu.v2 * mlu.v3;
        }
        else if (mlu.cur_type == "DIV" || mlu.cur_type == "DIVI")
        {
            mlu_ans = mlu.v2 / mlu.v3;
        }

        for (map<string, string>::iterator it = RAT.begin(); it != RAT.end(); ++it)
        {
            if (it->second == mlu.dst_name)
            {
                RF[it->first] = mlu_ans;
                it->second = "";
            }
        }

        for(int i = 0; i < RS.size(); i++){
            if(mlu.dst_name == RS[i].name){
                RS[i].busy = false;
                RS[i].op = "";
                RS[i].vj = 0;
                RS[i].vk = 0;
                RS[i].qj = "";
                RS[i].qk = "";
                RS[i].disp = false;
                break;
            }
        }

        mlu.cur_type = "";
        mlu.dst_name = "";
        mlu.v2 = 0;
        mlu.v3 = 0;
    }
    else if (mlu.cur_exe_cycle != 0 && mlu.cur_type != "")
    {
        mlu.cur_exe_cycle--;
    }
}

bool Tomasulo::exe()
{
    bool notdone = true;
    if (this->pc ==  this->IQ.size())
    {
        if(this->alu.cur_exe_cycle == 0 && this->mlu.cur_exe_cycle ==0){
            notdone = false;
            for(int i = 0; i < this->RS.size(); i++){
                if(RS[i].busy){
                    notdone = true;
                    break;
                }
            }

        }
        
    }
    if(!notdone)
        return false;

    vector<RS_cell> new_RS = this->RS;
    this->issue(new_RS);

    ALU new_alu = this->alu;
    MLU new_mlu = this->mlu;
    this->dispatch(this->RF, this->RAT, new_RS, new_alu, new_mlu);

    this->RS = new_RS;
    this->broadcast(this->RS, this->RF, this->RAT, new_alu, new_mlu);

    this->alu = new_alu;
    this->mlu = new_mlu;

    this->cycle++;
    return true;
}
