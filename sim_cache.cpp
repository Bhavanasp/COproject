#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <cmath>

using namespace std;

float clk = 0;
float i_cnt = 0;
float stall = 0;

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
int dataseg[1024];
string instarray[400];
int instcnt = 0;

void printdata(){
    int y;
    cout<<"---------------------------------------------------------DATA SEGMENT----------------------------------------------------------";
    for(y = 0;y<1024;y++){
        if(y%64==0){
            cout<<endl;
        }
        cout<<dataseg[y]<<" ";
    }
    cout<<"\n-------------------------------------------------------------------------------------------------------------------------------"<<endl;
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
          cout<<"error : invalid register "<<st<<" in line no. "<<instcnt<<", \""<<instarray[instcnt]<<"\""<<endl;
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
        cout<<"error : No label \'"<<st<<"\' in line no. : "<<instcnt<<", \""<<instarray[instcnt]<<"\""<<endl;
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
                    imm = stoi(temp[2]);
                    inst = inst + (regcheck(temp[3])<<21) + (regcheck(temp[1])<<16) + ((imm)&(0x0000ffff));
            }
            else if(op=="beq"||op=="bne"){
                instarray[instcnt] = op+" "+temp[1]+","+temp[2]+" "+temp[3];
                inst = inst + (regcheck(temp[1])<<21) + (regcheck(temp[2])<<16) + ((getTarget(temp[3],instcnt,0))&(0x0000ffff));
            }
            else{
                instarray[instcnt] = op+" "+temp[1]+","+temp[2]+","+temp[3];
                try{stoi(temp[3]);}
                catch(exception e){cout<<"error : immediate value not an integer in line no. "<<instcnt<<" "<<instarray[instcnt]<<endl;exit(0);};
                imm = stoi(temp[3]);
                inst = inst + (regcheck(temp[2])<<21) + (regcheck(temp[1])<<16) + ((imm)&(0x0000ffff));
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

class block{
    public :
        int tag = 0x0;
        int *blk;
        int age = 0;
        bool dbit = false;
        bool vbit = false;
        int s;
    void setBlockSize(int s){
        this->s = s;
        blk = new int[s];
    }
    bool isDirty(){
        if(dbit){
            return true;
        }
        return false;
    }
    bool isValid(){
        if(vbit){
            return true;
        }
        return false;
    }
};

class set{
    public :
        block *st;
        int ac;
        int s;
    int setSetSize(int ac,int s){
        this->ac = ac;
        this->s = s;
        st = new block[ac];
        for(int i = 0;i<s;i++){
            (st[i]).setBlockSize(s);
        }
    }
    int searchTag(int tag){
        for(int i = 0;i<ac;i++){
            if(((st[i]).tag)==tag&&(st[i].isValid())){
                return i;
            }
        }
        return -1;
    }
    int getFreeBlk(){
        for(int i = 0;i<ac;i++){
            if(!((st[i]).isValid())){
                return i;
            }
        }
        return -1;
    }
    int setage(int mru){
        for(int i = 0;i<ac;i++){
            if(st[i].isValid()){
                (st[i].age)++;
            }
        }
        st[mru].age = 0;
    }
    int getMaxAge(){
        int maxAge = 0;
        for(int i = 0;i<ac;i++){
            if(maxAge<(st[i].age)){
                maxAge = st[i].age;
            }
        }
        return maxAge;
    }
    int getLRU(){
        for(int i = 0;i<ac;i++){
            if(st[i].age==getMaxAge()){
                return i;
            }
        }
    }
};

class Cache{
    public :
        set *ch;
        int tag,index,offset;
        int ibits,obits;
        int found;
        int lru;
        int freeBlk;
        int nb,ac,s;
        int noOfSets;
        int lruBlkAd;

        void setCacheParam(int nb,int ac,int s){
            this->nb = nb;
            this->ac = ac;
            this->s = s;

            noOfSets = nb/(ac*s*4);
            obits = log(s)/log(2);
            ibits = log(noOfSets)/log(2);
            ch = new set[noOfSets];

            for(int i = 0;i<noOfSets;i++){
                ch[i].setSetSize(ac,s);
            }
        }

        int find(int ad,bool processreq){

            tag = ad>>(ibits+obits);
            index = ((unsigned int)(ad<<(32-(ibits+obits))))>>(32-ibits);
            offset = ((unsigned int)(ad<<(32-obits)))>>(32-obits);

            if(ibits==0){ index = 0;}            

            found = ch[index].searchTag(tag);

            if((found!=-1)&&(processreq)){
                ch[index].setage(found);
            }

            return found;

        }

        void insert(int ad,int *incarrier,int *outcarrier,bool *dbi,bool *dbo){

            tag = ad>>(ibits+obits);
            index = ((unsigned int)(ad<<(32-(ibits+obits))))>>(32-ibits);
            offset = ((unsigned int)(ad<<(32-obits)))>>(32-obits);

            if(ibits==0){ index = 0;}

            freeBlk = ch[index].getFreeBlk();

            if(freeBlk!=-1){
                for(int i = 0;i<s;i++){
                    ch[index].st[freeBlk].blk[i] = incarrier[i];
                }
                ch[index].st[freeBlk].tag = tag;
                ch[index].st[freeBlk].vbit = true;
                ch[index].st[freeBlk].dbit = *dbi;
                ch[index].setage(freeBlk);
                *dbo = false;
            }
            else{
                lru = ch[index].getLRU();
                if(ch[index].st[lru].isDirty()){
                    for(int i = 0;i<s;i++){
                        outcarrier[i] = ch[index].st[lru].blk[i];
                    }
                    *dbo = true;
                    lruBlkAd = ((ch[index].st[lru].tag)<<(ibits+obits))+(index<<obits);
                }
                else{
                    *dbo = false;
                }
                for(int i = 0;i<s;i++){
                    ch[index].st[lru].blk[i] = incarrier[i];
                }
                ch[index].setage(lru);
                ch[index].st[lru].tag = tag;
                ch[index].st[lru].vbit = true;
                ch[index].st[lru].dbit = *dbi;
            }
        }

        int update(int *incarrier,int ad,int found,int s1,int s2){

            tag = ad>>(ibits+obits);
            index = ((unsigned int)(ad<<(32-(ibits+obits))))>>(32-ibits);
            offset = ((unsigned int)(ad<<(32-obits)))>>(32-obits);
            
            if(ibits==0){ index = 0;}

            for(int i = 0;i<s1;i++){
                ch[index].st[found].blk[(ad%s2)-(ad%s1)+i] = incarrier[i];
            }

            ch[index].st[found].dbit = true;
        }
};

class CacheCtrl{
    public :
        Cache L1cache;
        Cache L2cache;
        int *inblock1 = NULL;
        int *inblock2 = NULL;
        int *outblock1 = NULL;
        int *outblock2 = NULL;
        bool dbi = false,dbo = false;
        int nb1,ac1,s1;
        int nb2,ac2,s2;

        int totalreq = 0;
        int L1req = 0;
        int L2req = 0;
        float L1hits = 0;
        float L1miss = 0;
        float L2hits = 0;
        float L2miss = 0;
        int servtime = 0;

        int L1latency;
        int L2latency;
        int Memlatency;
        float L1hitrate;
        float L2hitrate;
        float L1missrate;
        float L2missrate;

        int servtype;

    bool powerOf2(int x){
            return x&&(!(x&(x-1)));
    }

    void checkParam(int nb,int ac,int s,int l){
            if(!powerOf2(nb)||!powerOf2(ac)||!powerOf2(s)){
                cout<<"\nerror in L"<<l<<" cache parameters : ";
                cout<<"cache size / set associativity / block size is not in powers of 2"<<endl;
                exit(0);
            }
            if(s<4){
                cout<<"\nerror in L"<<l<<" cache parameters : ";
                cout<<"block size should be in words (word = 4 bytes) and hence can't be less than 4"<<endl;
                exit(0);
            }
            if((nb/(ac*s*4))<1){
                cout<<"\nerror in L"<<l<<" cache parameters : ";
                cout<<"size of cache inputted is less than sum of sizes of all blocks"<<endl;
                exit(0);
            }
    }

    void setCaches(int nb1,int ac1,int s1,int nb2,int ac2,int s2,int L1latency,int L2latency,int Memlatency){
        checkParam(nb1,ac1,s1,1);
        checkParam(nb2,ac2,s2,2);
        this->nb1 = nb1; this->nb2 = nb2;
        this->ac1 = ac1; this->ac2 = ac2;
        this->s1 = s1;   this->s2 = s2;
        if(s1>s2){
            cout<<"\nerror : L2 cache block size can't be less than L1 cache block size"<<endl;
            exit(0);
        }
        this->L1latency = L1latency;
        this->L2latency = L2latency;
        this->Memlatency = Memlatency;

        if(!((L2latency>L1latency)&&(Memlatency>L2latency))){
            cout<<"\nerror :  L1 cache latency < L2 cache latency < Memory access latency ,this relation doesnot hold for the inputted latencies "<<endl;
            exit(0);
        }

        inblock1 = new int[s1];
        outblock1 = new int[s1];
        inblock2 = new int[s2];
        outblock2 = new int[s2];
        L1cache.setCacheParam(nb1,ac1,s1);
        L2cache.setCacheParam(nb2,ac2,s2);
        
    }

    void evictToNext(Cache *L_up,Cache *L_lo,int *outblock,int s_up,int s_lo,int s){
        int lruAd;
        int lrufound;
        lruAd = L_up->lruBlkAd;
        lrufound = L_lo->find(lruAd,false);
        if(lrufound!=-1){
            L_lo->update(outblock,lruAd,lrufound,s_up,s_lo);
        }
        else{
            updateMem(outblock,lruAd,s);
        }
    }

    void evictToMem(Cache *L,int *outblock,int s){
        int lruAd;
        lruAd = L->lruBlkAd;
        updateMem(outblock,lruAd,s);
    }

    void updateMem(int *inblock,int ad,int s){
        for(int  i = 0;i<s;i++){
            dataseg[ad-(ad%s)+i] = inblock[i];
        }
    }

    int read(int ad){

        totalreq++;

        int found;

        found = L1cache.find(ad,true);

        int offset = L1cache.offset;
        int index = L1cache.index;
        int tag = L1cache.tag;

        if(found!=-1){

            L1hits++;
            servtime = L1latency;
            servtype = 1;

            return L1cache.ch[index].st[found].blk[offset];

        }
        else{

            L1miss++;

            found = L2cache.find(ad,true);

            offset = L2cache.offset;
            index = L2cache.index;
            tag = L2cache.tag;

            if(found!=-1){

                L2hits++;
                servtime = L2latency;
                servtype = 2;

                for(int i = 0;i<s1;i++){
                    inblock1[i] = L2cache.ch[index].st[found].blk[(ad%s2)-(ad%s1)+i];
                }

                dbi = false;

                L1cache.insert(ad,inblock1,outblock1,&dbi,&dbo);
                if(dbo){
                    evictToNext(&L1cache,&L2cache,outblock1,s1,s2,s1);
                }   

                return L2cache.ch[index].st[found].blk[offset];

            }
            else{

                L2miss++;
                servtime = Memlatency;
                servtype = 3;

                for(int i = 0;i<s2;i++){
                    if(i<s1){
                        inblock1[i] = dataseg[ad-(ad%s1)+i];
                    }
                    inblock2[i] = dataseg[ad-(ad%s2)+i];
                }

                dbi = false;

                L1cache.insert(ad,inblock1,outblock1,&dbi,&dbo);
                if(dbo){
                    evictToNext(&L1cache,&L2cache,outblock1,s1,s2,s1);
                }

                L2cache.insert(ad,inblock2,outblock2,&dbi,&dbo);    
                if(dbo){
                    evictToMem(&L2cache,outblock2,s2);
                }       

                return dataseg[ad]; 

            }           
        }
    }

    void write(int val,int ad){
        
        totalreq++;

        int found;

        found = L1cache.find(ad,true);

        int offset = L1cache.offset;
        int index = L1cache.index;
        int tag = L1cache.tag;

        if(found!=-1){

            L1hits++;
            servtime = L1latency;
            servtype = 1;

            L1cache.ch[index].st[found].blk[offset] = val;
            L1cache.ch[index].st[found].dbit = true;

        }
        else{

            L1miss++;

            found = L2cache.find(ad,true);

            offset = L2cache.offset;
            index = L2cache.index;
            tag = L2cache.tag;

            if(found!=-1){

                L2hits++;
                servtime = L2latency;
                servtype = 2;

                L2cache.ch[index].st[found].blk[offset] = val;
                L2cache.ch[index].st[found].dbit = true;

                for(int i = 0;i<s1;i++){
                    inblock1[i] = L2cache.ch[index].st[found].blk[(ad%s2)-(ad%s1)+i];
                }

                dbi = false;

                L1cache.insert(ad,inblock1,outblock1,&dbi,&dbo);
                if(dbo){
                    evictToNext(&L1cache,&L2cache,outblock1,s1,s2,s1);
                }

            }
            else{

                L2miss++;
                servtime = Memlatency;
                servtype = 3;

                dataseg[ad] = val;

                for(int i = 0;i<s2;i++){
                    if(i<s1){
                        inblock1[i] = dataseg[ad-(ad%s1)+i];
                    }
                    inblock2[i] = dataseg[ad-(ad%s2)+i];
                }

                dbi = false;

                L1cache.insert(ad,inblock1,outblock1,&dbi,&dbo);
                if(dbo){
                    evictToNext(&L1cache,&L2cache,outblock1,s1,s2,s1);
                }

                L2cache.insert(ad,inblock2,outblock2,&dbi,&dbo);    
                if(dbo){
                    evictToMem(&L2cache,outblock2,s2);
                }           

            }           
        }
    }

    void clearCacheLevel(Cache *L){

        int ad;

        for(int i = 0;i<(L->noOfSets);i++){
            for(int j = 0;j<(L->ac);j++){
                if((L->ch[i].st[j].isDirty())){
                    ad = ((L->ch[i].st[j].tag)<<((L->ibits)+(L->obits)))+(i<<(L->obits));
                    for(int k = 0;k<(L->s);k++){
                        dataseg[ad+k] = (L->ch[i]).st[j].blk[k];
                    }
                }
            }
        }

    }

    void clearCache(){

        clearCacheLevel(&L2cache);
        clearCacheLevel(&L1cache);
        L1req = L1hits+L1miss;
        L1hitrate = L1hits/L1req;
        L2req = L2hits+L2miss;
        L2hitrate = L2hits/L2req;

    }
    
};

class processor{

    public :
        int reg[32] = {0};
        int *pc,*idpc,*expc,*mempc,*wbpc;
        int fnc,select = -1,iselect = -1,end = 0,r;
        int ifreg,idi,exi,memi,wbi;
        int regs1,regs2,imm,result,memreg,wbreg;
        int *prs1,*prs2;
        int *regd = NULL,*regsw = NULL,*memsw = NULL,*exsw = NULL ;
        int *exdr = NULL,*memdr = NULL,*wbdr = NULL;
        int eslct,mslct,wslct;
        int eislct,mislct,wislct;
        int mend = 0,wend = 0;
        bool ld_lock = false,br_lock = false;
        bool ild_lock = false,eld_lock = false,mld_lock = false,wld_lock = false;
        bool ld_depends = false;
        bool id_rf = false,ex = false,mem = false,wbr = false;
        bool istall = false,estall = false,mstall = false,wstall = false;
        bool ibdepends = false,ebdepends = false,mbdepends = false,wbdepends = false;
        bool bdepends = false,mem_in_ld = false;

        CacheCtrl cc;
        int servtime_m = 1,servtime_w = 1;
        bool mem_stall = false;
        bool wmem_stall = false;
        int stall_type_i = 0,stall_type_e = 0,stall_type_m = 0,stall_type_w = 0;

    void getCacheParam(fstream &file){
        int a[9],i = 0;
        string buff;
        
        while(file>>buff){
            try{ a[i] = stoi(buff);i++; }
            catch(exception e){}
        }
        
        file.close();
        
        if(i==9){
            cc.setCaches(a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8]);
        }
        else{
            cout<<"\nerror : Invalid cache parameters, check the file contents"<<endl;
            exit(0);
        }
    }
        
    void setpc(int main_i){
        pc = text+main_i;
    }   
    
    void fetch(){
        wbdr = NULL;
        if(br_lock){
            clk++;
            br_lock = false;
        }
        ifreg = text[pc-text];
        pc++;
        id_rf = true;
    }

    void idrf(){
        idi = ifreg;
        idpc = pc-1;
        ex = true;
        
        int opc = ((unsigned int)ifreg)>>26;
        
        regs1 = reg[(0x03e00000&ifreg)>>21];
        prs1 = reg+((0x03e00000&ifreg)>>21);
        regs2 = reg[(0x001f0000&ifreg)>>16];
        prs2 = reg+((0x001f0000&ifreg)>>16);
        
        if((exdr!=NULL)&&(prs1!=reg)&&(prs1==exdr)){
            regs1 = result;
            if(ld_lock){
                ld_depends = true;
            }
            bdepends = true;
        }
        else if((memdr!=NULL)&&(prs1!=reg)&&(prs1==memdr)){
            regs1 = memreg;
            if(mem_in_ld){
                bdepends = true;
            }
        }
        if(opc==0){
            fnc = (0x3f)&ifreg;
            select = getIndex(func,fnc,6);
            if(select==4||select==5){
                regs2 = (ifreg&(0x000007c0))>>6;
                regd = reg+((0x001f0000&ifreg)>>16);
            }
            else{
                if((exdr!=NULL)&&(prs2!=reg)&&(prs2==exdr)){
                    regs2 = result;
                    if(ld_lock){
                        ld_depends = true;
                    }
                    bdepends = true;
                }
                else if((memdr!=NULL)&&(prs2!=reg)&&(prs2==memdr)){
                    regs2 = memreg;
                    if(mem_in_ld){
                        bdepends = true;
                    }
                }
                regd = reg+(((0x0000f800)&ifreg)>>11);
            }
        }
        else if(opc==3){
             select = -1;
             pc = text+((0x03ffffff)&(ifreg));
             br_lock = true;
             istall = true;
             stall_type_i = 5;
        }
        else{
            iselect = getIndex(iop,opc,6);
            select = 0;
            if(iselect==2||iselect==3){
                if((exdr!=NULL)&&(prs2!=reg)&&(prs2==exdr)){
                    regs2 = result;
                    if(ld_lock){
                        ld_depends = true;
                    }
                    bdepends = true;
                }
                else if((memdr!=NULL)&&(prs2!=reg)&&(prs2==memdr)){
                    regs2 = memreg;
                    if(mem_in_ld){
                        bdepends = true;
                    }
                }
            }
            
            imm = (ifreg)&(0x0000ffff);
            if(((ifreg&(0x00008000))>>15)==1){
                     imm = imm + 0xffff0000;
            }
            
            if(iselect==5){ select = 2;}
            
            if(iselect==2){
                if(!(ld_lock&&ld_depends)){
                    if(regs1==regs2){
                        regs2 = imm;
                        pc = pc+regs2;
                        regd = NULL;
                    }
                    br_lock = true;
                    istall = true;
                    if(bdepends){
                        ibdepends = true;
                        stall_type_i = 4;
                    }
                    else{
                        stall_type_i = 5;
                    }
                    bdepends = false;
                }
            }
            else if(iselect==3){
                if(!(ld_lock&&ld_depends)){
                    if(regs1!=regs2){
                        regs2 = imm;
                        pc = pc+regs2;
                        regd = NULL;
                    }
                    br_lock = true;
                    istall = true;
                    if(bdepends){
                        ibdepends = true;
                        stall_type_i = 4;
                    }
                    else{
                        stall_type_i = 5;
                    }
                    bdepends = false;
                }
            }
            else{
                regs2 = imm;
                if(iselect==0||iselect==1){
                    if(!(ld_lock&&ld_depends)){
                        r = (regs1 + regs2)%4;
                        if(r!=0){
                            cout<<"error : unaligned address access in line no. "<<pc-text-1<<" "<<instarray[pc-text-1]<<endl;
                            exit(0);
                        }
                    }
                }
                if(iselect!=1){
                    regd = reg+(((0x001f0000)&ifreg)>>16);
                    regsw = NULL;
                }
                else{
                    regd = NULL;
                    regsw = reg+(((0x001f0000)&ifreg)>>16);
                }
            }
        }

        if(ld_lock&&ld_depends){
                ld_lock = false;
                ld_depends = false;
                ild_lock = true;
                istall = true;
                stall_type_i = 4;
                pc--;
        }
        else if(ld_lock){
                ld_lock = false;            
        }
        if(iselect==0){
            ld_lock = true;
        }
        
        bdepends = false;
    }
    
    void execute(){

        eld_lock = ild_lock;
        estall = istall;
        stall_type_e = stall_type_i; 
        stall_type_i = 0;
        ebdepends = ibdepends;
        mem = true;
        ibdepends = false;
        istall = false;

        if(!eld_lock){

            eslct = select;
            eislct = iselect;
            exdr = regd;
            regd = NULL;
            exi = idi;
            exsw = regsw;
            iselect = -1;
            select = -1;
            expc = idpc;
            switch(eslct){
                case 0:if(eislct!=2&&eislct!=3&&end==0){ result = regs1+regs2; }; break;
                case 1:result = regs1-regs2; break;
                case 2:result = ((regs1<regs2)?(1):(0));break;
                case 3:end = 1; break;
                case 4:result = regs1<<regs2; break;
                case 5:result = ((unsigned int)regs1)>>regs2; break;
                default : break;
            }
        }
        else{
            ild_lock = false;
        }
    }
    
    void memrw(){
        mld_lock = eld_lock;
        mstall = estall;
        mbdepends = ebdepends;
        stall_type_m = stall_type_e;
        stall_type_e = 0;
        wbr = true;
        ebdepends = false;
        estall = false;

        if(!mld_lock){

            mslct = eslct;
            mislct = eislct;
            memdr = exdr;
            memi = exi;
            memsw = exsw;
            mempc = expc;
            mend = end;
            exdr = NULL;

            if(mislct==0){
                mem_in_ld = true;
                if((result/4)<1024){
                    memreg = cc.read(result/4);
                    servtime_m = cc.servtime;
                    if(servtime_m!=1){
                        mem_stall = true;
                    }
                    stall_type_m = cc.servtype;
                }
                else{
                    cout<<"error : address out of bounds in line no. "<<pc-text-1<<" "<<instarray[pc-text-1]<<endl;
                    exit(0);
                }
            }
            else if(mislct==1){
                if((result/4)<1024){
                    cc.write(*memsw,result/4);
                    memreg = *memsw;
                    servtime_m = cc.servtime;
                    if(servtime_m!=1){
                        mem_stall = true;
                    }
                    stall_type_m = cc.servtype;
                }
                else{
                    cout<<"error : address out of bounds in line no. "<<pc-text-1<<" "<<instarray[pc-text-1]<<endl;
                    exit(0);
                }
            }
            else {
                if(end==0){
                    memreg = result;
                    servtime_m = 1;
                    mem_stall = false;
                }
            }
        }
        else{
            eld_lock = false;
        }
    }
    
    bool wb(){

        wld_lock = mld_lock;
        wstall = mstall;
        mstall = false;
        wbdepends = mbdepends;
        mbdepends = false;
        mem_in_ld = false;
        wmem_stall = mem_stall;
        mem_stall = false;
        servtime_w = servtime_m;
        stall_type_w = stall_type_m;
        stall_type_m = 0;

        if(!wld_lock){

            wslct = mslct;
            wislct = mislct;
            wbdr = memdr;
            wbi = memi;
            wbpc = mempc;
            wend = mend;
            memdr = NULL;
            mld_lock = false;

            if(((wislct<1)||(wislct>3))&&wslct!=-1&&wend==0){
             *wbdr = memreg;
            }
            if(wend==1){
                 cc.clearCache();
                 return true;
            }
            return false;
        }
        else{
            return false;
        }
    }
    
    void print(){
        cout<<"-----------------------------------------------------------REGISTERS-----------------------------------------------------------"<<endl;
        cout<<endl<<"pc = "<<wbpc<<endl<<endl;
        for(int u = 0;u<32;u++){
          cout<<regno[u]<<" = "<<reg[u]<<endl;
        }
        cout<<"-------------------------------------------------------------------------------------------------------------------------------"<<endl;
    }
};

