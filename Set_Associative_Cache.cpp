#include <bits/stdc++.h>
using namespace std;

//******************************Set Associative Cache******************************//
#define WAYS 8
#define SETS 64
#define WORDS_PER_BLOCK 16

#define INVALID 0
#define VALID 1

#define READ 0
#define WRITE 1
#define MISS 0
#define HIT 1
//*********************************************************************************//

//**************************String to Int & Int to String**************************//
string to_str(uint64_t num)
{
    bitset<40> binary(num);
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
//*********************************************************************************//

//*********************************Class for Block*********************************//
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
//*********************************************************************************//

//***************************CPU Request & CPU Response****************************//
class CPUReq {
    public:
        uint32_t Index;
        uint32_t Tag;
        uint32_t bitOffset;
        uint64_t address;
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
            string binaryAddress = to_str(address);
            string idx = binaryAddress.substr(0, 6);
            string tag = binaryAddress.substr(6, 28);
            string off = binaryAddress.substr(34, 6);

            this->Index = to_int(idx);
            this->Tag = to_int(tag);
            this->bitOffset = to_int(off);
            this->data = rand();
            this->read_write = rand()%2;
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
//*********************************************************************************//

//********************************MemReq & MemResp*********************************//
class MemReq {
    public:
        uint32_t Index;
        uint32_t Tag;
        uint32_t bitOffset;
        uint64_t address;
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
            int memBlockNo = memReq.Index*SETS + memReq.Tag;
            this->Tag = memReq.Tag;

            vector<int> block;
            for(int i = 0; i < WORDS_PER_BLOCK; i++){
                block.push_back(rand());
            }

            // cout << "Read and brought back into the cache from the memory after miss" << endl;
            this->dataBlock = block;
        }

        void fullfillWriteRequest(MemReq &memReq){
            this->Tag = memReq.Tag;
            int memBlockNo = memReq.Index*SETS + memReq.Tag;
            int offset = memReq.bitOffset/4;

            // cout << "Written (Write - No Allocate) to the memory when miss" << endl;
        }
};
//*********************************************************************************//

//**********************************CACHE CLASS************************************//
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

        //***************************Read Request****************************//
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
            }

            int evictedBlock = rand()%WAYS;
            Block B;
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
        //*********************************************************************//

         //***************************Write Request****************************//
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
            }

            MemReq memReq;
            this->generateMemoryRequest(memReq, req);

            //write no allocate
            MemResp memResp;
            memResp.fulfillMemReq(memReq);

            resp.hit_miss = MISS;
        }
         //********************************************************************//

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
            int memBlockNo = req.Index*SETS + req.Tag;

            // cout << "Written through into memory when Hit at Memory Block No. : " << memBlockNo << " with offset : " << offset<< endl;
        }
};
//*********************************************************************************//

//**********************************MAIN FUNCTION**********************************//
int main(){
    cout << "\n40-addressable memory\n";
    
    srand(time(NULL));
    Cache cache;

    int hits = 0;
    int REQUESTS = 10000000;
    cout << REQUESTS << " Requests\n";

    CPUReq req;

    for(int i = 1; i <= REQUESTS; i ++){
        req.generateRequest();
        cout << "\nRequest " << i << " generated";
        CPUResp resp;
        cache.processRequest(req, resp);
        hits += resp.hit_miss;
        
        int misses = i-hits;

        // if(REQUESTS < 10){
        //     cout << "......................................";
        // }
        // else if(REQUESTS < 1e3)
        // {
        //     if(i % (REQUESTS/10) == 0)
        //     {
        //         cout << "#";
        //     }
        // }
        // else if(REQUESTS < 1e5)
        // {
        //     if(i % (REQUESTS/1000) == 0)
        //     {
        //         cout << "#";
        //     }
        // }
        // else if(REQUESTS < 1e7)
        // {
        //     if(i % (REQUESTS/100000) == 0)
        //     {
        //         cout << "#";
        //     }
        // }
        // else 
        // {
        //     if(i % (REQUESTS/10000000) == 0)
        //     {
        //         cout << "#";
        //     }
        // }

        if(i % (REQUESTS/10) == 0)
        {   
            cout << "\nProgress at " << ((i*100) / REQUESTS) << endl;
            int misses = i-hits;
            double hitPercent = (hits/(double)i) * 100;
            double missPercent = (misses/(double)i) * 100;
            cout << "Hits:   " << hits << "  Hit percentage: " << hitPercent << endl;
            cout << "Misses: " << misses << "  Miss percentage: " << missPercent << endl;
        }

        cout << "Request " << i << " processed\n";
    }

    int misses = REQUESTS-hits;
    double hitPercent = (hits/(double)REQUESTS) * 100;
    double missPercent = (misses/(double)REQUESTS) * 100;
    
    cout << endl;
    cout << "Hits:   " << hits << "  Hit percentage: " << hitPercent<< endl;
    cout << "Misses: " << misses << "  Miss percentage: " << missPercent << endl;
}
//*********************************************************************************//

