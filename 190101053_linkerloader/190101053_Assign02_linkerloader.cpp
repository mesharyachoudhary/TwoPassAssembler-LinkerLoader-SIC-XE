#include <bits/stdc++.h>
using namespace std; 		

fstream fin;
fstream fout1;  
fstream fout2; 


//ESTAB
vector<pair<string,pair<int,int>>> EStable;	

vector<pair<int,string>> memoryobjcode;	//stores the memory content byte wise : address mapped to memoryobjcode

//searches the vector for index
int findIndexBySymbolName(vector<pair<string,pair<int,int>>> table,string symbol){
    int exist=-1;
	int i=0,n=table.size();
	while(i<n){
		if(table[i].first==symbol){
			exist=i;
			break;
		}
		i++;
	}
	return exist;
}

//search the vector for index
int findIndexBySymbolAddr(vector<pair<int,string>> objcode,int addr){
    int exist=-1;
	int i=0,n=objcode.size();
	while(i<n){
		if(objcode[i].first==addr){
			exist=i;
			break;
		}
		i++;
	}
	return exist;
} 
//convert an integer to a hex value of size len
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

vector<string> readline(string str) { 
	vector <string> words;
	string temp="";
	for(int i=0;i<str.length();i++){
		if(str[i]!=' '){
            temp+=str[i];
		}else{
			if(temp!=""){
			words.push_back(temp);
			}
			temp="";
		}
	}
	if(temp!=""){
    words.push_back(temp);
	temp="";
	}
	return words;
} 
int csecaddr; //control section address
int progaddr; //program address 
int execaddr; //execution address

void DTypeRecords(vector<string> records){
    int i=0,n=records[0].length();
	string temp;
	temp = "";
	while(i<n){
		if(i<1){

		}else{
		temp+= records[0][i];
		}
		i++;
	}
	records[0] = temp;
	string esname;
	esname = records[0];
	n = records.size()-1;
	i=0;
	while(i<n){
		string es_addr;
		es_addr = "";
		int j=0;
		while(j<6){
			es_addr += records[i+1][j++];
		}
		if(findIndexBySymbolName(EStable,esname)!=-1){
			cout<<"ERROR: DUPLICATE SYMBOL PRESENT"<<endl;
			exit(0);
		}else{
			int new_addr = converthextointeger(es_addr);
			new_addr+=csecaddr ;
			pair<int,int>new_es={new_addr,0};
			EStable.push_back({esname,new_es});
			fout1<<"    \t\t";
			fout1<<esname;
			fout1<<" \t\t ";
			fout1<<convertintegertohex(new_es.first,4);
			fout1<<" \t\t "<<endl;

		}
		if(i!=(n-1)){
			esname="";
			int j=0,n1=records[i+1].length();
			while(j<n1){
			   if(j<6){

			   }else{
				esname+=records[i+1][j];
			   }
			   j++;
			}
		}
		i=i+1;
	}
}
int TTypeRecords(string records){
	string addr="";
	string temp=addr;
	int i=1,n=7;
	while(i<n){
		temp+=records[i];
		i++;
	}
	addr=temp;
	int location = csecaddr;
	location += converthextointeger(addr);
	i=9,n=records.length();
	while(i<n){
		temp = "";
		temp+=records[i];
		temp+=records[i+1];
		location=location+1;
		memoryobjcode.push_back({location-1,temp});
		i+=2;
	}
	return 1;
}

string ModificationType(string old_val,string symname,string op){
	string temp;
	temp = "";
	int new_val;
	if(old_val[0]=='F'){
		long int overflow_handler = (long int)0xFFFFFFFF000000;
		new_val = overflow_handler;
	}
	if(old_val[0]!='F'){
		new_val = 0;
	}
	new_val += converthextointeger(old_val); 
	new_val +=(op.back()=='+'?1:-1)*EStable[findIndexBySymbolName(EStable,symname)].second.first;
	string new_hex = convertintegertohex(new_val,6);
	if(new_hex.length()>6 && new_hex[0]=='F' && new_hex[1]=='F'){
		int i=2,n=new_hex.length();
		while(i<n){
			temp+= new_hex[i++];
		}
		new_hex = temp;
	}
	return new_hex;
}

void MTypeRecords(string records){
	string symname="";
	string symloc=symname;
	string symlen=symloc;

	int i=1,n=records.length();
	while(i<n){
		if(i<=6){
			symloc+=records[i++];
		}else if(i>9){ 
			symname+=records[i++];
		}else if(i>6 && i<=9){
			symlen+=records[i++];
		}
	}
	int symbolAddr = converthextointeger(symloc);
	symbolAddr += csecaddr;
	string old_val="";
	vector<int>ind(3);
	i=0,n=3;
    while(i<n){
		if(findIndexBySymbolAddr(memoryobjcode,symbolAddr+i)>=0){
		ind[i]=findIndexBySymbolAddr(memoryobjcode,symbolAddr+i);
		old_val+=memoryobjcode[ind[i]].second;
		}else{

		}
		i++;
	}
	string str_hex = ModificationType(old_val,symname,symlen);
	int k=0;
	n=3;
	i=0;
	while(i<n){
		if(ind[i]==-1){
			string temp1="";
			temp1+=str_hex[k];
			temp1+=str_hex[k+1];
			memoryobjcode.push_back({symbolAddr+i,""});
			memoryobjcode.back().second+=temp1;
			k+=2;i++;
		}else{
			string temp1="";
			temp1+=str_hex[k];
			temp1+=str_hex[k+1];
			memoryobjcode[ind[i]].second=temp1;
			k+=2;i++;
		}
	}
}

