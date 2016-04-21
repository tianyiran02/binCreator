/*************************************************
||                       _      _		        ||
||    /\  /\  __  _  __ | | __ | | ____  ___ _  ||
||   /  \/  \/ _ ` |/ _ ` |/ _ ` |/ _  \/ _ ` | ||
||  / /\  /\  (_|    (_|    (_|    (_)   (_)  | ||
|| /_/  \/  \_.__, |\__, _|\__, _|\____/\___. | ||
|| =====================================|____/  ||
||                                              ||
**************************************************/

#include <string>
#include <string.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <math.h>
#include <algorithm>
#include <stdexcept>
#include <ctime>
#include <process.h>

using namespace std;

int string_to_hex(string& input);

int main()
{
string binfileAddr;
string binlength;
string binfilename;
string timeaddr;

fstream outputFile;
fstream logoFile;
string STRING;

long temp;
unsigned char counter;
long *addrtable;
long timeaddr_hex;
char *flashblock;

time_t t = time(0);

// delete file if exist
if( remove( "flash.bin" ) != 0 )
    perror( "Error deleting file" );
else
    puts( "File successfully deleted" );

outputFile.open("flash.bin",ios::out|ios::in|ios::binary|ios::trunc);
logoFile.open("logo.txt",ios::in);

// input the basic information
while(getline(logoFile,STRING)) // To get you all the lines.
{
    cout<<STRING<<endl; // Prints our STRING.
}
logoFile.close();
cout << "Author: Yiran TIAN" << endl;
cout << "\nTo use this tool, please place .bin file under name image1.bin, image2.bin, etc. Please input the starting address with the same order." << endl;
cout << "==================================================" << endl;

cout << "\n\nInput information." << endl;

// input flash size
std::cout << "\nInput bin file size:\n - For example, 1MB size flash (0x00000 - 0xFFFFF), type in: FFFFF\n";
// solid information
// std::getline (std::cin, binlength);
binlength = "7FFFFF"; // 8M flash

std::cout << "\nInput time storing address:\n";
// solid information
// getline(cin,timeaddr);
timeaddr = "301001";

// input image starting address
std::cout << "\nInput bin file starting address:\n - For example, 2 files in total, 1 file start at 0x002000, another tart at 0x004000. Then type in 2000;4000;\n";
// solid information
// std::getline (std::cin , binfileAddr);
binfileAddr = "401000;502000;"; // image1: STM32, image2:CC2530

// input image file name
// std::cout << "\nInput iamge files name:\nFor example, files name image1.bin and image2.bin, type in image1.bin;image2.bin;\n";
// std::getline (std::cin , binfilename);

// get the file numbers
int lastIndex = 0;
int fileNumber = 0;
for (int i = 0; i < binfileAddr.length() ; i++)
{
   if (binfileAddr[i] == ';')
      fileNumber++;
}

// verify whether there is enough image file
if(fileNumber > 7)
{
    std::cout << "\nThis program can only play with 5 .bin files\n\nProcess Failed!!!";
    return 0;
}

// get time address
timeaddr_hex = string_to_hex(timeaddr);

// get a new array for address table
addrtable = new long [fileNumber];

// do the loop again to store the address value
counter = 0;
for (int i = 0; i < binfileAddr.length() ; i++)
{
   if (binfileAddr[i] == ';')
   {
      // an address located
      if (lastIndex == 0)
      {
        // first index
        string tempstring = binfileAddr.substr(0,i);
        addrtable[0] = string_to_hex(tempstring);

        lastIndex = i + 1;
        counter++;
      }
      else
      {
        // other index
        string tempstring = binfileAddr.substr(lastIndex,(i-lastIndex));
        addrtable[counter] = string_to_hex(tempstring);

        lastIndex = i + 1;
        counter ++;
      }
   }
}

// create empty file first
temp = string_to_hex(binlength) + 1;

flashblock = new char [temp];
memset(flashblock,0xff,temp);

outputFile.seekp(0, ios::beg);
outputFile.write(flashblock, temp);

// save the time info
outputFile.seekp(timeaddr_hex);
char buffer[8];
buffer[0] = (t & 0xff00000000000000) >> 56;
buffer[1] = (t & 0xff000000000000) >> 48;
buffer[2] = (t & 0xff0000000000) >> 40;
buffer[3] = (t & 0xff00000000) >> 32;
buffer[4] = (t & 0xff000000) >> 24;
buffer[5] = (t & 0xff0000) >> 16;
buffer[6] = (t & 0xff00) >> 8;
buffer[7] = t & 0xff;
outputFile.write(buffer, 8);

outputFile.seekp(timeaddr_hex - 1);
buffer[0] = 0x01;
outputFile.write(buffer,1);

cout << "\n\n==================================================" << endl;
cout << "Process Result" << endl;
cout << "==================================================" << endl;
cout << "\nFlash image created with size: \n - 0x" << binlength << endl;
cout << "\nManufacture date/time saved in address: \n - 0x" << timeaddr << endl;
cout << "\nSaved manufacture date/time is: \n - (UTC)" << asctime(gmtime(&t)) << endl;;

// use a loop to fill in bin file data
lastIndex = 0;
temp = 0;

if(temp < fileNumber)
{
    fstream fin;
    long sizetemp;

    fin.open("image1.bin",ios::out|ios::in|ios::binary);

    std::cout << "\nProcessing Image1.bin";

    // see whether file open
    if (fin.is_open() == false)
    {
      std::cout << " - Fail to open file: image1.bin\n";
    }

    outputFile.seekp(addrtable[temp]);
    outputFile << fin.rdbuf();

    cout << "\n - Image1 saved in address: 0x" << std::hex << addrtable[temp] << endl;

    fin.seekg(0,ios_base::end);
    sizetemp = fin.tellg();

    cout << "\n - Image file size: 0x" << std::hex << sizetemp << " written!";
    cout << "\n - Image length written in address: 0x" << std::hex << (addrtable[temp] - 6) << endl;

    outputFile.seekp(addrtable[temp] - 6);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0xff000000) >> 32);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x00ff0000) >> 16);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x0000ff00) >> 8);
    outputFile << std::hex <<(unsigned char)(sizetemp & 0x000000ff);

    // Now, write the version number
    // It is STM32 version number
    fin.seekg(0x2000);
    outputFile.seekp(0x400FFE);
    char tempread;
    fin.get(tempread);
    outputFile.put(tempread);

    temp++;
    fin.close();
}

