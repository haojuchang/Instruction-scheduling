import sys
import re
import copy


class Machine_Code:
    # definition of operations : add, sub, mul, div, addi, subi, muli, divi
    # lines : instructions
    # RSAC : rs adder count
    # RSMC : rs muldiv count
    # RSA : rs ALU (for add, sub)
    # RSM : rs MLU (for mul, div)
    # 先抓 rf 再 抓 rat
    # 假設順序執行???

    def __init__(self, lines, RSAC, RSMC):
        self.fp = open("ans.txt", "w")
        self.RSAC = int(RSAC)
        self.RSMC = int(RSMC)
        self.RS = []
        self.ALU = {'RS':'', 'op':'', 'vj':'', 'vk':''}
        self.MLU = {'RS':'', 'op':'', 'vj':'', 'vk':''}
        self.RF = {}
        self.RAT = {}
        self.insts = []

        self.parse(lines)
        self.init()

        self.pc = 0
        self.cycle = 0
        # self.opc = {'add': self.RSAC, 'sub': self.RSAC, 'mul': self.RSMC, 'div': self.RSMC, 'addi': self.RSAC,
        #             'subi': self.RSAC, 'muli': self.RSMC, 'divi': self.RSMC}  # operation cycles
        # self.pc = 0
        # self.insts = []
        # self.REG = {}  # key: register name; value: [RF, RAT]
        # self.RSA = []
        # self.RSM = []
        # self.ALU = ['', '', '', '']  # ['RS0', 'add', '2', '4']
        # self.MLU = ['', '', '', '']
        # self.init()
        # self.parse(lines)
        # self.cycle = 0
        # self.write_info()

    def init(self):
        for i in range(self.RSAC):
            self.RS.append({'RS':'ALU' + str(i), 'busy': False, 'op': '',
                                      'vj': '', 'vk': '', 'qj': '', 'qk': '', 'disp': False})
        for i in range(self.RSMC):
            self.RS.append({'RS':'MLU' + str(i), 'busy': False, 'op': '',
                                      'vj': '', 'vk': '', 'qj': '', 'qk': '', 'disp': False})

    def parse(self, lines):
        for line in lines:
            # line example : 0x110 li R1, 0
            split = re.split(r'[;,\s]\s*', line)

            # self.insts.append(split)
            # split example : [0x11C, add, R1, R2, R3]
            # 連 addi 的 r3 一起存成 str
            # pos = str(split[0])  # 0x11C
            op = str(split[1])  # add
            r1 = str(split[2])  # R1
            r2 = str(split[3])  # R2
            r3 = str(split[4])  # R3

            self.insts.append({'op':op, 'r1':r1, 'r2':r2, 'r3':r3})
            # init RF and RAT
            self.RF[r1] = 0
            self.RF[r2] = 0

            self.RAT[r1] = ''
            self.RAT[r2] = ''
            
            if op in ['add', 'sub', 'mul', 'div']:
                self.RF[r3] = 0
                self.RAT[r3] = ''

    def issue(self):
        newRS = copy.deepcopy(self.RS)
        newRAT = copy.deepcopy(self.RAT)
        if self.pc > len(self.insts):
            return newRS, newRAT

        inst = self.insts[self.pc]
        op = str(inst['op'])
        r1 = str(inst['r1'])
        r2 = str(inst['r2'])
        r3 = str(inst['r3'])

        pos = -1 # 找可以放的 RS
        if op in ['add', 'sub', 'addi', 'subi']:
            for i in range(0, self.RSAC):
                if newRS[i]['busy'] == False:
                    pos = i
                    break
        else:
            for i in range(self.RSAC, self.RSAC + self.RSMC):
                if newRS[i]['busy'] == False:
                    pos = i
                    break

        if pos == -1: # 沒找到
            return newRS, newRAT
        
        # 找到
        newRS[pos]['busy'] = True
        newRS[pos]['op'] = op

        if newRAT[r2] != '': # 先去 rat 找值
            newRS[pos]['qj'] = newRAT[r2]
        else: # 去 rf 抓值
            newRS[pos]['vj'] = self.RF[r2]
            print('hi', r2, self.RF[r2])

        if op not in ['muli', 'divi', 'addi', 'subi']: # 不是 i-type 指令
            if newRAT[r3] != '':
                newRS[pos]['qk'] = newRAT[r3]
            else:
                newRS[pos]['vk'] = self.RF[r3]
        else:
            newRS[pos]['vk'] = r3

        newRAT[r1] = newRS[pos]['RS'] # 更新 rat
        self.pc += 1
        return newRS, newRAT

    def dispatch(self):
        newALU = copy.deepcopy(self.ALU)
        newMLU = copy.deepcopy(self.MLU)
        return newALU, newMLU

    def WB(self, newRS, newRAT, newALU, newMLU):
        self.RS = newRS
        self.RAT = newRAT
        self.ALU = newALU
        self.MLU = newMLU

    def exe(self):
        self.write_info()
        self.cycle += 1
        newRS, newRAT = self.issue()
        newALU, newMLU = self.dispatch()
        self.WB(newRS, newRAT, newALU, newMLU)
        # self.RS = newRS
        # self.ALU = newALU
        # self.MLU = newMLU
        # self.RF = newRF
        # self.RAT = newRAT

    def write_info(self):
        self.fp.write('>>>>>>> Cycle ' + str(self.cycle) + ' <<<<<<<\n')
        self.fp.write('      ----RF----\n')
        for key, value in self.RF.items():
            self.fp.write(str(key).center(5) + '|' +
                          str(value).center(10) + '|\n')
        self.fp.write('      ----------\n\n')

        self.fp.write('      ----RAT----\n')
        for key, value in self.RAT.items():
            self.fp.write(str(key).center(5) + '|' +
                          str(value).center(11) + '|\n')
        self.fp.write('      -----------\n\n')

        self.fp.write('      --------RS-------\n')
        for r in self.RS:
            self.fp.write(str(r['RS']).center(5) + '|' + str(r['op']).center(5) + '|')

            if str(r['qj']) != '':
                self.fp.write(str(r['qj']).center(5) + '|')
            else:
                self.fp.write(str(r['vj']).center(5) + '|')

            if str(r['qk']) != '':
                self.fp.write(str(r['qk']).center(5) + '|')
            else:
                self.fp.write(str(r['vk']).center(5) + '|\n')
        self.fp.write('      -----------------\n')

        self.fp.write('ALU: ')
        if(self.ALU['RS'] != ''):
            self.fp.write(str(self.ALU['RS']) + ' ' + self.ALU['op'] + ' ' + self.ALU['vj'] + ' ' + self.ALU['vk'] + '\n')
        else:
            self.fp.write('empty\n')

        self.fp.write('MLU: ')
        if(self.MLU['RS'] != ''):
            self.fp.write(str(self.MLU['RS']) + ' ' + self.MLU['op'] + ' ' + self.MLU['vj'] + ' ' + self.MLU['vk'] + '\n')
        else:
            self.fp.write('empty\n\n')

    # def find_RSA_pos(self):  # 找到空的 RSA 位置
    #     for i in range(self.RSAC):
    #         if self.RSA[i][1] == '':
    #             return i
    #     return -1

    # def find_RSM_pos(self):  # 找到空的 RSM 位置
    #     for i in range(self.RSMC):
    #         if self.RSM[i][1] == '':
    #             return i
    #     return -1

    # def issue(self, pos, op, r1, r2, r3):
    #     if self.REG[r1]['RAT'] == "": # 前一個指令同樣寫進r1但還沒 dispatch
    #         if op in ['add', 'sub', 'addi', 'subi']:
    #             pos = self.find_RSA_pos()
    #             if pos != -1:
    #                 self.RSA[pos][1] = op
    #                 self.RSA[pos][2] = r2
    #                 self.RSA[pos][3] = r3
    #                 self.pc += 1
    #         elif op in ['mul', 'div', 'muli', 'divi']:
    #             pos = self.find_RSM_pos()
    #             if pos != -1:
    #                 self.RSM[pos][1] = op
    #                 self.RSM[pos][2] = r2
    #                 self.RSM[pos][3] = r3
    #                 self.pc += 1

    # def capture(self):
    #     for i in range(self.RSAC):
    #         print(self.RSA[i][2])
    #         if self.RSA[i][2] in self.REG.keys():
    #             if self.REG[self.RSA[i][2]]['RAT'] == '':
    #                 self.RSA[i][2] = self.REG[self.RSA[i][2]]['RF']
    #             else:
    #                 self.RSA[i][2] = self.REG[self.RSA[i][2]]['RAT']
    #     for i in range(self.RSMC):
    #         if self.RSM[i][2] in self.REG.keys():
    #             if self.REG[self.RSM[i][2]]['RAT'] == '':
    #                 self.RSM[i][2] = self.REG[self.RSM[i][2]]['RF']
    #             else:
    #                 self.RSM[i][2] = self.REG[self.RSM[i][2]]['RAT']

    # def exe(self):
    #     self.cycle += 1
    #     if self.pc > len(self.insts):
    #         return -1

    #     inst = self.insts[self.pc]
    #     pos = inst[0]
    #     op = inst[1]
    #     r1 = inst[2]
    #     r2 = inst[3]
    #     r3 = inst[4]
        
    #     self.issue(pos, op, r1, r2, r3)
    #     self.write_info()
    #     self.capture()

        
        # if op == "add":  # 0x11C add R1, R2, R3
        #     self.REG[r1] = self.REG[r2] + self.REG[r3]
        # elif op == "sub":  # 0x11C sub R1, R2, R3
        #     self.REG[r1] = self.REG[r2] - self.REG[r3]
        # elif op == "mul":  # 0x11C add R1, R2, R3
        #     self.REG[r1] = self.REG[r2] * self.REG[r3]
        # elif op == "div":  # 0x11C add R1, R2, R3
        #     self.REG[r1] = self.REG[r2] / self.REG[r3]
        # elif op == "addi":  # 0x11C addi R5, R5, 1
        #     self.REG[r1] = self.REG[r2] + int(r3)
        # elif op == "subi":  # 0x11C subi R1, R2, 1
        #     self.REG[r1] = self.REG[r2] - int(r3)
        # elif op == "muli":  # 0x11C addi R5, R5, 1
        #     self.REG[r1] = self.REG[r2] * int(r3)
        # elif op == "divi":  # 0x11C addi R5, R5, 1
        #     self.REG[r1] = self.REG[r2] / int(r3)

        # self.pc += 1


# 執行指令 : python main.py [case] [rs_adder_count] [rs_muldiv_count]
case_dir = sys.argv[1]
rs_adder_count = sys.argv[2]
rs_muldiv_count = sys.argv[3]

f = open(case_dir, 'r')
data = f.read()
lines = data.splitlines()
mc = Machine_Code(lines, rs_adder_count, rs_muldiv_count)


print('RS---', mc.RS[0])
print('ALU---', mc.ALU)
print('MLU---', mc.MLU)
print('RF---', mc.RF)
print('RAT---', mc.RAT)
print('inst---', mc.insts[0])
# print('RSA', mc.RSA)
# print('RSM', mc.RSM)
# print('REG', mc.REG)

mc.exe()
mc.exe()
mc.exe()
# 卡在 連續兩次都寫進 r1
# 這個時候該怎麼半
# 就不issue 他了嗎
# e = 0
# while e >= 0:
#     mc.exe()
