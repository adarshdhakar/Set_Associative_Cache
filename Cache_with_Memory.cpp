#include <bits/stdc++.h>
using namespace std;

#define MEM_BLOCKS 16384 // 2^14
#define WAYS 8
#define SETS 64
#define WORDS_PER_BLOCK 16

#define INVALID 0
#define VALID 1
#define MISPENDING 2

#define READ 0
#define WRITE 1
#define MISS 0
#define HIT 1

vector<vector<int>> main_memory(MEM_BLOCKS, vector<int>(WORDS_PER_BLOCK));

void initializeMainMemoryBlocks() {
    for (int i = 0; i < MEM_BLOCKS; i++) {
        for (int j = 0; j < WORDS_PER_BLOCK; j++) {
            main_memory[i][j] = rand();
        }
    }
}

string to_str(uint32_t num)
{
    bitset<32> binary(num);
    return binary.to_string();
}

uint32_t to_int(string binaryStr)
{
    uint32_t decimalValue = 0;
    int length = binaryStr.length();

    for (int i = 0; i < length; i++)
    {
        char bit = binaryStr[length - 1 - i];
        if (bit == '1')
        {
            decimalValue += (1 << i);
        }
    }

    return decimalValue;
}

class Block {
    public:
        int state;
        int tag;
        vector<int> dataBlock;
        Block() {
            this->state = INVALID;
            this->tag = -1;
            this->dataBlock = vector<int>();
        }
};

class CPUReq {
    public:
        uint32_t Index;
        uint32_t Tag;
        uint32_t bitOffset;
        uint32_t address;
        int data;
        int read_write;

                //-----------------------Random Value Generator from Normal Distribution-----------------------//
        random_device rd;
        mt19937 gen;
        normal_distribution<double> dist;

        CPUReq() : gen(rd()), dist(10000, 3000) {} 

        uint64_t generate_normal_random_address(long long mean, long long stddev) {
            dist = normal_distribution<double>(mean, stddev);
            uint64_t result;

            do {
                result = (dist(gen));
            } while (result < 0);

            return result * 4;
        }
        //---------------------------------------------------------------------------------------------//

        void generateRequest(){
            this->address = generate_normal_random_address(100, 110);

            string binaryAddress = to_str(address).substr(12, 20);
            string idx = binaryAddress.substr(0, 6);
            string tag = binaryAddress.substr(6, 8);
            string off = binaryAddress.substr(14, 6);

            this->Index = to_int(idx);
            this->Tag = to_int(tag);
            this->bitOffset = to_int(off);
            this->data = rand();
            this->read_write = rand()%2;
            this->read_write = READ;

            // cout << this->Index << " " << this->Tag << " " << this->bitOffset << endl;
            // cout << "Address: " << address << "(" << binaryAddress << ")" << endl;
            // cout << "Extracted bits: " 
            //     << idx << " (" << this->Index << ") "
            //     << tag << " (" << this->Tag << ") "
            //     << off << " (" << this->bitOffset << ") "
            //     << endl;
            
            if(read_write == READ){
                cout << "Read Request" << endl;
            }
            else {
                cout << "Request data -> " << this->data << endl;
                cout << "Write Request" << endl;
            }
        }
        
        void wait(){
            //current request waits until the previous request is fulfilled
        }
};

class CPUResp {
    public:
        int data;
        int hit_miss;
        Block block;

        CPUResp(){
            this->hit_miss = MISS;
        }
};

class MemReq {
    public:
        uint32_t Index;
        uint32_t Tag;
        uint32_t bitOffset;
        uint32_t address;
        int data;
        int read_write;
};

class MemResp {
    public:
        uint32_t Tag;
        vector<int> dataBlock;
        
        void fulfillMemReq(MemReq &memReq){
            if(memReq.read_write == READ){
                this->fulfillReadRequest(memReq);
            }
            else {
                this->fullfillWriteRequest(memReq);
            }
        }

        void fulfillReadRequest(MemReq &memReq){
            int memBlockNo = memReq.Index*memReq.Tag;
            this->Tag = memReq.Tag;
            this->dataBlock = main_memory[memBlockNo];
        }

        void fullfillWriteRequest(MemReq &memReq){
            this->Tag = memReq.Tag;
            int memBlockNo = memReq.Index*memReq.Tag;
            int offset = memReq.bitOffset/4;

            main_memory[memBlockNo][offset] = memReq.data;
        }
};

class Cache {
    public:
        vector<vector<Block>> cacheArray{SETS, vector<Block>(WAYS)};

