#include <bits/stdc++.h>
using namespace std;


fstream fout,fin;

//Instruction class
class Instruction
{
public:
    string LABEL,OPCODE,OPERAND;
    bool n, i, x, e,checkLiteral;
    Instruction(){
        LABEL = "";
        OPCODE = "";
        OPERAND = "";
        x = 0;
        e = 0;
        n = 1;
        i = 1;
        checkLiteral = false;        
    }
    Instruction(string line)
    {
        LABEL = "";
        OPCODE = "";
        OPERAND = "";
        x = 0;
        e = 0;
        n = 1;
        i = 1;
        checkLiteral = false;
        if(line.size()>0){
            LABEL = line.substr(0, 10);
            while (LABEL.size()!=0 && LABEL[0] == ' '){
                LABEL.erase(LABEL.begin());
            }
            while (LABEL.size()!=0 && LABEL.back() == ' '){
                LABEL.pop_back();
            }
        }
        if (line.size() > 10){
            OPCODE = line.substr(10, 10);
            while (OPCODE.size()!=0 && OPCODE[0] == ' '){
                OPCODE.erase(OPCODE.begin());
            }
            while (OPCODE.size()!=0 && OPCODE.back() == ' '){
                OPCODE.pop_back();
            }
        }
        if (line.size() > 20){
            OPERAND = line.substr(20, 30);
            while (OPERAND.size()!=0 && OPERAND[0] == ' '){
                OPERAND.erase(OPERAND.begin());
            }
            while (OPERAND.size()!=0 && OPERAND.back() == ' '){
                OPERAND.pop_back();
            }
        }
        if (LABEL[0] == '.' || (LABEL.size()==0 && OPCODE.size()==0 && OPERAND.size()==0))
        {
            OPCODE = ".";
            OPERAND = LABEL = "";
        }else{

            if (OPCODE[0] == '+')
            {
                e = 1;
                string temp1="";
                for(int i=1;i<OPCODE.size();i++){
                    temp1+=OPCODE[i];
                }
                OPCODE=temp1;
            }
            string temp="";
            for(int i=0;i<OPERAND.size();i++){
                if(i>=(int)OPERAND.size()-2){
                temp+=OPERAND[i];
                }
            }
            if (OPERAND.size() > 1 && temp==",X")
            {
                OPERAND = OPERAND.substr(0,(int)OPERAND.size()-2);
                x = 1;
            }

            if (OPERAND[0] == '#')
            {
                OPERAND.erase(OPERAND.begin());
                i = 1;n = 0;
            }else if (OPERAND[0] == '=')
            {
                OPERAND.erase(OPERAND.begin());
                checkLiteral = true;
            }else if (OPERAND[0] == '@')
            {
                OPERAND.erase(OPERAND.begin());
                i = 0;n = 1;
            }

        }
    }
};

unordered_map<string, pair<int, string>> EXTREFTAB,OPTAB;
pair<int, string> getvalfromextreftab(string key)
{   pair<int,string>entry;
    if (EXTREFTAB.find(key) == EXTREFTAB.end()){
        entry={-1, ""};
    }else{
        entry=EXTREFTAB[key];
    }
    return entry;
}

pair<int, string> getvalfromoptab(string key)
{
    pair<int,string>entry={-1,""};
    if (OPTAB.find(key) != OPTAB.end()){
        entry=OPTAB[key];
    }
    return entry;
}
int LOCCTR;            //store the address of current instruction
int STARTADDR;         //store the start address of the program
int BASE;              //Base Register
//SYMTAB and LITTAB 
class Table
{

public:
    map<string, pair<char,int>> mp;
    bool isPresent(string key, int controlsectionid)
    {
        key += to_string(controlsectionid);
        bool flag=false;
        flag|=(mp.find(key) != mp.end());
        return flag;
    }
    bool InsertEntry(string key, char addresstype, int controlsectionid, int address)
    {
        key += to_string(controlsectionid);
        if (mp.find(key) == mp.end() || mp[key].first == ' ')
        {
            mp[key] = {addresstype, address};
            return true;
        }else{
            return false;
        }
    }