if(temp < fileNumber)
{
    fstream fin;
    long sizetemp;

    fin.open("image2.bin",ios::out|ios::in|ios::binary);

    std::cout << "\nProcessing Image2.bin";

    // see whether file open
    if (fin.is_open() == false)
    {
      std::cout << " - Fail to open file: image2.bin\n";
    }

    outputFile.seekp(addrtable[temp]);
    outputFile << fin.rdbuf();

    cout << "\n - Image2 saved in address: 0x" << std::hex << addrtable[temp] << endl;

    fin.seekg(0,ios_base::end);
    sizetemp = fin.tellg();

    cout << "\n - Image file size: 0x" << std::hex << sizetemp << " written!";
    cout << "\n - Image length written in address: 0x" << std::hex << (addrtable[temp] - 6) << endl;

    outputFile.seekp(addrtable[temp] - 6);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0xff000000) >> 32);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x00ff0000) >> 16);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x0000ff00) >> 8);
    outputFile << std::hex <<(unsigned char)(sizetemp & 0x000000ff);

    // Now, write the version number
    // It is CC2530 version number
    fin.seekg(0x2000);
    outputFile.seekp(0x501FFE);
    char tempread;
    fin.get(tempread);
    outputFile.put(tempread);

    temp++;
    fin.close();
}

if(temp < fileNumber)
{
    fstream fin;
    long sizetemp;

    fin.open("image3.bin",ios::out|ios::in|ios::binary);

    std::cout << "\nProcessing Image3.bin";

    // see whether file open
    if (fin.is_open() == false)
    {
      std::cout << " - Fail to open file: image3.bin\n";
    }

    outputFile.seekp(addrtable[temp]);
    outputFile << fin.rdbuf();

    cout << "\n - Image3 saved in address: 0x" << std::hex << addrtable[temp] << endl;

    fin.seekg(0,ios_base::end);
    sizetemp = fin.tellg();

    cout << "\n - Image file size: 0x" << std::hex << sizetemp << " written!";
    cout << "\n - Image length written in address: 0x" << std::hex << (addrtable[temp] - 4) << endl;

    outputFile.seekp(addrtable[temp] - 4);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0xff000000) >> 32);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x00ff0000) >> 16);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x0000ff00) >> 8);
    outputFile << std::hex <<(unsigned char)(sizetemp & 0x000000ff);

    temp++;
    fin.close();
}

if(temp < fileNumber)
{
    fstream fin;
    long sizetemp;

    fin.open("image4.bin",ios::out|ios::in|ios::binary);

    std::cout << "\nProcessing Image4.bin";

    // see whether file open
    if (fin.is_open() == false)
    {
      std::cout << " - Fail to open file: image4.bin\n";
    }

    outputFile.seekp(addrtable[temp]);
    outputFile << fin.rdbuf();

    cout << "\n - Image4 saved in address: 0x" << std::hex << addrtable[temp] << endl;

    fin.seekg(0,ios_base::end);
    sizetemp = fin.tellg();

    cout << "\n - Image file size: 0x" << std::hex << sizetemp << " written!";
    cout << "\n - Image length written in address: 0x" << std::hex << (addrtable[temp] - 4) << endl;

    outputFile.seekp(addrtable[temp] - 4);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0xff000000) >> 32);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x00ff0000) >> 16);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x0000ff00) >> 8);
    outputFile << std::hex <<(unsigned char)(sizetemp & 0x000000ff);

    temp++;
    fin.close();
}