void print_stall(int stall_type,int netstalls,int no_of_stalls){
    cout<<"-------------------------------------------------------------------------------------------------------------------------------"<<endl;
    cout<<"                        PIPELINE STALLS BY "<<no_of_stalls<<" CYCLE(S), [REASON : ";
    switch(stall_type){
        case 1: cout<<"L1 latency]";break;
        case 2: cout<<"L2 latency]";break;
        case 3: cout<<"Memory latency]";break;
        case 4: cout<<"data hazard]";break;
        case 5: cout<<"control hazard]";break;
        default: break;
    }
    cout<<", TOTAL NO. OF STALLS : "<<netstalls<<endl;
    cout<<"-------------------------------------------------------------------------------------------------------------------------------\n"<<endl;
}

int main(){
    int o = 1;
    bool b = false;
    
    parser p;
    converter c;
    processor pr;
    
    fstream file,cachefile;
    string file_name;
    cout<<"Enter the cache parameters file name: ";
    cin>>file_name;
    cachefile.open(file_name.c_str(),ios::in);
    
    if(cachefile){
       pr.getCacheParam(cachefile); 
    }
    else{ 
       cout<<"\nerror : cache parameters file not accessible"<<endl;
       exit(0);
    }
    
    cout<<"Enter the assembly file name: ";
    cin>>file_name;
    file.open(file_name.c_str(),ios::in);

    cachefile.close();

    if(file){
        
        cout<<"-----------------------------------------------------------TEXT----------------------------------------------------------------"<<endl;
    
        p.tknizer(file,c);
        
        cout<<"-------------------------------------------------------------------------------------------------------------------------------"<<endl;
        
        cout<<"----------------------------------------------------------LABELS---------------------------------------------------------------"<<endl;
        
        for(int i = 0;i<label.size();i++){
            cout<<"\'"<<label[i]<<"\'"<<" in line "<<label_i[i]<<endl;
        }
        cout<<"-------------------------------------------------------------------------------------------------------------------------------"<<endl;
        
        pr.setpc(getTarget("main",0,1));
        
        printdata();
        pr.print();
        cout<<endl;
        
        while(true){
            if(o==1&&(pr.wbr)&&(!(pr.mld_lock))){
                cout<<"\n1. single step"<<endl;
                cout<<"2. run/continue"<<endl;
                cout<<"3. exit\n"<<endl;
                cout<<"ENTER OPTION : ";
                cin>>o;
                cout<<endl;
            }
            if(o==1||o==2){
                clk++;
                if(pr.wbr){
                    if(pr.wstall){
                        stall++;
                        print_stall(pr.stall_type_w,stall,1);
                        pr.servtime_w = 1;
                        pr.stall_type_w = 0;
                        pr.wstall = false;
                    }
                    b = pr.wb();
                    if(pr.wbdepends){
                        stall++;
                        clk++;
                        print_stall(pr.stall_type_w,stall,1);
                        pr.servtime_w = 1;
                        pr.stall_type_w = 5;
                        pr.wbdepends = false;
                    }
                    if((!pr.wld_lock)){
                        i_cnt++;
                        cout<<i_cnt<<". [line no. : "<<(pr.wbpc)-text<<"] "<<instarray[(pr.wbpc)-text]<<" ";
                        printf("[instruction in hexadecimal : 0x%x] executing \n\n",pr.wbi);
                        if(pr.wmem_stall){
                            stall = stall+pr.servtime_w-1;
                            clk = clk+pr.servtime_w-1;
                            print_stall(pr.stall_type_w,stall,pr.servtime_w-1);
                            pr.wmem_stall = false;
                            pr.servtime_w = 1;
                            pr.stall_type_w = 0;
                        }
                        if(o==1){
                            printdata();
                            pr.print();
                            cout<<endl;
                        }
                    }
                    else{
                        pr.wld_lock = false;
                    }
                }
                if(b){
                    break;
                }
                if(pr.mem){pr.memrw();}
                if(pr.ex){pr.execute();}
                if(pr.id_rf){pr.idrf();}
                pr.fetch();
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
            cout<<"\n-------------------------------------------------------------------------------------------------------------------------------"<<endl;
            printdata();
            pr.print();
            cout<<endl;
        }
        cout<<"MIPS pipelined processor with data forwarding and two levels of non inclusive cache (Both levels use LRU eviction policy)"<<endl;
        
        cout<<endl;
        
        cout<<"Number of accesses made to L1 cache : "<<pr.cc.L1req<<endl;
        cout<<"Number of L1 cache hits : "<<pr.cc.L1hits<<endl;
        if(pr.cc.L1req!=0){
            printf("L1 cache hit rate : %.2f\n",pr.cc.L1hitrate);
            printf("L1 cache miss rate : %.2f\n",1-pr.cc.L1hitrate);
        }
        else{
            cout<<"L1 cache hit rate : N/A"<<endl;
            cout<<"L1 cache miss rate : N/A"<<endl;
            cout<<"No accesses made"<<endl;
        }
        
        cout<<endl;
        
        cout<<"Number of accesses made to L2 cache : "<<pr.cc.L2req<<endl;
        cout<<"Number of L2 cache hits : "<<pr.cc.L2hits<<endl;
        if(pr.cc.L2req!=0){
            printf("L2 cache hit rate : %.2f\n",pr.cc.L2hitrate);
            printf("L2 cache miss rate : %.2f\n",1-pr.cc.L2hitrate);
        }
        else{
            cout<<"L2 cache hit rate : N/A"<<endl;
            cout<<"L2 cache miss rate : N/A"<<endl;
            cout<<"No accesses made"<<endl;
        }
        
        cout<<endl;
        
        cout<<"Number of clock cycles passed : "<<clk<<endl;
        cout<<"Number of stalls (1 stall = 1 cycle) : "<<stall<<endl;
        cout<<"Number of instructions executed : "<<i_cnt<<endl;
        printf("Average CPI (CPI = (Number of clock cycles passed / Number of instruction executed)) : %.2f\n",(clk/i_cnt));
        printf("IPC (IPC = 1/CPI) : %.2f\n",(i_cnt/clk));
        
    }
    else{
        cout<<"\nerror : assembly file not accessible"<<endl;
    }
    file.close();
}