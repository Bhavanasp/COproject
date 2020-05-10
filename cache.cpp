#include <iostream>
#include <cmath>
using namespace std;

//Main memory is filled with  all zeros initially

int dataseg[256] = {0};

void printData(){
    cout<<"-----------------------------------------------------------MAIN MEMORY----------------------------------------------------------";
	for(int i = 0;i<256;i++){
		if(!(i%64)){
			cout<<endl;
		}
		cout<<dataseg[i]<<" ";
	}
	cout<<endl;
	cout<<"--------------------------------------------------------------------------------------------------------------------------------"<<endl;
}

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

		void print(){
	    	for(int i = 0;i<noOfSets;i++){
	    	        	cout<<"***********************SET "<<i<<"***********************\n"<<endl;
	    		for(int j = 0;j<ac;j++){
	    			cout<<"dbit "<<ch[i].st[j].dbit<<endl;
	    			cout<<"vbit "<<ch[i].st[j].vbit<<endl;
	    			if(ch[i].st[j].isValid()){
	    			    cout<<"age "<<ch[i].st[j].age<<endl;
	    				cout<<"tag "<<ch[i].st[j].tag<<endl;
	    				cout<<"---------------------------------------------------"<<endl;
	    				for(int k = 0;k<s;k++){
	    					cout<<ch[i].st[j].blk[k]<<" ";
	    				}
	    				cout<<"\n---------------------------------------------------"<<endl;
	    			}
	    			else{
	    			    cout<<"age N/A"<<endl;
	    				cout<<"tag N/A"<<endl;
	    				cout<<"---------------------------------------------------"<<endl;
	    				for(int k = 0;k<s;k++){
	    					cout<<"N/A ";
	    				}
	    				cout<<"\n---------------------------------------------------"<<endl;
	    			}
	    			cout<<endl;
	    		}
	    		cout<<endl;
	    	}
	    }

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

	void checkParam(int nb,int ac,int s){
			if(!powerOf2(nb)||!powerOf2(ac)||!powerOf2(s)){
				cout<<"\nerror : cache size / set associativity / block size is not in powers of 2"<<endl;
				exit(0);
			}
			if(s<4){
				cout<<"\nerror : block size should be in words (word = 4 bytes) and hence can't be less than 4"<<endl;
				exit(0);
			}
			if((nb/(ac*s*4))<1){
				cout<<"\nerror : size of cache inputted is less than sum of sizes of all blocks"<<endl;
				exit(0);
			}
	}

	void setCaches(int nb1,int ac1,int s1,int nb2,int ac2,int s2,int L1latency,int L2latency,int Memlatency){
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

			cout<<"------------------------------------------------------------L1 hit--------------------------------------------------------------"<<endl;

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

				cout<<"------------------------------------------------------------L2 hit--------------------------------------------------------------"<<endl;

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

				cout<<"----------------------------------------------------------cache miss------------------------------------------------------------"<<endl;
                
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

			cout<<"------------------------------------------------------------L1 hit--------------------------------------------------------------"<<endl;

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

				cout<<"------------------------------------------------------------L2 hit--------------------------------------------------------------"<<endl;

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

				cout<<"----------------------------------------------------------cache miss------------------------------------------------------------"<<endl;
                
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

	}
	
};

void print_cache(CacheCtrl cc){
    cout<<"\n--------------------------------------------------------------------------------------------------------------------------------"<<endl;
    cout<<"                                                              L1 CACHE"<<endl;
    cout<<"--------------------------------------------------------------------------------------------------------------------------------\n"<<endl;
    cc.L1cache.print();
    cout<<"--------------------------------------------------------------------------------------------------------------------------------"<<endl;
    cout<<"                                                              L2 CACHE"<<endl;
    cout<<"--------------------------------------------------------------------------------------------------------------------------------\n"<<endl;
    cc.L2cache.print();
    cout<<"totalreq : "<<cc.totalreq<<endl;
	cout<<"L1 hits : "<<cc.L1hits<<endl;
	cout<<"L1 miss : "<<cc.L1miss<<endl;
	cc.L1hitrate = (cc.L1hits)/(cc.L1hits+cc.L1miss);
    cc.L1missrate = 1-cc.L1hitrate;
    printf("L1 hit rate : %.2f\n",cc.L1hitrate);
    printf("L1 miss rate : %.2f\n",cc.L1missrate);
	cout<<"L2 hits : "<<cc.L2hits<<endl;
	cout<<"L2 miss : "<<cc.L2miss<<endl;
    cc.L2hitrate = (cc.L2hits)/(cc.L2hits+cc.L2miss);
    cc.L2missrate = 1-cc.L2hitrate;
    printf("L2 hit rate : %.2f\n",cc.L2hitrate);
    printf("L2 miss rate : %.2f\n",cc.L2missrate);
    cout<<"servtime (in cycles) : "<<cc.servtime<<endl;
    cout<<endl;
	cout<<"--------------------------------------------------------------------------------------------------------------------------------"<<endl;
	cout<<endl;
	printData();
}

int main(){
	int val,ad;
	int op;
	bool b = true;
	bool set = false;
	CacheCtrl cc;
	int nb1,ac1,s1;
	int nb2,ac2,s2;
	int L1lat,L2lat,memlat;
	while(b){
		cout<<"\nCHOOSE ONE :"<<endl;
		cout<<"0. SET CACHE PARAMETERS\n1. READ\n2. WRITE\n3. EXIT\n"<<endl;
		cout<<"option : ";
		cin>>op;
		cout<<endl;
		switch(op){
			case 0: cout<<"L1 Cache Size(Bytes): ";cin>>nb1;
					cout<<"L1 Associativity (Number of blocks per set) : ";cin>>ac1;
					cout<<"L1 Block Size (number of words per block): ";cin>>s1;
					cc.checkParam(nb1,ac1,s1);
					cout<<"L2 Cache Size(Bytes): ";cin>>nb2;
					cout<<"L2 Associativity (Number of blocks per set) : ";cin>>ac2;
					cout<<"L2 Block Size (number of words per block): ";cin>>s2;
					cc.checkParam(nb2,ac2,s2);
					cout<<"L1 cache latency (in cycles) : ";cin>>L1lat;
					cout<<"L2 cache latency (in cycles) : ";cin>>L2lat;
					cout<<"memory latency (in cycles) : ";cin>>memlat;
					cc.setCaches(nb1,ac1,s1,nb2,ac2,s2,L1lat,L2lat,memlat);
					set = true;
					cout<<endl;
					printData();
					break;
            case 1: if(!set){cout<<"Please set the cache parameters"<<endl; break; }
                    cout<<"What is the address of the word to be read? : ";cin>>ad;
                    cout<<"\nread "<<ad<<endl;
                    cc.read(ad);
                    print_cache(cc);
            		break;
            case 2: if(!set){cout<<"Please set the cache parameters"<<endl; break; }
                    cout<<"What Should be stored ? :";cin>>val;
                    cout<<"At what address ? :";cin>>ad;
                    cout<<"\nwrite "<<val<<" at "<<ad<<endl;
                    cc.write(val,ad);
                    print_cache(cc);
            		break;
            case 3: cc.clearCache();printData();b = false;break;
			default: cout<<"no such option, try again"<<endl;break;
		}		
	}
}