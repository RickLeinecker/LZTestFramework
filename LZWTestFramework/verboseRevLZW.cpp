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
    char workingString[500];

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

    bool processByte(char byte, bool cScSc)
    {
        cout << "               ";
        cout << "Processing single byte, \"" << byte << "\" and adding to working string.." << endl;
        // Add to working string.
        workingString[strIdx] = byte;
        workingString[strIdx + 1] = '\0';
        strIdx++;

        // Write to stream.
        if (cScSc == false)
        {
            cout << "               ";
            cout << "(no cScSc) Writing \"" << byte << "\" to output stream." << endl;
            outputFile->write(&byte, 1);
        }

        // Faster Char* toString() w/o strlen()
        string wrkStr(workingString, strIdx);

        cout << "               ";
        cout << "Processing working string, \"" << wrkStr << "\" for possible table input." << endl;
        // If the working string isn't in the symbol table, add it.
        return addToTable(wrkStr, byte);
    }

    void processSymbol(unsigned int symbol)
    {
        cout << endl << "     Processing symbol " << bitset<16>(symbol) << endl;
        // If symbol represents one byte, add to the working string and output.
        char singleByte = symbol;
        if (symbol < ASCII_OFFSET)
        {
            cout << "     Symbol " << bitset<16>(symbol);
            cout << " represents one byte. Processing typical single byte symbol." << endl;
            processByte(singleByte, TYPICAL_SYMBOL);
        }
        
        // Otherwise, substitute from table, push to output, and move to next symbol. 
        else
        {
            cout << "     Symbol " << bitset<16>(symbol) << " :: " << symbol;
            cout << " represents a substituted string.";
            cout << " Processing typical substituion symbol." << endl;
            auto subIt = inverseTable.find(symbol);

            // cScSc Problem Handling
            if (subIt == inverseTable.end())
            {
                cout << "     cScSc Problem detected!" << endl;
                // Next symbol must begin with part of current working string. 
                int i = 0;
                while (i < strIdx)
                {
                    cout << "          Processing byte # " << i << " of " << workingString;
                    cout << endl;
                    // Process each byte in working string, if symbol added, proceed
                    // with fetching symbol and continue the algorithm.
                    if (processByte(workingString[i], CSCSC) == SYMBOL_ADDED)
                    {
                        cout << "          Detected added symbol." << endl;
                        i++;
                        break;
                    }
                     i++;
                }

                // Number of bytes already processed from
                int pushedBytes = i;

                cout << "     ";
                cout << "Fetching symbol added to table by cScSc handling." << endl;
                // Fetch substitute string now that it is safe to do so.
                subIt = inverseTable.find(symbol);
                const char *substituteString = subIt->second.c_str();
                int subStrLen = subIt->second.length();

                cout << "     ";
                cout << "Found substitute: \"" << substituteString << "\"" << endl;
                cout << "     ";
                cout << "Writing string to file..." << endl;
                for (int j = 0; j < pushedBytes; j++)
                {
                    cout << "     ";
                    cout << "Writing \"" << substituteString[j] << "\" to file..." << endl; 
                    outputFile->write(&substituteString[j], 1);
                }
                // outputFile->write(substituteString, pushedBytes);

                while (i < subStrLen)
                {
                    processByte(substituteString[i], TYPICAL_SYMBOL);
                    i++;
                }
            }

            // Typical Symbol Handling
            else
            {
                cout << "     ";
                cout << "Fetching symbol from existing table entry." << endl;
                // Fetch substitute string now that it is safe to do so.
                subIt = inverseTable.find(symbol);
                const char *substituteString = subIt->second.c_str();
                int subStrLen = subIt->second.length();

                cout << "     ";
                cout << "Found substitute: \"" << substituteString << "\"" << endl;
                // Follow working string and table procedure for every byte
                // in the substitute string and push each to output stream.
                for (int i = 0; i < subStrLen; i++)
                {
                    cout << "          Processing byte # " << i << " of \"" << substituteString << "\"";
                    cout << endl;
                    processByte(substituteString[i], TYPICAL_SYMBOL);
                }
            }
        }
    }

    bool addToTable(string wrkStr, char singleByte)
    {
        // Single character strings are ostensibly in the symbol table.
        if (strIdx < 2)
        {
            return false;
        }
        
        // If the working string isn't in the symbol table, add it.
        auto it = symbolTable.find(wrkStr);
        if (it == symbolTable.end())
        {
            cout << "                    ";
            cout << "Working string, \"" << wrkStr << "\" not found in symbol table.";

            unsigned int entryInRange = currSymbolEntry % MAX_TABLE_SIZE;

            // Look for existing entry at current index. 
            auto invIt = inverseTable.find(entryInRange);
            if (invIt != inverseTable.end())
            {
                cout << "                    ";
                cout << "Current entry, \"" << entryInRange << "\" occupied. Deleting string \"" << invIt->second << "\" from symbol table.";
                
                // Entry found. Delete from symbol table.
                symbolTable.erase(invIt->second);
                inverseTable.erase(invIt);
            }
            cout << " Adding working string as symbol: ";
            printf("%d\n", entryInRange);
            // Add to table
            symbolTable[wrkStr] = entryInRange;
            inverseTable[entryInRange] = wrkStr;
            currSymbolEntry++;

            cout << "                    ";
            cout << "Current symbol table entry space: " << (unsigned int)entryInRange << endl;
            cout << "                    ";
            cout << "Current symbol table capacity: " << symTabCapacity << endl;
            // Reset working string to byte represented by the latest symbol.
            workingString[0] = singleByte;
            workingString[1] = '\0';
            strIdx = 1;

            cout << "                    ";
            cout << "New working string: \"" << workingString << "\"" << endl;

            // Expand bits per symbol if necessary
            if (entryInRange >= symTabCapacity)
            {
                bitsPerSym++;
                symTabCapacity *= 2;
                debuffSpace++;
                cout << "                    ";
                cout << "Symbol bits expanded to: " << bitsPerSym << endl;
            }

            return true;
        }

        return false;
    }

        // Pushes a symbol onto the compression stream. 
    void pushToCompStream(unsigned int symbol, int symbolBits)
    {
        unsigned int hexSym = symbol;
        // cout << endl << "SYMBOL: " << bitset<9>(symbol) << " : ";
        cout << "(" << bitset<16>(symbol)<< ") : ";
        printf("%d\n", symbol);
        // cout << hex << hexSym << endl; // FOR DEBUGGING. DELETE
        char mask = 0b00000001;
        char firstBit = 0b00000000;

        for (int i = symbolBits; i > 0; i--)
        {
            // Revese bit order and push into buffer.

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
            cout << "BUFFER : "<< i << "::"<< bitset<8>(buffer) << endl; // FOR DEBUGGING. DELETE

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

                unsigned int hexPrint = pushByte;
                buffer = 0b00000000; 
                outputFile->write(&pushByte, 1);
                cout << "OUTPUT BYTE: " << bitset<8>(pushByte) << " : ";
                cout << hex << hexPrint;
                cout << endl;
            }
        }
    }

    // Pushes a symbol onto the decompresion stream. 
    void processDecompStream(char readByte, ofstream &outputFile)
    {
        cout << endl << "Reading next byte from input..." << endl;
        cout << endl << "ReadByte: "<< bitset<8>(readByte) << endl; // FOR DEBUGGING. DELETE
        unsigned int mask = 0b00000001;
        unsigned int firstBit = 0b00000000;

        printf("Current bits per symbol = %f\n", bitsPerSym);
        printf("Debuff Space = %d\n", debuffSpace);

        for (int i = BYTE_SIZE; i > 0; i--)
        {
            // Revese bit order and push into buffer.

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
            cout << "DEBUFFER : "<< i << "::"<< bitset<16>(debuffer) << endl; // FOR DEBUGGING. DELETE

            // When buffer is full, process symbol.
            if (debuffSpace == 0)
            {
                int symbolBits = bitsPerSym;
                unsigned int symbol = 0b00000000;
                printf("Found a symbol... Bits in this symbol = %d, ", symbolBits);
                printf("Current bits per symbol = %f\n", bitsPerSym);

                // Reverse bit order back and restore buffer space.
                while (debuffSpace < symbolBits)
                {
                    cout << "          SYMBUFF: ";
                    printf(" :: %d : ", debuffSpace);
                    firstBit = debuffer & mask;
                    debuffer >>= 1;
                    debuffSpace++;

                    symbol <<= 1;
                    symbol |= firstBit;
                    cout << bitset<16>(symbol) << endl;
                }

                debuffer = 0b00000000; 
                cout << "OUTPUT SYMBOL: " << bitset<16>(symbol) << " : " << symbol << endl; // FOR DEBUGGING. DELETE
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

    // Write a member function to push the current symbol to the bitstream using bitsPerSym 
    // bits.

    // Write a member function which expands the number of bits per symbol and the current
    // capacity before next expansion. DO NOT MANUALLY EXPAND THE HASH MAP
    // NOTE that debuffSpace may need to be expanded by the difference between the previous
    // bitsPerSym and the new bitsPetSym

    // AT THE END OF COMPRESSION, EMPTY THE BUFFER

    void compress()
    {
        // Open input file.
        ifstream inputFile (inputFileName, ios::in | ios::binary); // NOTE: Turn char * to String first???
        if (inputFile.fail())
        {
            cout << "Error: could not open file \"" << inputFileName << "\" " << endl;
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

        // Initialize working string
        workingString[0] = '\0';

        // Compress input stream.
        char readByte = 0;
        unsigned int symbol;
        auto it = symbolTable.begin();
        auto oldIt = symbolTable.begin();
        while (true)
        {
            symbol = 0b00000000000000000000000000000000;
            int symbolBits = bitsPerSym;

            // Read one byte from input.
            inputFile.read(&readByte, 1);

            // Process byte for potential substitution. 
            // ================================================================================

            cout << endl << "Read Byte: \"";
            printf("%c\"\n", readByte);
            cout << "Adding read byte to working string." << endl;
            // Append byte to working string
            workingString[strIdx] = readByte;
            workingString[strIdx + 1] = '\0';
            strIdx++;

            cout << "Augmented working string: \"" << workingString << "\"" << endl << endl;

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
                // If there is no next byte, push the symbol
                // representing the working string. 
                if (inputFile.eof())
                {
                    symbol |= oldIt->second;
                    pushToCompStream(symbol, symbolBits);
                    break;
                }

                continue;
            }

            // Otherwise, add it to the symbol table.
            if (strIdx > 1)
            {
                unsigned int entryInRange = currSymbolEntry % MAX_TABLE_SIZE;

                // Expand bits per symbol if necessary
                if (entryInRange >= symTabCapacity)
                {
                    bitsPerSym++;
                    symTabCapacity *= 2;

                    cout << "                    ";
                    cout << "EXPANDING =====" << endl;
                    cout << "                    ";
                    cout << "Number of symbols per bit expanded to: ";
                    printf("%d\n", (int)bitsPerSym);
                    cout << "                    ";
                    cout << "New symbol table capacity: ";
                    printf("%d\n", symTabCapacity);
                }

                // Look for existing entry at current index. 
                auto invIt = inverseTable.find(entryInRange);
                if (invIt != inverseTable.end())
                {
                    // Entry found. Delete from symbol table.
                    symbolTable.erase(invIt->second);
                    inverseTable.erase(invIt);
                }

                cout << "                    ";
                cout << "Adding working string: \"" << wrkStr <<"\"";
                printf(" as symbol: %d\n", entryInRange);

                // Add to table
                symbolTable[wrkStr] = entryInRange;
                inverseTable[entryInRange] = wrkStr;
                currSymbolEntry++;

                cout << "                    ";
                cout << "Current symbol table entry space: ";
                printf("%d\n", currSymbolEntry);
                cout << "                    ";
                cout << "Current symbol table capacity: ";
                printf("%d\n", symTabCapacity);
            }

            // If prior working string is a single byte, push first byte 
            // into stream and set the new working string to the second 
            // byte in the working string. 
            if (strIdx == 2)
            {
                unsigned char temp = workingString[0];
                symbol |= temp;
                cout << "     In single byte prior working string case:" << endl;
                cout << "Symbol = " << symbol << ", First Byte of Working String = " << bitset<8>(workingString[0]) << endl;
                workingString[0] = workingString[1];
                workingString[1] = '\0';
                strIdx = 1;
                cout << "                    ";
                cout << "New working string: \"" << workingString << "\"" << endl;
            }

            // Otherwise, push the old working string as normal, and reset
            // the working string to the current byte. 
            else
            {
                // string oldWrkStr(workingString, strIdx - 1);
                //iterator = symbolTable.find(oldWrkStr); // OPTIMIZE THIS WITH ITERATOR FIELD
                //symbol = iterator->second;
                symbol |= oldIt->second;
                workingString[0] = readByte;
                workingString[1] = '\0';
                strIdx = 1;

                cout << "                    ";
                cout << "New working string: \"" << workingString << "\"" << endl;

                cout << "    Fetched symbol for prior working string: ";
                printf("%d\n", oldIt->second);

                // DEBUGGING NOTE: FIGURE OUT WHAT THE it and oldIt map to.
            }
            // ================================================================================

            char sym = symbol;
            if (symbol < 256)
                cout << "SYMBOL: " << sym << endl;
            else
                cout << "SYMBOL: " << symbol << endl;
            // Output compressed stream. 
            pushToCompStream(symbol, symbolBits);

            // Terminate reading if file or stream ends.
            if (inputFile.eof())
            {
                break;
            }
            // cout << endl << "WE'RE PUSHING TO THE STREAM! WOOOOOOOO!" << endl; // FOR DEBUGGING. DELETE

        }

        // char sym = symbol;
        // if (symbol < 256)
        //     cout << "SYMBOL: " << sym << endl;
        // else
        //     cout << "SYMBOL: " << symbol << endl;
        // pushToCompStream(symbol, outputFile);

        // Empty the buffer onto the output stream.
        // cout << "BUFFER : "<< bitset<8>(buffer) << endl; // FOR DEBUGGING. DELETE
        char pushByte = 0b00000000;
        char firstBit = 0b00000000;
        char mask = 0b00000001;
        if (buffer != 0)
        {
            buffer <<= buffSpace;
            for (int i = 0; i < BYTE_SIZE; i++)
            {
                firstBit = buffer & mask;
                buffer >>= 1;

                pushByte <<= 1;
                pushByte |= firstBit;
            }

            // Output final padded byte.
            outputFile.write(&pushByte, 1);
        }

        // unsigned int hexPrint = pushByte;
        // cout << "OUTPUT BYTE: " << bitset<8>(pushByte) << " : ";
        // cout << hex << hexPrint;
        // cout << endl;

        outputFile.close();
        inputFile.close();

    }

    void decompress()
    {
        // Open input file.
        ifstream inputFile (inputFileName, ios::in | ios::binary); // NOTE: Turn char * to String first???
        if (inputFile.fail())
        {
            cout << "Error: could not open file \"" << inputFileName << "\" " << endl;
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


        // Initialize working string
        strIdx = 0;

        // Decompress input stream.
        auto it = inverseTable.begin();
        auto oldIt = inverseTable.begin();
        while (true)
        {
            // Read one byte from input.
            inputFile.read(&readByte, 1);

            // cout << endl << "WE'RE PUSHING TO THE STREAM! WOOOOOOOO!" << endl; // FOR DEBUGGING. DELETE

            // Build symbols from input bytes and substitute, if possible.
            processDecompStream(readByte, outputFile);

            // Terminate stream if file or stream ends.
            if (inputFile.eof())
            {
                break;
            }
        }

        outputFile.close();
        inputFile.close();

    }
};

int main(int argc, char* argv[])
{

    //This placeholder code reads bytes from stdin and writes the bitwise complement
    //to stdout (obviously this is useless for the assignment, but maybe it gives some impression
    //of how I/O works in C++)

    //The .get() method of std::istream objects (like the standard input stream std::cin) reads a single unformatted
    //byte into the provided character (notice that c is passed by reference and is modified by the method).
    //If the byte cannot be read, the return value of the method is equivalent to the boolean value 'false'.

    if (argc < 3)
    {
        LZW test(argv[1]);
        test.compress();
    }
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