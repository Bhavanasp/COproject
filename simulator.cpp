#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <vector>

using namespace std;

string ropf[6] = {"add","sub","slt","jr","sll","srl"};
string iopf[6] = {"lw","sw","beq","bne","addi","slti"};
int func[6] = {0x20,0x22,0x2a,0x08,0x00,0x02};
int iop[6] = {0x23,0x2b,0x04,0x05,0x0c,0x0a};
string regno[32] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                    "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                    "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                    "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};
vector<string>label;
vector<int>label_i;
int text[400];
int dataseg[600];
string instarray[100];
int instcnt = 0;

void printdata(){
    int y;
    cout<<"-----------------------------------------------------DATA SEGMENT------------------------------------------------------";
    for(y = 0;y<600;y++){
        if(y%60==0){
            cout<<endl;
        }
        cout<<dataseg[y]<<" ";
    }
    cout<<"\n-----------------------------------------------------------------------------------------------------------------------"<<endl;
}


int getIndex(auto a[],auto st,int s){
      int i;
      for(i=0;i<s;i++){
          if(a[i]==st){
              return i;
          }
      }
      return -1;
}

int regcheck(string st){
      int l,regn;
      if(st=="$0"){
	   return 0;
      }
      regn = getIndex(regno,st,32);
      if(regn!=-1){
          return regn;    
      }
      else{
          cout<<"error : invalid register "<<st<<" in line no \""<<instarray[instcnt]<<"\""<<endl;
          exit(0);
      }
}

int getTarget(string st,int instcnt,int o){ 
    int h;
    for(h = 0;h<label.size();h++){
        if(label[h]==st){
            if(o==0){
              return label_i[h]-instcnt-1;
            }
            else if(o==1){
              return label_i[h];
            }
        }
    }
    if(st!="main"){
        cout<<"error : No label \'"<<st<<"\' in line no : "<<instcnt<<" \""<<instarray[instcnt]<<"\""<<endl;
    }
    else{
        cout<<"error : main not found"<<endl;
    }
    exit(0);
}

class converter{

    public :
    int opc = 0,inst = 0,imm;
    string op;

    void convert(string temp[]){
        op = temp[0];
        if(getIndex(ropf,op,6)!=-1){
            opc = 0x00;
            if(op=="jr"){
                instarray[instcnt] = op+" "+temp[1];
                inst = ((regcheck(temp[1])&0x03ffffff)<<21)+func[getIndex(ropf,op,6)];
            }
            else if(op=="sll"||op=="srl"){
                instarray[instcnt] = op+" "+temp[1]+","+temp[2]+","+temp[3];
                try{ stoi(temp[3]); }catch(exception e){ cout<<"error : shift amount not an integer in line no. "<<instcnt<<" "<<instarray[instcnt]<<endl ; exit(0);};
                if(stoi(temp[3])<0||stoi(temp[3])>31){
                    cout<<"error : shift amount value out of bounds in line no. "<<instcnt<<" "<<instarray[instcnt]<<endl;
                    exit(0);
                }
                inst = opc + (regcheck(temp[2])<<21) + (regcheck(temp[1])<<16) + (((stoi(temp[3]))&(0x0000001f))<<6) + func[getIndex(ropf,op,6)];
            }
            else{
                instarray[instcnt] = op+" "+temp[1]+","+temp[2]+","+temp[3];
                inst = opc + (regcheck(temp[2])<<21) + (regcheck(temp[3])<<16) + (regcheck(temp[1])<<11) + func[getIndex(ropf,op,6)];
            }
        }
        else if((getIndex(iopf,op,6))!=-1){
            opc = iop[getIndex(iopf,op,6)];
            inst = opc<<26;
                if(op=="lw"||op=="sw"){
                    instarray[instcnt] = op+" "+temp[1]+","+temp[2]+"("+temp[3]+")";
                    try{stoi(temp[2]);}
                    catch(exception e){cout<<"error : immediate value not an integer in line no. "<<instcnt<<" "<<instarray[instcnt]<<endl;exit(0);};
                    imm = (int)strtol(temp[3].c_str(), 0, 16);
	                inst = inst + (regcheck(temp[3])<<21) + (regcheck(temp[1])<<16) + (((int)strtol(temp[2].c_str(), 0, 16))&(0x0000ffff));
	        }
	        else if(op=="beq"||op=="bne"){
	            instarray[instcnt] = op+" "+temp[1]+","+temp[2]+" "+temp[3];
	            inst = inst + (regcheck(temp[1])<<21) + (regcheck(temp[2])<<16) + ((getTarget(temp[3],instcnt,0))&(0x0000ffff));
	        }
	        else{
	            instarray[instcnt] = op+" "+temp[1]+","+temp[2]+","+temp[3];
	            try{stoi(temp[3]);}
	            catch(exception e){cout<<"error : immediate value not an integer in line no. "<<instcnt<<" "<<instarray[instcnt]<<endl;exit(0);};
	            imm = (int)strtol(temp[3].c_str(), 0, 16);
	            inst = inst + (regcheck(temp[2])<<21) + (regcheck(temp[1])<<16) + (((int)strtol(temp[3].c_str(), 0, 16))&(0x0000ffff));
	        }
	        if((op!="bne"&&op!="beq")&&(imm>32767||imm<-32768)){
                       cout<<"error : immediate value out of bounds in line no. "<<instcnt<<" "<<instarray[instcnt]<<endl;
                       exit(0);
            }
        }
        else if(op=="j"){
             opc = 0x0c000000;
             instarray[instcnt] = op+" "+temp[1];
	         inst = opc + getTarget(temp[1],instcnt,1) ;
        }
        else{
	         cout<<"error : invalid instruction \""<<op<<"\" in line "<<instcnt<<endl;
	         exit(0);
        }
        temp[0] = "\0";temp[1] = "\0";temp[2] = "\0";temp[3] = "\0";
        cout<<instcnt<<" "<<instarray[instcnt]<<endl;
        cout<<endl;
        text[instcnt] = inst;
        instcnt++;
    }
};

