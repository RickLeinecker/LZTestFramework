#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cmath>
#include <bitset>
#include<iterator>
#include <string>
#include <string.h>

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
    char *genOutName(bool compress)
    {
        char *outputFileName; 
        int i = 0;

        // Add ".Z" to input file name when compressing. 
        if (compress)
        {
            while (inputFileName[i] != '\0')
            {
                i++;
            }
            outputFileName = (char *) malloc (i + 3); // 3 = current chars + ".Z"
            strcpy(outputFileName, inputFileName);
            strcat(outputFileName, ".Z");
            return outputFileName;
        }

        // Remove the ".Z" from input file name when decompressing.
        else
        {
            while (inputFileName[i] != '\0')
            {
                i++;
            }

            // Output file name will be at most as long as input.
            outputFileName = (char *) malloc (i);
            int j = i - 1;
            while (inputFileName[j] != '.')
            {
                j--;
            }
            strcpy(outputFileName, inputFileName);
            outputFileName[j] = '\0'; 
            return outputFileName;
        }
    }

    // This appends a single byte to the working string, writes it to the decompression
    // stream, and then processes the augmented working string for potential entry into
    // the symbol table.
    bool processByte(char byte, bool cScSc)
    {
        // Prevent buffer overflow.
        if (strIdx >= MAX_STR_LEN)
        {
            cout << "Error: exceeding maximum working string length. Exiting..." << endl;
            exit(1);
        }

        // Add to working string.
        workingString[strIdx] = byte;
        strIdx++;

        // Write to stream.
        if (cScSc == false)
        {
            outputFile->write(&byte, 1);
        }

        // Faster Char* toString() w/o strlen()
        string wrkStr(workingString, strIdx);

        // If the working string isn't in the symbol table, add it.
        // We capture the return value for cScSc special case handling.
        return addToTable(wrkStr, byte);
    }

    // Processes a symbol for potential substitution from the symbol table, and then either
    // pushes it to the decompression stream as the single byte it represents or fetches the sequence
    // of substituted bytes from the symbol table and pushes that sequence instead. 
    void processSymbol(unsigned int symbol)
    {
        // If symbol represents one byte, add to the working string and output.
        char singleByte = symbol;
        if (symbol < ASCII_OFFSET)
        {
            processByte(singleByte, TYPICAL_SYMBOL);
        }
        
        // Otherwise, substitute from table, push to output, and move to next symbol. 
        else
        {
            // Substitution iterator derived from searching the inverse symbol table 
            // for the symbol. 
            auto subIt = inverseTable.find(symbol);

            // cScSc Problem Handling
            if (subIt == inverseTable.end())
            {
                // Next symbol must begin with part of current working string. 
                int i = 0;
                while (i < strIdx)
                {
                    // Process each byte in working string, if symbol added, proceed
                    // with fetching symbol and continue the algorithm.
                    if (processByte(workingString[i], CSCSC) == SYMBOL_ADDED)
                    {
                        i++;
                        break;
                    }
                     i++;
                }

                // Number of bytes already processed from
                int pushedBytes = i;

                // Fetch substitute string now that it is safe to do so.
                subIt = inverseTable.find(symbol);
                const char *substituteString = subIt->second.c_str();
                int subStrLen = subIt->second.length();

                // Push the prefix bytes already processed above during cScSc symbol
                // construction. 
                for (int j = 0; j < pushedBytes; j++)
                {
                    outputFile->write(&substituteString[j], 1);
                }

                // Push the remaining bytes not processed during cScSc symbol construction.
                while (i < subStrLen)
                {
                    processByte(substituteString[i], TYPICAL_SYMBOL);
                    i++;
                }
            }

            // Typical Symbol Handling
            else
            {
                // Fetch substitute string now that it is safe to do so.
                subIt = inverseTable.find(symbol);
                const char *substituteString = subIt->second.c_str();
                int subStrLen = subIt->second.length();

                // Follow working string and table procedure for every byte
                // in the substitute string and push each to output stream.
                for (int i = 0; i < subStrLen; i++)
                {
                    processByte(substituteString[i], TYPICAL_SYMBOL);
                }
            }
        }
    }

    // Processes working string for potential entry into the symbol table. 
    bool addToTable(string wrkStr, char singleByte)
    {
        // Single character strings are ostensibly in the symbol table.
        if (strIdx < 2)
        {
            return false; // No table entry
        }
        
        // If the working string isn't in the symbol table, add it.
        auto it = symbolTable.find(wrkStr);
        if (currSymbolEntry < MAX_TABLE_SIZE && it == symbolTable.end())
        {
            // Add to table
            symbolTable[wrkStr] = currSymbolEntry;
            inverseTable[currSymbolEntry] = wrkStr;
            currSymbolEntry++;

            // Reset working string to byte represented by the latest symbol.
            workingString[0] = singleByte;
            strIdx = 1;

            // Expand bits per symbol if necessary
            if (currSymbolEntry >= symTabCapacity)
            {
                bitsPerSym++;
                symTabCapacity *= 2;

                // Decompression buffer space must be incremented, as it
                // is based on the number of bits per symbol.
                debuffSpace++;
            }

            return true;
        }

        return false;
    }

    // Pushes a symbol onto the compression stream. 
    void pushToCompStream(unsigned int symbol, int symbolBits)
    {
        char mask = 0b00000001;
        char firstBit = 0b00000000;

        // Revese bit order and push into buffer.
        for (int i = symbolBits; i > 0; i--)
        {
            // xxxxxxxxA Symbol
            // --------- &
            // _00000001 Mask
            //           =
            // _0000000A FirstBit
            firstBit = symbol & mask;
            symbol >>= 1;

            // xxxxxxx0 Buffer
            // -------- |
            // 0000000A FirstBit
            //          =
            // xxxxxxxA
            buffer <<= 1;
            buffSpace--;
            buffer |= firstBit;

            // When buffer is full, push byte to stream.
            if (buffSpace == 0)
            {
                char pushByte = 0b00000000;

                // Reverse bit order back and restore buffer space.
                while (buffSpace < BYTE_SIZE)
                {
                    firstBit = buffer & mask;
                    buffer >>= 1;
                    buffSpace++;

                    pushByte <<= 1;
                    pushByte |= firstBit;
                }

                // Reset buffer and output byte.
                buffer = 0b00000000; 
                outputFile->write(&pushByte, 1);
            }
        }
    }

    // Pushes a symbol onto the decompresion stream. 
    void processDecompStream(char readByte, ofstream &outputFile)
    {
        unsigned int mask = 0b00000001;
        unsigned int firstBit = 0b00000000;

        // Revese bit order and push into buffer.
        for (int i = BYTE_SIZE; i > 0; i--)
        {
            // xxxxxxxA Symbol
            // --------- &
            // 00000001 Mask
            //           =
            // 0000000A FirstBit
            firstBit = readByte & mask;
            readByte >>= 1;

            // xxxxxxxx0 Decompression Buffer
            // -------- |
            // 00000000A FirstBit
            //          =
            // xxxxxxxxA
            debuffer <<= 1;
            debuffSpace--;
            debuffer |= firstBit;

            // When buffer is full, process symbol.
            if (debuffSpace == 0)
            {
                // Copy object field to local container to prevent
                // change in loop during processing of current symbol. 
                int symbolBits = bitsPerSym;

                // Reverse bit order back and restore buffer space.
                unsigned int symbol = 0b00000000;
                while (debuffSpace < symbolBits)
                {
                    firstBit = debuffer & mask;
                    debuffer >>= 1;
                    debuffSpace++;

                    symbol <<= 1;
                    symbol |= firstBit;
                }

                debuffer = 0b00000000; 
                processSymbol(symbol);
            }
        }
    }

    public:
    LZW(char *inputFileName)
    {
        this->inputFileName = inputFileName;
        this->bitsPerSym = MIN_SYM_BITS;
        this->debuffSpace = bitsPerSym;

        // Symbol Table Capacity = 2^BitsPerSymbol
        // We will expand each symbol after exceeding this.
        this->symTabCapacity = pow(2, bitsPerSym);

        // We reserve enough space in the symbol table
        // for the first tier of entries. This prevents
        // unnecessary initial hash table expansions.
        // But since we will not store values 0 - 255 
        // in the symbol table, we reserve 2^(BitsPerSymbol - 1).
        this->symbolTable.reserve(pow(2, bitsPerSym - 1));
        this->inverseTable.reserve(pow(2, bitsPerSym - 1));
    }

    // Performs LZW compression on input stream specified by the inputFilename object field. 
    void compress()
    {
        // Open input file.
        ifstream inputFile (inputFileName, ios::in | ios::binary); // NOTE: Turn char * to String first???
        if (inputFile.fail())
        {
            cout << "Error: could not open input file." << endl;
            return;
        }
        this->inputFile = &inputFile;

        // Open output file.
        ofstream outputFile (genOutName(COMPRESS), ios::out | ios::binary); // change name of output file
        if (outputFile.fail())
        {
            cout << "Error: could not open output file." << endl;
            return;
        }
        this->outputFile = &outputFile;

        // Meta data
        outputFile.write(&MAGIC1, 1);
        outputFile.write(&MAGIC2, 1);
        outputFile.write(&MODE, 1);

        // Compress input stream.
        char readByte = 0;
        unsigned int symbol;
        auto it = symbolTable.begin();
        auto oldIt = symbolTable.begin();
        while (true)
        {
            // Copy object field to local container to prevent
            // change in loop during processing of current symbol. 
            int symbolBits = bitsPerSym;
            symbol = 0b00000000000000000000000000000000;

            // Read one byte from input.
            inputFile.read(&readByte, 1);

            // Process byte for potential substitution. 
            // ================================================================================

            // Append byte to working string
            workingString[strIdx] = readByte;
            strIdx++;

            // All single bytes exist abstractly in symbol table.
            // This saves some space and a minute amount of run time.
            if (strIdx == 1)
            {
                continue;
            }

            // Faster Char* toString() w/o strlen()
            string wrkStr(workingString, strIdx);

            // Save prior working string symbol, if any.
            oldIt = it;

            // If working string is already in symbol table, 
            // move to next byte. 
            it = symbolTable.find(wrkStr);
            if (it != symbolTable.end())
            {
                continue;
            }

            // Otherwise, add it to the symbol table.
            if (currSymbolEntry < MAX_TABLE_SIZE && strIdx > 1)
            {
                // Expand bits per symbol if necessary
                if (currSymbolEntry >= symTabCapacity)
                {
                    bitsPerSym++;
                    symTabCapacity *= 2;
                }

                // Add to table
                symbolTable[wrkStr] = currSymbolEntry;
                currSymbolEntry++;
            }

            // If prior working string is a single byte, push first byte 
            // into stream and set the new working string to the second 
            // byte in the working string (the latest read byte). 
            if (strIdx == 2)
            {
                unsigned char temp = workingString[0];
                symbol |= temp;
                workingString[0] = workingString[1];
                strIdx = 1;
            }

            // Otherwise, push the old working string as normal, and reset
            // the working string to the current byte. 
            else
            {
                symbol |= oldIt->second;
                workingString[0] = readByte;
                strIdx = 1;
            }
            // ================================================================================

            pushToCompStream(symbol, symbolBits);

            // Terminate reading if file or stream ends.
            if (inputFile.eof())
            {
                break;
            }
        }

        // Empty the buffer onto the output stream.
        buffer <<= buffSpace;
        char pushByte = 0b00000000;
        char firstBit = 0b00000000;
        char mask = 0b00000001;
        for (int i = 0; i < BYTE_SIZE; i++)
        {
            firstBit = buffer & mask;
            buffer >>= 1;

            pushByte <<= 1;
            pushByte |= firstBit;
        }

        // Output final padded byte.
        outputFile.write(&pushByte, 1);
        unsigned int hexPrint = pushByte;

        outputFile.close();
        inputFile.close();

    }

    void decompress()
    {
        // Open input file.
        ifstream inputFile (inputFileName, ios::in | ios::binary); // NOTE: Turn char * to String first???
        if (inputFile.fail())
        {
             cout << "Error: could not open input file." << endl;
            return;
        }
        this->inputFile = &inputFile;

        // Open output file.
        ofstream outputFile (genOutName(DECOMPRESS), ios::out | ios::binary); // change name of output file
        if (outputFile.fail())
        {
            cout << "Error: could not open output file." << endl;
            return;
        }
        this->outputFile = &outputFile;

        // Process Meta data
        char readByte = 0;
        inputFile.read(&readByte, 1);
        if (readByte != MAGIC1)
        {
            cout << "Error: invalid format." << endl;
            return;
        }
        inputFile.read(&readByte, 1);
        if (readByte != MAGIC2)
        {
            cout << "Error: invalid format." << endl;
            return;
        }
        inputFile.read(&readByte, 1);
        if (readByte != MODE)
        {
            cout << "Error: invalid mode." << endl;
            return;
        }

        // Decompress input stream.
        auto it = inverseTable.begin();
        auto oldIt = inverseTable.begin();
        while (true)
        {
            // Read one byte from input.
            inputFile.read(&readByte, 1);

            // Terminate stream if file or stream ends.
            if (inputFile.eof())
            {
                break;
            }

            // Build symbols from input bytes and substitute, if possible.
            processDecompStream(readByte, outputFile);
        }

        outputFile.close();
        inputFile.close();

    }
};

int main(int argc, char* argv[])
{
    // Compress
    if (argc < 3)
    {
        LZW test(argv[1]);
        test.compress();
    }

    // Decompress if d flag present. 
    else
    {
        LZW test(argv[2]);
        if (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "-D") == 0)
        {
            test.decompress();
        }
    }

    return 0;
}