    int findValue(int controlsectionid, string key)
    {
        key += to_string(controlsectionid);
        if (mp.find(key) != mp.end()){
            return mp[key].second;
        }else{
            return INT_MIN;
        }
    }
    int findTargetAddr(int controlsectionid, int type, int LOCCTR, int BASE, string key)
    {
        key += to_string(controlsectionid);
        if (mp.find(key) != mp.end()){
        if (type == 4 || mp[key].first == 'A'){
            return mp[key].second;
        }else{
            int targetaddr = mp[key].second - LOCCTR;
            if (targetaddr < -2048 || targetaddr > 2047){
                return mp[key].second - BASE;
            }else{
                return targetaddr;
            }
        }
        }else{
            return INT_MIN;
        }
    }
} SYMTAB, LITTAB;


//convert integer to hex of length len
string convertintegertohex(int val, int len)
{
       stringstream ss;
       ss<< hex << val;
       string loc ( ss.str() );
       while(loc.size()<len){
         loc='0'+loc;
       }
       for(int i=0;i<len;i++){
         loc[i]=toupper(loc[i]);
       }
       return loc;
}

//convert hex to integer
int converthextointeger(string h)
{
    int val=0;
    if(h.size()){
        val+=stoi(h,0,16);
    }
    return val;
}


//reads new line and converts to instruction
Instruction readline(string &line)
{
    getline(fin, line);
    int i = 0, n = line.size();
    if (line[0] == '0')
    {
        i++;
        while (i < line.size()){
            if(!(line[i]-'A'>=0 && line[i]-'A'<=5) && !isdigit(line[i])){
                break;
            }
            i++;
        }
    }
    if (i > 0){
        LOCCTR = converthextointeger(line.substr(1, i - 1));
    }else{

    }
    line = line.substr(i, n - i);
    return Instruction(line);
}

map<char,string> REGTAB;      //code of registers
// Initializes OPTAB and REGTAB
void InitializeOPTAB()
{
    LITTAB.mp.clear();
    SYMTAB.mp.clear();
    REGTAB['A'] = "0";
    REGTAB['X'] = "1";
    REGTAB['B'] = "3";
    REGTAB['S'] = "4";
    REGTAB['T'] = "5";
    REGTAB['F'] = "6";
    OPTAB["LDA"] = {converthextointeger("00"),"3"};
    OPTAB["LDX"] = {converthextointeger("04"),"3"};
    OPTAB["LDL"] = {converthextointeger("08"),"3"};
    OPTAB["LDT"] = {converthextointeger("74"),"3"};
    OPTAB["STA"] = {converthextointeger("0C"),"3"};
    OPTAB["STX"] = {converthextointeger("10"),"3"};
    OPTAB["STL"] = {converthextointeger("14"),"3"};
    OPTAB["LDCH"] = {converthextointeger("50"),"3"};
    OPTAB["STCH"] = {converthextointeger("54"),"3"};
    OPTAB["ADD"] = {converthextointeger("18"),"3"};
    OPTAB["SUB"] = {converthextointeger("1C"),"3"};
    OPTAB["MUL"] = {converthextointeger("20"),"3"};
    OPTAB["DIV"] = {converthextointeger("24"),"3"};
    OPTAB["COMP"] = {converthextointeger("28"),"3"};
    OPTAB["COMPR"] = {converthextointeger("A0"),"2"};
    OPTAB["CLEAR"] = {converthextointeger("B4"),"2"};
    OPTAB["J"] = {converthextointeger("3C"),"3"};
    OPTAB["JLT"] = {converthextointeger("38"),"3"}; 
    OPTAB["JEQ"] = {converthextointeger("30"),"3"};
    OPTAB["JGT"] = {converthextointeger("34"),"3"};
    OPTAB["JSUB"] = {converthextointeger("48"),"3"};
    OPTAB["RSUB"] = {converthextointeger("4C"),"3"};
    OPTAB["TIX"] = {converthextointeger("2C"),"3"};
    OPTAB["TIXR"] = {converthextointeger("B8"),"2"};
    OPTAB["TD"] = {converthextointeger("E0"),"3"};
    OPTAB["RD"] = {converthextointeger("D8"),"3"};
    OPTAB["WD"] = {converthextointeger("DC"),"3"};
}



