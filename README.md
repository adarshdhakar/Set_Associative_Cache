# Set Associative Cache Implementation

This project implements an 8-way set associative cache simulator with write-through and write-no-allocate policies. 
The implementation demonstrates fundamental cache memory concepts and can be used for educational purposes or as a starting point for more complex cache implementations.

## Memory Address Structure (20-bit)

```
|----- Index -----|------ Tag ------|----- Offset -----|
|     6 bits      |       8 bits    |     6 bits       |
```

- **Tag**: 8 bits for block identification
- **Index**: 6 bits to select one of 64 sets
- **Offset**: 6 bits for word selection within a block

## Cache Configuration

- **Architecture**: 8-way set associative
- **Number of Sets**: 64
- **Block Size**: 16 words
- **Memory Size**: 16384 blocks (2^14)
- **Replacement Policy**: Random
- **Write Policy**: 
  - Write-through (all writes immediately update main memory)
  - Write-no-allocate (on write miss, block is not loaded into cache)

## Features

- Normal distribution-based address generation for realistic memory access patterns
- Full support for read and write operations
- Miss handling with pending state tracking
- Comprehensive hit/miss statistics
- Debug utilities for cache and memory state inspection

## Prerequisites

- C++ compiler with C++11 support
- Standard Template Library (STL)

## Building and Running

1. Clone the repository:
```bash
git clone https://github.com/yourusername/Set-Associative-Cache.git
cd Set-Associative-Cache
```

2. Compile the code:
```bash
g++ -std=c++11 Set_Associative_Cache.cpp -o cache_simulator
```

3. Run the simulator:
```bash
./cache_simulator
```

## Output Example

The simulator will generate output similar to this:

```
20-bit addressable memory simulation

Request 1 generated
Read Request
Hits: 0
Misses: 1
Request 1 processed

...

Final Statistics:
Hits: 7523 (Hit percentage: 75.23%)
Misses: 2477 (Miss percentage: 24.77%)
```

## Implementation Details

### Classes

1. **Block**: Represents a cache block
   - State (INVALID, VALID, MISPENDING)
   - Tag
   - Data block (vector of words)

2. **CPUReq**: CPU request handler
   - Address generation
   - Request type (READ/WRITE)
   - Address decomposition (tag, index, offset)

3. **CPUResp**: CPU response handler
   - Hit/miss tracking
   - Data return
   - Block information

4. **MemReq/MemResp**: Memory request/response handlers
   - Memory access simulation
   - Block transfer

5. **Cache**: Main cache implementation
   - Set associative structure
   - Read/write handling
   - Replacement policy
   - Write-through implementation

### Key Methods

- `CPUReq::generateRequest()`: Generates memory requests following a normal distribution
- `Cache::processRequest()`: Main request processing pipeline
- `Cache::readReq()`: Handle read requests with hit/miss logic
- `Cache::writeReq()`: Handle write requests with write-through policy
- `printCache()`: Debug utility to display cache state
- `printMem()`: Debug utility to display memory state

## Performance Characteristics

The implementation typically achieves:
- Hit rates between 45-50% for normal distribution access patterns
- Consistent performance across read operations
- Efficient handling of write operations through write-through policy

## Limitations

1. Simple replacement policy (random)
2. No prefetching implementation
3. No multi-level cache support
4. No cache coherence protocol

## Future Improvements

Potential areas for enhancement:
1. Implement LRU/FIFO replacement policies
2. Add prefetching mechanisms
3. Extend to multi-level cache hierarchy
4. Add cache coherence protocols
5. Implement more sophisticated address generation patterns
6. Add performance counters and detailed statistics

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Computer Architecture course materials
- Cache simulation research papers
- Computer Organization and Design (Patterson and Hennessy)