        void processRequest(CPUReq &req, CPUResp &resp) {
            if(req.read_write == READ){
                this->readReq(req, resp);
            }
            else {
                this->writeReq(req, resp);
            }
        }
        void readReq(CPUReq &req, CPUResp &resp){
            int index = req.Index;
            auto indexedSet = this->cacheArray[index];

            int x = 0;
            for(auto &B : indexedSet){
                if(B.tag == req.Tag && B.state == VALID){
                    resp.hit_miss = HIT;
                    resp.block = B;
                    int offset = req.bitOffset/4;
                    resp.data = B.dataBlock[offset];
                    return;
                }
                if(B.tag == req.Tag && B.state == MISPENDING){
                    req.wait();
                    return;
                }
            }

            int evictedBlock = rand()%WAYS;
            Block B;
            B.state = MISPENDING;
            B.tag = req.Tag;

            MemReq memReq;
            this->generateMemoryRequest(memReq, req);

            MemResp memResp;
            memResp.fulfillMemReq(memReq);

            B.dataBlock = memResp.dataBlock;
            B.state = VALID;
            this->cacheArray[index][evictedBlock] = B;

            resp.hit_miss = MISS;
            resp.block = B;
            int offset = req.bitOffset/4;
            resp.data = B.dataBlock[offset];
        }

        void writeReq(CPUReq &req, CPUResp &resp){
            int index = req.Index;
            auto indexedSet = this->cacheArray[index];

            for(auto &B : indexedSet){
                if(B.tag == req.Tag && B.state == VALID){
                    resp.hit_miss = HIT;
                    resp.block = B;
                    int offset = req.bitOffset/4;
                    B.dataBlock[offset] = req.data;

                    //write through
                    this->writeThroughIntoMemory(req);

                    return;
                }
                if(B.tag == req.Tag && B.state == MISPENDING){
                    req.wait();
                    return;
                }
            }

            MemReq memReq;
            this->generateMemoryRequest(memReq, req);

            //write no allocate
            MemResp memResp;
            memResp.fulfillMemReq(memReq);

            resp.hit_miss = MISS;
        }

        void generateMemoryRequest(MemReq &memReq, CPUReq &req){
            memReq.address = req.address;
            memReq.Index = req.Index;
            memReq.Tag = req.Tag;
            memReq.data = req.data;
            memReq.bitOffset = req.bitOffset;
            memReq.read_write = req.read_write;
        }

        void writeThroughIntoMemory(CPUReq &req){
            int offset = req.bitOffset/4;
            int memBlockNo = req.Index*req.Tag;
            main_memory[memBlockNo][offset] = req.data;
        }
};

void printMem(){
    for (int i = 0; i < MEM_BLOCKS; i++) {
        for (int j = 0; j < WORDS_PER_BLOCK; j++) {
            cout << main_memory[i][j] << " ";
        }
        cout << endl;
    }
}

void printCache(Cache &cache){
    cout << "State" << endl;

    for(int i = 0; i < SETS; i++){
        for(int j = 0; j < WAYS; j++){
            cout << cache.cacheArray[i][j].state << " ";
        }
        cout << endl;
    }

    for(int i = 0; i < SETS; i++){
        for(int j = 0; j < WAYS; j++){
            cout << cache.cacheArray[i][j].tag << " ";
        }
        cout << endl;
    }

    for(int i = 0; i < SETS; i++){
        for(int j = 0; j < WAYS; j++){
            if(!cache.cacheArray[i][j].dataBlock.empty()){
                for(int k = 0; k < WORDS_PER_BLOCK; k++){
                    cout << cache.cacheArray[i][j].dataBlock[k] << " ";
                }
            }
            else {
                cout << "Empty Block" << " ";
            }
            cout << "    ";
        }
        cout << endl;
    }
}

int main(){
    cout << "20-addressable memory\n" << endl;
    Cache cache;

    initializeMainMemoryBlocks(); 
    int hits = 0;
    int REQUESTS = 10000;

    CPUReq req;
    for(int i = 1; i <= REQUESTS; i ++){
        cout << "Request " << i << " generated" << endl;
        req.generateRequest();
        CPUResp resp;
        cache.processRequest(req, resp);
        
        hits += resp.hit_miss;
        
        int misses = i-hits;
        double hitPercent = (hits/(double)REQUESTS) * 100;
        double missPercent = (misses/(double)REQUESTS) * 100;

        cout << "Hits: " << hits << endl;
        cout << "Misses: " << misses << endl;
        cout << "Request " << i << " processed" << endl;
        cout << endl;
    }

    int misses = REQUESTS-hits;
    double hitPercent = (hits/(double)REQUESTS) * 100;
    double missPercent = (misses/(double)REQUESTS) * 100;

    cout << "Hits: " << hits << "Hit percentage: " << hitPercent<< endl;
    cout << "Misses: " << misses << "Miss percentage: " << missPercent << endl;

    // printMem();
    // printCache(cache);
    return 0;
}