//check if constant operand
bool checkConstantOperand(string s)
{   
    int n=s.length();
    int idx=0;
    bool flag=true;
    while(idx<n){
        if (!((s[idx]-'0'>=0) && (s[idx]-'0'<=9))){
            flag=false;
        }
        idx++;
    }
    return flag;
}

//returns operand value
pair<int, int> getvalueofoperand(int controlsectionid,string operand)
{
    if (!checkConstantOperand(operand))
    {
    if (SYMTAB.findValue(controlsectionid, operand)!= INT_MIN){
        return {SYMTAB.findValue(controlsectionid, operand), 0};
    }else{
     if (getvalfromextreftab(operand).first != -1){
        return {0, 1};
     }else{
        return {INT_MIN, 0};
     }
    }
    }else{
    return {stoi(operand), 0};
    }
}

//prints Refer,Define records
void printreferdefinerecord( bool define, string operand,int controlsectionid)
{
    string temp = "";
    operand+=',';
    int symbolCnt = 0;
    if (!define){
        fout << "R";
    }else{
        fout << "D";
    }
    for (int i=0;i<operand.length();i++)
    {    
        char c=operand[i];
        if (c != ',')
        {
           temp += c;
        }
        else{

            ++symbolCnt;
            if (symbolCnt == 7)
            {   fout<<endl;
                fout << "D";
                symbolCnt = 1;
            }
            fout<<"^";
            for (int i = 0; i < 6; i++){
                if (i >= temp.size()){
                    fout << " ";
                }else{
                    fout << temp[i];
                }
            }
            if (define){
                fout<<"^";
                int val=SYMTAB.findTargetAddr(controlsectionid, 'R', LOCCTR, BASE, temp);
                fout << convertintegertohex(val, 6);
            }
            temp = "";
        }
    }
    fout << endl;
}
int controlsectcount;                   //number of control sections
//returns value of expression
pair<int, vector<string>> expreval(string expr)
{
    int addr = 0;
    vector<string> list;
    string symbol = "";
    expr+='+';
    int d = 1;
    int i=0,n=expr.size();
    while (i<n)
    {    
        char c=expr[i];
        if (c != '-' && c != '+')
        {
            symbol += c;
        }
        else{
            pair<int,int> p = getvalueofoperand( controlsectcount - 1,symbol);
            string sign="";
            if (p.second)
            {
                if (d == -1){
                    sign = "-";
                    list.push_back(sign + symbol);
                }else{
                    sign = '+';
                    list.push_back(sign + symbol);
                }
            }
            if (p.first != INT_MIN)
            {   int val = d * p.first;
                addr += val;
                d = 1;
            }
            else
            {
                d = 1;
            }
            if (c != '-'){
                symbol.clear();
            }else{
                d = -1;
                symbol.clear();
            }
        }
        i++;
    }
    return {addr, list};
}
vector<string> literallist;
//convert binary to hex
string binaryToHex(vector<int>&b, int n)
{
    string ans = "";
    int num = 0;
    for (int i = 0; i < n; i+=4)
    {
        num = b[i]*8+b[i+1]*4+b[i+2]*2+b[i+3];
        ans+=convertintegertohex(num,1);
    }
    return ans;
}
vector<int> controlsectlengths;        //lengths of control sections
vector<string> controlsectlabels;       //control section labels
bool DUPLABEL=false,INVOP=false,INVLABEL=false; //Flags to indicate errors
string line; 
Instruction instruction;
string currtextrecord;  
void pass1(){
// -----------------PASS 1 STARTS------------------------------
    fin.open("input.txt", ios::in);
    fout.open("intermediate.txt", ios::out);
    instruction = readline(line);

    //iterate till END is encountered
    while (instruction.OPCODE.compare("END"))
    {
        bool printed = true;

        //start
        if (instruction.OPCODE == "START")
        {
            STARTADDR = converthextointeger(instruction.OPERAND);
            LOCCTR = STARTADDR;
            controlsectcount++;

            controlsectlabels.push_back(instruction.LABEL);
            EXTREFTAB[controlsectlabels.back()]={controlsectcount - 1, controlsectlabels.back()};
            fout << "0" << convertintegertohex(LOCCTR, 4) << " ";
        }
        else if(instruction.OPCODE == "CSECT"){
                while (literallist.size())
                    {
                        string currliteral = literallist.back();
                        literallist.pop_back();
                        fout << "0" << convertintegertohex(LOCCTR, 4) << " ";
                        for(int i=0;i<10;i++){
                            fout << " ";
                        }
                        fout << "LITERAL";
                        for(int i=0;i<3;i++){
                            fout << " ";
                        }
                        fout << currliteral << "\n";
                        int literalsize = (int)currliteral.size()-3;
                        if (currliteral[0] == '='){
                            literalsize--;
                        }
                        if (currliteral[1] == 'X' || currliteral[0] == 'X'){
                            literalsize/=2;
                        }
                        if(currliteral[1]== 'C' || currliteral[0]=='C'){

                        }
                        LITTAB.InsertEntry(currliteral, 'R', controlsectcount-1, LOCCTR);
                        LOCCTR += literalsize;
                    }



                controlsectlengths.push_back(LOCCTR - STARTADDR);
            STARTADDR = converthextointeger(instruction.OPERAND);
            LOCCTR = STARTADDR;
            controlsectcount++;

            controlsectlabels.push_back(instruction.LABEL);
            EXTREFTAB[controlsectlabels.back()]={controlsectcount - 1, controlsectlabels.back()};
            fout << "0" << convertintegertohex(LOCCTR, 4) << " ";
        }
        //EXTDEF
        else if (instruction.OPCODE == "EXTDEF")
        {   int n=instruction.OPERAND.size();
            char c;
            string word;
            int i=0;
            while (i<n)
            {   c=instruction.OPERAND[i];
                if (c != ',')
                {
                    word += c;
                }
                else{
                    EXTREFTAB[word]={controlsectcount - 1, controlsectlabels.back()};
                    word = "";
                }
                i++;
            }
            if (word.size()){
                EXTREFTAB[word]={controlsectcount - 1, controlsectlabels.back()};
            }
        }

        else if (instruction.OPCODE.size() > 0 && instruction.OPCODE.compare(".") && instruction.OPCODE.compare("EXTREF"))
        {

            //LTORG
            if (instruction.OPCODE == "LTORG")
            {
                while (literallist.size())
                    {
                        string currliteral = literallist.back();
                        literallist.pop_back();
                        fout << "0" << convertintegertohex(LOCCTR, 4) << " ";
                        for(int i=0;i<10;i++){
                            fout << " ";
                        }
                        fout << "LITERAL";
                        for(int i=0;i<3;i++){
                            fout << " ";
                        }
                        fout << currliteral << "\n";
                        int literalsize = (int)currliteral.size()-3;
                        if (currliteral[0] == '='){
                            literalsize--;
                        }
                        if (currliteral[1] == 'X' || currliteral[0] == 'X'){
                            literalsize/=2;
                        }
                        if(currliteral[1]== 'C' || currliteral[0]=='C'){

                        }
                        LITTAB.InsertEntry(currliteral, 'R', controlsectcount-1, LOCCTR);
                        LOCCTR += literalsize;
                    }
                printed &= false;
            }

            //EQU
            else if (instruction.OPCODE == "EQU")
            {   int val=expreval(instruction.OPERAND).first;
                int addr = (instruction.OPERAND != "*")?val:LOCCTR;
                SYMTAB.InsertEntry(instruction.LABEL, 'A', controlsectcount - 1, addr);
            }
            else
            {
                fout << "0" << convertintegertohex(LOCCTR, 4) << " ";
                if (instruction.LABEL.size() > 0)
                {
                    if (SYMTAB.InsertEntry(instruction.LABEL, 'R', controlsectcount - 1, LOCCTR) == false)
                    {
                        DUPLABEL=true;
                    }
                }
                if (getvalfromoptab(instruction.OPCODE).first != -1){
                    LOCCTR += stoi(getvalfromoptab(instruction.OPCODE).second);
                    LOCCTR+=instruction.e;
                }else if (instruction.OPCODE == "WORD"){
                    LOCCTR += 3;
                }else if (instruction.OPCODE == "RESB"){
                    LOCCTR += stoi(instruction.OPERAND);
                }else if (instruction.OPCODE == "RESW"){
                    LOCCTR += 3 * stoi(instruction.OPERAND);
                }else if (instruction.OPCODE == "BYTE"){
                    int value=(int)instruction.OPERAND.length()-3;
                    if (instruction.OPERAND[0] == '='){
                        value--;
                    }
                    if (instruction.OPERAND[1] == 'X' || instruction.OPERAND[0] == 'X'){
                        value = value / 2;
                    }
                    if (instruction.OPERAND[1] == 'C' || instruction.OPERAND[0] == 'C'){

                    }
                    LOCCTR += value;
                }else
                {
                    INVOP=true;
                }

                if (instruction.checkLiteral && !LITTAB.isPresent(instruction.OPERAND, controlsectcount - 1))
                {
                    literallist.push_back(instruction.OPERAND);
                    LITTAB.InsertEntry(instruction.OPERAND, ' ', controlsectcount - 1, INT_MIN);
                }
            }
        }
        if (printed){
            fout << line;
            fout << endl;
        }
        instruction = readline(line);
    }
        while (literallist.size())
            {
                string currliteral = literallist.back();
                literallist.pop_back();
                fout << "0" << convertintegertohex(LOCCTR, 4) << " ";
                for(int i=0;i<10;i++){
                    fout << " ";
                }
                fout << "LITERAL";
                for(int i=0;i<3;i++){
                    fout << " ";
                }
                fout << currliteral << "\n";
                int literalsize = (int)currliteral.size()-3;
                if (currliteral[0] == '='){
                    literalsize--;
                }
                if (currliteral[1] == 'X' || currliteral[0] == 'X'){
                    literalsize/=2;
                }
                if(currliteral[1]== 'C' || currliteral[0]=='C'){

                }
                LITTAB.InsertEntry(currliteral, 'R', controlsectcount-1, LOCCTR);
                LOCCTR += literalsize;
            }

    controlsectlengths.push_back(LOCCTR - STARTADDR);

    fout << line << endl;

    fin.close();
    fout.close();
}
string currtextrecord1;
void pass2(){
// -----------------PASS 2 STARTS------------------------------
    fin.open("intermediate.txt", ios::in);
    fout.open("output.txt", ios::out);

    int controlsectionid = -1;
    string temp2 = "";
    vector<string> modificationrecords;
    instruction = readline(line);

    //Iterate till END is encountered
    while (instruction.OPCODE.compare("END"))
    {
        if (instruction.OPCODE == "START")
        {
            //New textrecord
            currtextrecord1="";
            currtextrecord1+="T^";
            currtextrecord1+=convertintegertohex(LOCCTR, 6);
            currtextrecord1+="^00";
            currtextrecord = "";
            currtextrecord.append("T");
            currtextrecord.append(convertintegertohex(LOCCTR, 6));
            currtextrecord.append("00");
            controlsectionid++;
            STARTADDR = converthextointeger(instruction.OPERAND);
            temp2 = "";
            if (instruction.OPERAND.size() > 0){
                temp2 = convertintegertohex(STARTADDR, 6);
            }

            //Header record
            int len=6 - (int)controlsectlabels[controlsectionid].size();
            int i=0;
            fout << "H^" << controlsectlabels[controlsectionid];
            while (i<len){
                fout << " ";
                i++;
            }
            fout<<"^";
            fout << convertintegertohex(STARTADDR, 6);
            fout <<"^"<< convertintegertohex(controlsectlengths[controlsectionid], 6);
            fout<<endl;
        }
        else if(instruction.OPCODE == "CSECT"){
                fout << currtextrecord1 << endl;
                for (int i=0;i<modificationrecords.size();i++)
                {   
                    int len=15 - (int)modificationrecords[i].length();
                    for(int j=0;j<len;j++)
                    {
                        modificationrecords[i] += ' ';
                    }
                    fout << "M" << modificationrecords[i] << endl;
                }
                modificationrecords.clear();
                if(temp2.length()!=0){
                    temp2="^"+temp2;
                }
                fout << "E"<<temp2<< "\n\n";
            //New textrecord
            currtextrecord1="";
            currtextrecord1+="T^";
            currtextrecord1+=convertintegertohex(LOCCTR, 6);
            currtextrecord1+="^00";
            currtextrecord = "";
            currtextrecord.append("T");
            currtextrecord.append(convertintegertohex(LOCCTR, 6));
            currtextrecord.append("00");
            controlsectionid++;
            STARTADDR = converthextointeger(instruction.OPERAND);
            temp2 = "";
            if (instruction.OPERAND.size() > 0){
                temp2 = convertintegertohex(STARTADDR, 6);
            }

            //Header record
            int len=6 - (int)controlsectlabels[controlsectionid].size();
            int i=0;
            fout << "H^" << controlsectlabels[controlsectionid];
            while (i<len){
                fout << " ";
                i++;
            }
            fout<<"^";
            fout << convertintegertohex(STARTADDR, 6);
            fout <<"^"<< convertintegertohex(controlsectlengths[controlsectionid], 6);
            fout<<endl;  
        }
        else if (instruction.OPCODE == "EXTDEF" || instruction.OPCODE == "EXTREF"){
            printreferdefinerecord( (instruction.OPCODE == "EXTDEF"),instruction.OPERAND, controlsectionid);
        }
        else if (instruction.OPCODE == "RSUB")
        {   string objCode="";
            objCode.append("4F0");
            objCode.append(convertintegertohex(STARTADDR, 3));
            if (converthextointeger(currtextrecord.substr(7, 2)) + converthextointeger(currtextrecord.substr(1, 6)) != LOCCTR || (int)objCode.size() + (int)currtextrecord.size() > 69)
            {
                fout << currtextrecord1 << "\n";
                currtextrecord1="";
                currtextrecord1+="T^";
                currtextrecord1+=convertintegertohex(LOCCTR, 6);
                currtextrecord1+="^00";
                currtextrecord = "";
                currtextrecord.append("T");
                currtextrecord.append(convertintegertohex(LOCCTR, 6));
                currtextrecord.append("00");
            }   
                currtextrecord1 += "^"+objCode; 
                currtextrecord += objCode;
                string textRecordSizeNewHex = convertintegertohex(converthextointeger(currtextrecord.substr(7, 2)) + ((int)objCode.size()) / 2, 2);
                currtextrecord[7] = textRecordSizeNewHex[0];
                currtextrecord[8] = textRecordSizeNewHex[1];
                currtextrecord1[9] = textRecordSizeNewHex[0];
                currtextrecord1[10] = textRecordSizeNewHex[1];
        }
        else if (instruction.OPCODE.size() > 0 && instruction.OPCODE.compare(".") && instruction.OPCODE.compare("EQU") && instruction.OPCODE.compare("RESB") && instruction.OPCODE.compare("RESW"))
        {
            string objCode = "";
            string type=getvalfromoptab(instruction.OPCODE).second;
            int opcode=getvalfromoptab(instruction.OPCODE).first;
            if (opcode != -1)
            {
                if (type == "2")
                {
                    objCode = convertintegertohex(opcode, 2);
                    char c;
                    int len=instruction.OPERAND.length();
                    for (int i=0;i<len;i++)
                    {   c=instruction.OPERAND[i];
                        if (c == ','){

                        }else{
                            objCode += REGTAB[c];
                        }
                    }
                    len=4-objCode.size();
                    int i=0;
                    while (i<len){
                        objCode += "0";
                        i++;
                    }
                }
                else
                {   vector<int>objCodeBinary(32);
                    //setting n,i,x,e flags
                    objCodeBinary[6] = instruction.n;
                    objCodeBinary[7] = instruction.i;
                    objCodeBinary[8] = instruction.x;
                    objCodeBinary[11] = instruction.e;
                    int opCodeDec = opcode;
                    opCodeDec/=4;
                    int opCodeSz=6;
                    int i=opCodeSz-1;
                    while(i>=0){
                        objCodeBinary[i] = opCodeDec % 2;
                        opCodeDec /= 2;
                        i--;
                    }
                    int addr = 0;
                    bool isMRecord = false;
                    bool PCaddr = true;
                    bool flag=checkConstantOperand(instruction.OPERAND);
                    if (flag){
                        addr = stoi(instruction.OPERAND);
                    }
                    else if (instruction.checkLiteral)
                    {
                        addr = LITTAB.findTargetAddr(controlsectionid, 3 + instruction.e, LOCCTR + 3 + instruction.e, BASE, instruction.OPERAND);
                        if (addr <= 2047)
                        {  
                            objCodeBinary[9] = 0;
                            objCodeBinary[10] = 1;
                        }
                        else
                        {
                            objCodeBinary[9] = 1;
                            objCodeBinary[10] = 0;
                            PCaddr = false;
                        }
                    }
                    else if (SYMTAB.isPresent(instruction.OPERAND, controlsectionid))
                    {
                        addr = SYMTAB.findTargetAddr(controlsectionid, 3 + instruction.e, LOCCTR + 3 + instruction.e, BASE, instruction.OPERAND);
                        if (addr<= 2047)
                        {
                            objCodeBinary[9] = 0;
                            objCodeBinary[10] = 1;
                        }
                        else
                        {
                            objCodeBinary[9] = 1;
                            objCodeBinary[10] = 0;
                            PCaddr = false;
                        }
                    }
                    else if (getvalfromextreftab(instruction.OPERAND).first != -1)
                    {   string modificationRecord="";
                        addr = 0;
                        isMRecord = true;
                        modificationRecord.append("^"+convertintegertohex(LOCCTR + 1, 6) + "^05^+" + instruction.OPERAND);
                        modificationrecords.push_back(modificationRecord);
                    }
                    else
                    {
                        INVLABEL=true;
                    }

                    if (instruction.e)
                    {   string modificationRecord = "";
                        objCodeBinary[9] = 0;objCodeBinary[10] = 0;
                        if (!isMRecord)
                        {   string temp1="";
                            temp1+=controlsectlabels[controlsectionid];
                            temp1="^05^+"+temp1;
                            temp1=convertintegertohex(LOCCTR + 1, 6)+temp1;
                            temp1="^"+temp1;
                            modificationRecord.append(temp1);
                            modificationrecords.push_back(modificationRecord);
                        }
                        int i=31;
                        while(i>0){
                            if(i<=12){

                            }else{
                            objCodeBinary[i] = addr % 2;
                            addr /= 2;
                            }
                            i--;                                
                        }
                    }
                    else
                    {
                        if (PCaddr)
                        {
                            bool flag1 = false;
                            if (addr < 0)
                            {
                                objCodeBinary[12] = 1;
                                addr = -1*addr;
                                flag1 = true;
                            }
                            for (int i = 23; i > 12; i--)
                            {   objCodeBinary[i] = addr % 2;
                                if (!flag1){

                                }else{
                                    objCodeBinary[i] = 1 - objCodeBinary[i];
                                }

                                addr /= 2;
                            }
                            if (flag1)
                            {
                                int y = 0;
                                int i=23;
                                while(i>0){
                                    if(i<=12){

                                    }else{
                                        if (objCodeBinary[i] == 0)
                                        {
                                            objCodeBinary[i] = 1;
                                            break;
                                        }
                                        objCodeBinary[i] = 0;
                                    }
                                    i--;                                
                                }
                            }
                        }
                        else
                        {   

                        }
                    }
                    int instrlen;
                    if(instruction.e){
                        instrlen=32;
                    }else{
                        instrlen=24;
                    }
                    objCode = binaryToHex(objCodeBinary,instrlen);
                }
            }
            else if(instruction.OPCODE == "LITERAL"){
                int val=(int)instruction.OPERAND.size() - 3;
                if (instruction.OPERAND[0] == 'X'){
                    objCode += instruction.OPERAND.substr(2,val);
                
                }else if (instruction.OPERAND[0] == 'C')
                {   
                    string temp1=instruction.OPERAND.substr(2,val);
                    int i = 0;
                    while (i< temp1.size())
                    {
                        objCode += convertintegertohex((int)temp1[i], 2);
                        i++;
                    }
                }
            }else if (instruction.OPCODE == "BYTE")
            {   int val=(int)instruction.OPERAND.size() - 3;
                if (instruction.OPERAND[0] == 'X'){
                    objCode += instruction.OPERAND.substr(2,val);
                
                }else if (instruction.OPERAND[0] == 'C')
                {   
                    string temp1=instruction.OPERAND.substr(2,val);
                    int i = 0;
                    while (i< temp1.size())
                    {
                        objCode += convertintegertohex((int)temp1[i], 2);
                        i++;
                    }
                }
            }else if (instruction.OPCODE == "WORD")
            {   int val=expreval(instruction.OPERAND).first;
                objCode = convertintegertohex(val, 6);
                vector<string>v=expreval(instruction.OPERAND).second;
                int i=0,n=v.size();
                while(i<n)
                {  
                    string mRecord = "^"+convertintegertohex(LOCCTR, 6) + "^06^" + v[i];
                    modificationrecords.push_back(mRecord);
                    i++;
                }
            }
            else
            {
                INVOP=true;
            }

            if (converthextointeger(currtextrecord.substr(7, 2)) + converthextointeger(currtextrecord.substr(1, 6)) != LOCCTR || (int)objCode.size() + (int)currtextrecord.size() > 69)
            {
                fout << currtextrecord1 << endl;
                currtextrecord1="";
                currtextrecord1+="T^";
                currtextrecord1+=convertintegertohex(LOCCTR, 6);
                currtextrecord1+="^00";
                currtextrecord = "";
                currtextrecord.append("T");
                currtextrecord.append(convertintegertohex(LOCCTR, 6));
                currtextrecord.append("00");
            }
                currtextrecord1 += "^"+objCode;
                currtextrecord += objCode;
                string textRecordSizeNewHex = convertintegertohex(converthextointeger(currtextrecord.substr(7, 2)) + ((int)objCode.size()) / 2, 2);
                currtextrecord[7] = textRecordSizeNewHex[0];
                currtextrecord[8] = textRecordSizeNewHex[1];          
                currtextrecord1[9] = textRecordSizeNewHex[0];
                currtextrecord1[10] = textRecordSizeNewHex[1];
        }
        instruction = readline(line);
    }
    fout << currtextrecord1 << endl;
    for (int i=0;i<modificationrecords.size();i++)
    {   
        int len=15 - (int)modificationrecords[i].length();
        for(int j=0;j<len;j++)
        {
            modificationrecords[i] += ' ';
        }
        fout << "M" << modificationrecords[i] << endl;
    }
    modificationrecords.clear();
    if(temp2!=""){
        temp2="^"+temp2;
    }
    fout << "E"<<temp2<< "\n\n";

    fin.close();
    fout.close();

}

int main()
{
    //Initialize OPTAB and REGTAB
    InitializeOPTAB();
    pass1();
    pass2();
    
    if (!DUPLABEL && !INVOP && !INVLABEL)
    {  

        cout << "Successful Execution"<<endl;
    }
    else
    {
        if (DUPLABEL){
            cout << "Duplicate LABEL has been found"<<endl;
        }
        if (INVOP){
            cout << "Invalid OPCODE has been found"<<endl;
        }
        if (INVLABEL){
            cout << "Invalid LABEL has been found"<<endl;
        }
    }
    return 0;
}