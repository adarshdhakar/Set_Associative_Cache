#include <bits/stdc++.h>
using namespace std;

#define MEM_BLOCKS pow(2, 19)
#define WAYS 8
#define SETS 64
#define WORDS_PER_BLOCK 16

#define INVALID 0
#define VALID 1
#define MISPENDING 2

vector<vector<int>> main_memory(MEM_BLOCKS, vector<int>(WORDS_PER_BLOCK, 0));

void initializeMainMemoryBlocks(){
    for(int i = 0; i < MEM_BLOCKS; i++){
        for(int j = 0; j < WORDS_PER_BLOCK; j++){
            main_memory[i][j] = rand();
        }
    }
}

// class Cache {
//     public:
//         vector<vector<int>> state(SETS, vector<int>(WORDS_PER_BLOCK, 0));
//         vector<vector<int>> tag(SETS, vector<int>(WORDS_PER_BLOCK, 0));
// };