class parser{

    public :
        string buff[100];
        string st = "\0";
        int instcnt = 0;
 
    void tknizer(fstream &file,converter c){
        string temp,t;
        char *token;
        int i = 0,j,k = 0,cnt = 0,f = 0;
        while(!file.eof()){
            i = 0;
            getline(file,temp);

            temp.erase(remove(temp.begin(),temp.end(),'\r'),temp.end());
            temp.erase(remove(temp.begin(),temp.end(),'\t'),temp.end());

            if(temp.length()!=0){
                t = "\0";
                if(temp.find(":")!=-1){
                    for(i=0;i<temp.length();i++){
                        if(temp.at(i)!=':'&&temp.at(i)!='#'){
                            if(temp.at(i)!=' '){
                                t = t + temp.at(i);
                            }
                        }
                        else{
                            if(t.length()!=0){
                                label.push_back(t);
                                label_i.push_back(cnt);
                            }
                            break;
                        }
                    }
        	}
                if(i!=temp.length()){
                    temp = temp.substr(i,temp.length());
                    if(temp.length()!=0){
                        char st[temp.length()+1];
                        strcpy(st,temp.c_str());
                        token = strtok(st,"(),: ");                    
                        if((token!=NULL)){
                            temp = token;
                            if((strcmp(token,".word")!=0)&&(temp.at(0)!='#')){
                                cnt++;
                            }
                        }
                    }
                }
            }
        }

        file.clear();
        file.seekg(0,ios::beg);

        while(!file.eof()){

            getline(file,temp);
            temp.erase(remove(temp.begin(),temp.end(),'\r'),temp.end());

                if(temp.length()!=0){

                    char st[temp.length()+1];
                    strcpy(st,temp.c_str());
                    i = 0;
                    int f = 0;

                    if(temp.find("#")!=-1){
                        while(i<temp.length()){
                            if(temp.at(i)=='#'&&f==0){
                                f = 1;
                            }
                            if(f==1){
                                st[i] ='#';
                            }
                            i++;
                        }
                    }

                    i = 0;

                    if(temp.find(":")!=-1){
                        while(temp.at(i)!=':'){
                            st[i] = '#';
                            i++;
                        }
                    }

                    i = 0,j = 0;
                    token = strtok(st," #\t(),:");

                    if(token!=NULL){
                        if(strcmp(token,".word")==0){
                		    while(token!=NULL){
                		        if(strcmp(token,".word")!=0){
                		            dataseg[k] = atoi(token);
                    		        k++;
                		        }
                    		    token = strtok(NULL," #\t(),:");
                		     }
                        }
                        else{
                            while(token!=NULL){
                                buff[j] = token;
                                token = strtok(NULL," #\t(),:");
                                j++;
                            }
                        }
                    }
                    else{
                        buff[0] = "\0";
                    }
                }
            else{
                buff[0] = "\0";
            }
            if(buff[0] != "\0"){
               c.convert(buff);
            }
        }
    }
};

class processor{

    public :
        int reg[32] = {0};
        int *pc,*regd;
        int regs1,regs2,result,imm,ifreg,memreg,wbreg;
        int fnc,select = -1,iselect = -1,end = 0,r;
    
    void setpc(int main_i){
        pc = text+main_i;
    }   
    
    void fetch(int o){
        ifreg = text[pc-text];
        if(o==1){
            cout<<"[line no :"<<pc-text<<"] "<<instarray[pc-text]<<" ";
            printf("[instructin in hexadecimal : 0x%x ] executing \n\n",ifreg);
        }
        pc++;
    }