if(temp < fileNumber)
{
    fstream fin;
    long sizetemp;

    fin.open("image5.bin",ios::out|ios::in|ios::binary);

    std::cout << "\nProcessing Image5.bin";

    // see whether file open
    if (fin.is_open() == false)
    {
      std::cout << " - Fail to open file: image5.bin\n";
    }

    outputFile.seekp(addrtable[temp]);
    outputFile << fin.rdbuf();

    cout << "\n - Image5 saved in address: 0x" << std::hex << addrtable[temp] << endl;

    fin.seekg(0,ios_base::end);
    sizetemp = fin.tellg();

    cout << "\n - Image file size: 0x" << std::hex << sizetemp << " written!";
    cout << "\n - Image length written in address: 0x" << std::hex << (addrtable[temp] - 4) << endl;

    outputFile.seekp(addrtable[temp] - 4);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0xff000000) >> 32);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x00ff0000) >> 16);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x0000ff00) >> 8);
    outputFile << std::hex <<(unsigned char)(sizetemp & 0x000000ff);

    temp++;
    fin.close();
}

if(temp < fileNumber)
{
    fstream fin;
    long sizetemp;

    fin.open("image6.bin",ios::out|ios::in|ios::binary);

    std::cout << "\nProcessing Image6.bin";

    // see whether file open
    if (fin.is_open() == false)
    {
      std::cout << " - Fail to open file: image6.bin\n";
    }

    outputFile.seekp(addrtable[temp]);
    outputFile << fin.rdbuf();

    cout << "\n - Image6 saved in address: 0x" << std::hex << addrtable[temp] << endl;

    fin.seekg(0,ios_base::end);
    sizetemp = fin.tellg();

    cout << "\n - Image file size: 0x" << std::hex << sizetemp << " written!";
    cout << "\n - Image length written in address: 0x" << std::hex << (addrtable[temp] - 4) << endl;

    outputFile.seekp(addrtable[temp] - 4);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0xff000000) >> 32);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x00ff0000) >> 16);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x0000ff00) >> 8);
    outputFile << std::hex <<(unsigned char)(sizetemp & 0x000000ff);

    temp++;
    fin.close();
}


if(temp < fileNumber)
{
    fstream fin;
    long sizetemp;

    fin.open("image7.bin",ios::out|ios::in|ios::binary);

    std::cout << "\nProcessing Image7.bin";

    // see whether file open
    if (fin.is_open() == false)
    {
      std::cout << " - Fail to open file: image7.bin\n";
    }

    outputFile.seekp(addrtable[temp]);
    outputFile << fin.rdbuf();

    cout << "\n - Image7 saved in address: 0x" << std::hex << addrtable[temp] << endl;

    fin.seekg(0,ios_base::end);
    sizetemp = fin.tellg();

    cout << "\n - Image file size: 0x" << std::hex << sizetemp << " written!";
    cout << "\n - Image length written in address: 0x" << std::hex << (addrtable[temp] - 4) << endl;

    outputFile.seekp(addrtable[temp] - 4);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0xff000000) >> 32);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x00ff0000) >> 16);
    outputFile << std::hex <<(unsigned char)((sizetemp & 0x0000ff00) >> 8);
    outputFile << std::hex <<(unsigned char)(sizetemp & 0x000000ff);

    temp++;
    fin.close();
}

cout << "\nSaving valid flag in address:\n - 0x7FFFFE";
outputFile.seekp(0x7FFFFE);
outputFile << std::hex << (unsigned char)(0x99);
cout << "\n - Value saved: 0x99";

outputFile.close();

// output result
cout << "\n\n" << fileNumber << " image saved in flash.bin file.\n\nProcess Success!!!\n\n" << endl;

system ("pause");

return 0;
}




int string_to_hex(std::string& input)
{
    static const char* const lut = "0123456789ABCDEF";
    size_t len = input.length();

    int output = 0;
    int temp;
    for (size_t i = 0; i < len; i++)
    {
        unsigned char c = input[i];

        for(temp = 0; temp <= 15 ; temp++)
        {
           if (c == lut[temp])
              break;
        }
        output += temp * pow(16,len-i-1);
    }
    return output;
}

