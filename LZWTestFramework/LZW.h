#include <iostream>
#include <fstream>
#include <unordered_map>

using namespace std;

const int ASCII_OFFSET = 256;
const int BYTE_SIZE = 8;
const int MAX_TABLE_SIZE = 65536;
const int MIN_SYM_BITS = 9;
const int MAX_STR_LEN = 500;

const bool COMPRESS = true;
const bool DECOMPRESS = false;

const char MAGIC1 = 0x1f;
const char MAGIC2 = 0x9d;const int MAX_SYM_BITS = 16;
const char MODE = 0b10010000; // Will always use the reset marker.

const bool SYMBOL_ADDED = true;
const bool CSCSC = true;
const bool TYPICAL_SYMBOL = false;

class LZW
{   private:

    char *inputFileName;
    ifstream *inputFile;
    ofstream *outputFile;

    // Compression Buffer
    char buffer = 0b00000000;
    int buffSpace = BYTE_SIZE;

    // Decompression Buffer
    unsigned int debuffer = 0b00000000;
    int debuffSpace;

    // Symbol Table
    double bitsPerSym;
    int symTabCapacity;
    int currSymbolEntry = ASCII_OFFSET + 1;
    unordered_map<string, unsigned int> symbolTable;

    // Inverse Symbol Table, for decompression
    unordered_map<unsigned int, string> inverseTable;


    // Working String
    int strIdx = 0;
    char workingString[MAX_STR_LEN];

    // Generates the output file name based on the input file name and
    // whether the program is compressing or decompressing. 
    char *genOutName(bool compress);

    // This appends a single byte to the working string, writes it to the decompression
    // stream, and then processes the augmented working string for potential entry into
    // the symbol table.
    bool processByte(char byte, bool cScSc);

    // Processes a symbol for potential substitution from the symbol table, and then either
    // pushes it to the decompression stream as the single byte it represents or fetches the sequence
    // of substituted bytes from the symbol table and pushes that sequence instead. 
    void processSymbol(unsigned int symbol);

    // Processes working string for potential entry into the symbol table. 
    bool addToTable(string wrkStr, char singleByte);

    // Pushes a symbol onto the compression stream. 
    void pushToCompStream(unsigned int symbol, int symbolBits);

    // Pushes a symbol onto the decompresion stream. 
    void processDecompStream(char readByte, ofstream &outputFile);

    public:
    LZW(char *inputFileName);

    // Performs LZW compression on input stream specified by the inputFilename object field. 
    void compress();

    void decompress();
};