    void idrf(){
        int opc = ((unsigned int)ifreg)>>26;
        regs1 = reg[(0x03e00000&ifreg)>>21];
        regs2 = reg[(0x001f0000&ifreg)>>16];
        if(opc==0){
            fnc = (0x3f)&ifreg;
            select = getIndex(func,fnc,6);
            if(select==4||select==5){
                regs2 = (ifreg&(0x000007c0))>>6;
                regd = reg+((0x001f0000&ifreg)>>16);
            }
            else{
                regd = reg+(((0x0000f800)&ifreg)>>11);
            }
        }
        else if(opc==3){
             select = -1;
             pc = text+((0x03ffffff)&(ifreg));
        }
        else{
            iselect = getIndex(iop,opc,6);
            select = 0;
            imm = (ifreg)&(0x0000ffff);
            if(((ifreg&(0x00008000))>>15)==1){
                     imm = imm + 0xffff0000;
            }
            if(iselect==5){ select = 2;}
            
            if((iselect==2&&regs1==regs2)||(iselect==3&&regs1!=regs2)){
                regs2 = imm;
                pc = pc+regs2;
            }
            else{
                regs2 = imm;
                if(iselect==0||iselect==1){
                    r = regs1%4 + regs2%4;
                    if(r!=0){
                        cout<<"error : unaligned address access in line no. "<<pc-text-1<<" "<<instarray[pc-text-1]<<endl;
                        exit(0);
                    }
                }
                regd = reg+(((0x001f0000)&ifreg)>>16);
            }
        }
        
    }
    
    void execute(){
        switch(select){
            case 0:if(iselect!=2&&iselect!=3&&end==0){ result = regs1+regs2; }; break;
            case 1:result = regs1-regs2; break;
            case 2:result = ((regs1<regs2)?(1):(0));break;
            case 3:end = 1; break;
            case 4:result = regs1<<regs2; break;
            case 5:result = ((unsigned int)regs1)>>regs2; break;
            default : break;
        }
    }
    
    void memrw(){
        if(iselect==0){
            if((result/4)<600){
                memreg = dataseg[result/4];
            }
            else{
                cout<<"error : address out of bounds in line no. "<<pc-text-1<<" "<<instarray[pc-text-1]<<endl;
                exit(0);
            }
        }
        else if(iselect==1){
            if((result/4)<600){
                dataseg[result/4] = *regd;
                memreg = dataseg[result/4];
            }
            else{
                cout<<"error : address out of bounds in line no. "<<pc-text-1<<" "<<instarray[pc-text-1]<<endl;
                exit(0);
            }
        }
        else {
            if(end==0){
                memreg = result;
            }
        }
    }
    
    bool wb(){
        if(((iselect<1)||(iselect>3))&&select!=-1&&end==0){
   		 *regd = memreg;
        }
        if(end==1){
	         return true;
     	}
        iselect = -1;
        select = -1;
        return false;
    }
    
    void print(){
        cout<<"---------------------------------------------------REGISTERS-----------------------------------------------------------"<<endl;
        cout<<endl<<"pc = "<<pc<<endl<<endl;
        for(int u = 0;u<32;u++){
          cout<<regno[u]<<" = "<<reg[u]<<endl;
        }
        cout<<"-----------------------------------------------------------------------------------------------------------------------"<<endl;
    }
};

int main(){
    int o = 1;
    bool b = false;
    parser p;
    converter c;
    processor pr;
    fstream file;
    string file_name;
    cout<<"Enter the file name: ";
    cin>>file_name;
    file.open(file_name.c_str(),ios::in);
    if(file){
        
        cout<<"---------------------------------------------------TEXT----------------------------------------------------------------"<<endl;
    
        p.tknizer(file,c);
        
        cout<<"-----------------------------------------------------------------------------------------------------------------------"<<endl;
        
        cout<<"---------------------------------------------------LABELS--------------------------------------------------------------"<<endl;
        
        for(int i = 0;i<label.size();i++){
            cout<<"\'"<<label[i]<<"\'"<<" in line "<<label_i[i]<<endl;
        }
        cout<<"-----------------------------------------------------------------------------------------------------------------------"<<endl;
        
        pr.setpc(getTarget("main",0,1));
        
        printdata();
        pr.print();
        cout<<endl;
        
        while(true){
            if(o==1){
                cout<<"\n1. single step"<<endl;
                cout<<"2. run/continue"<<endl;
                cout<<"3. exit\n"<<endl;
                cout<<"ENTER OPTION : ";
                cin>>o;
                cout<<endl;
            }
            if(o==1||o==2){
                pr.fetch(o);
                pr.idrf();
                pr.execute();
                pr.memrw();
                b = pr.wb();
                if(o==1){
                    printdata();
                    pr.print();
                    cout<<endl;
                }
                if(b){
		    break;
		}
            }
            if(o==3){
                exit(0);
            }
            if(o<1||o>3){
                cout<<"invalid option, try again "<<endl;
		        o = 1;
            }
        }
        if(o==2){
            printdata();
            pr.print();
            cout<<endl;
        }
    }
    else{
        cout<<"error : file not accessible"<<endl;
    }
    file.close();
}