void pass1(){
	string current_secname=""; 	
	fin.open("input.txt",ios::in);
    vector<string> line_w;
	//construct ESTAB
	fout1.open("ESTAB.txt",ios::out);
	fout1<<"ControlSec\tSymbolName   Addr    \t Length\n";
	//assign start address for relocation
	csecaddr=progaddr; 
	int cs_length=0;
	string line="";
	while(getline(fin,line,'\n')){
        line_w.clear();
		line_w=readline(line);
		if(line_w[0][0]=='H'){
            vector<string> records;
			string csLen="";
			string csecaddr_obj="";
            int i=0,n=6;
			while(i<n){
				csecaddr_obj+=line_w[1][i++];
			}
            i=0,n=line_w[1].length();
			while(i<n){
                if(i<6){

				}else{
                   csLen += line_w[1][i];
				}
				i++;
			}
			i=0,n=line_w[0].length();
			string temp;
			temp = "";
			while(i<n){
				if(i<1){

				}else{
				temp+= line_w[0][i];
				}
				i++;
			}
			line_w[0] = temp;
			if(findIndexBySymbolName(EStable,line_w[0])>=0){
				cout<<"ERROR : DUPLICATE SECTION FOUND\n";
			}else{
				current_secname=line_w[0];
				int addR = converthextointeger(csecaddr_obj);
				addR += csecaddr;
				pair<int,int>new_es={addR,converthextointeger(csLen)};
				EStable.push_back({line_w[0],new_es});

				fout1<<line_w[0];
				fout1<<" \t\t  \t\t\t ";
                fout1 << convertintegertohex(new_es.first,4);
				fout1<<" \t\t ";
                fout1<< convertintegertohex(new_es.second,4);
				fout1<<" \t\t \n";
			}

			while(getline(fin,line,'\n')){
				records.clear();
				records=readline(line);     
				if(records[0][0]=='D'){ 
					//define record
					DTypeRecords(records);
				}else if(records[0][0]=='E'){
					//end record 
					break;
				}else{

				}
			}
			csecaddr += converthextointeger(csLen);
		}
	}
	fin.close();
	fout1.close();
}

void pass2(){
    csecaddr=progaddr;
	execaddr=progaddr;
    vector<string> line_w;
	fin.open("input.txt",ios::in);
	int cs_len;
	string line="";
	while(getline(fin,line,'\n')){
        line_w.clear();
		line_w=readline(line);
		if(line_w[0][0]=='H'){
			string temp;
			temp = "";
			int i=0;
			int n=line_w[0].length();
			while(i<n){
				if(i<1){

				}else{
				temp+= line_w[0][i];
				}
				i++;
			}
			line_w[0] = temp;
            vector<string> records;
			string current_secname="";
			cs_len=EStable[findIndexBySymbolName(EStable,line_w[0])].second.second;
			while(getline(fin,line,'\n')){
				records.clear();
				records=readline(line);
				if(records[0][0]=='T'){
					//text record 
					TTypeRecords(records[0]);
				}else if(records[0][0]=='M'){
					//modification record
					MTypeRecords(records[0]);
				}else if(records[0][0]=='E'){
					break;
				}
			}

			if(line[0]=='E'){ 
				if(line.length()>1){
					string temp;
					temp = "";
					int i=0;
					int n=line.length();
					while(i<n){
						if(i<1){

						}else{
						temp+= line[i];
						}
						i++;
					}
					line = temp;
					execaddr+=(csecaddr);
					execaddr+=(converthextointeger(line)); 
				}
			csecaddr+=(cs_len);
			}else{
			csecaddr+=(cs_len);
			}
		}

	}
	fin.close();
}

void writeoutput(){
	fout2.open("output.txt",ios::out);
	int addr_counter= progaddr;
	addr_counter-=16;
    auto it=memoryobjcode.rbegin();
	int addrEnd =(*it).first;
	int i=0;
	while(addr_counter!=addrEnd+1){
		int index=0;
		if(i%16==0){
			fout2<<convertintegertohex(addr_counter,4)<<" \t ";
		}
        string temp1="";
		if(addr_counter<progaddr){
            temp1="xx";
			fout2<<temp1;
			if((i+5)%4==0){
				fout2<<"\t";
			}else{

			}

		}else if((findIndexBySymbolAddr(memoryobjcode,addr_counter))>=0 && memoryobjcode[index].second.length()!=0){
			index=findIndexBySymbolAddr(memoryobjcode,addr_counter);
			temp1=memoryobjcode[index].second;
			fout2<<temp1;
			if((i+5)%4==0){
				fout2<<" \t";	
			}else{

			}	
		}else{
			temp1="..";
			fout2<<temp1;
			if((i+5)%4==0){
				fout2<<"\t";
			}else{

			}
		}
		if((i+1)%16==0){
			fout2<<endl;
		}else{
			
		}
		addr_counter++;
		i++;
	}

	while((progaddr-addr_counter)%16!=0){
        string temp1="xx";
		fout2<<temp1;
		if((i+1)%4==0){
			fout2<<"\t";
			i++;
		    addr_counter++;
		}else{
		i++;
		addr_counter++;
		}

	}
	fout2.close();
}


int main(){
	progaddr=converthextointeger("4000");
	pass1();
	pass2();
	writeoutput();
}