###### tags: `git project`

# Instruction-scheduling
高等計算機結構 Project2
* HackMD 連結: https://hackmd.io/@EzhUyvwWT32Gy69CeyFNyA/ByHZeB4kL

# 目錄
[TOC]

## 程式執行方法
再 linux 環境上執行  
需要安裝 g++
```=
g++ -std=c++11 main.cpp
./a.out [case]

執行範例:
g++ -std=c++11 main.cpp
./a.out case1.txt
```
* [case]
    * i.e. case1.txt

## 測資
```=
ADDI F1 F2 1
SUB F1 F3 F4
DIV F1 F2 F3
MUL F2 F3 F4
ADD F2 F4 F2
ADDI F4 F1 2
MUL F5 F5 F5
ADD F1 F4 F4
```

## Function 說明
### class Tomasulo
* Tomasulo(string case_dir);
    * 建構子
    * 初始化 RF、RAT、RS、IQ
* void init_RF(int REG_count);
    * 初始化RF
* void init_RAT(int REG_count);
    * 初始化 RAT
* void init_RS(int RS_ALU_count, int RS_MLU_count);
    * 初始化 RS
* void init_IQ(string case_dir);
    * 初始化 IQ
* void print_IQ();
    * 印出/寫檔 IQ
* void print_RF();
    * 印出/寫檔 RF
* void print_RAT();
    * 印出/寫檔 RAT
* void print_RS();
    * 印出/寫檔 RS
* void print_ALU();
    * 印出/寫檔 ALU
* void print_MLU();
    * 印出/寫檔 MLU 
* int get_cycle() { return this->cycle; }
    * 回傳當前 cycle 數
* void issue(vector<RS_cell> &RS); 
    * 從 IQ 拿一個指令到 RS
    * pc 值超過指令數，不做事
    * pc 值之指令，已有 RAT 佔據，不做事
* void dispatch(map<string, int> &RF, map<string, string> &RAT, vector<RS_cell> &RS, ALU &alu, MLU &mlu);
    * 從 RS 中 找可以 執行的指令 運算
* void broadcast(vector<RS_cell> &RS, map<string, int> &RF, map<string, string> &RAT, ALU &alu, MLU &mlu); 
    * 將運算好的指令寫回 RF，RAT，RS
* bool Tomasulo::exe();
    * 執行 issue、dispatch、broadcast