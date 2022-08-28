#!/bin/bash

# Martin de Salterain

# ============================
# LZW Framework: largTest.h
# ============================
# You can run this script at the command line like so:
#
#   bash largeTest.sh <compressionSourceFile.cpp>


################################################################################
# Shell check.
################################################################################

# Running this script with sh instead of bash can lead to false positives on the
# test cases. Yikes! These checks ensure the script is not being run through the
# Bourne shell (or any shell other than bash).

#if [ "$BASH" != "/bin/bash" ]; then
#  echo ""
#  echo " Bloop! Please use bash to run this script, like so: bash test.sh"
#  echo ""
#  exit
#fi
#
#if [ -z "$BASH_VERSION" ]; then
#  echo ""
#  echo " Bloop! Please use bash to run this script, like so: bash test.sh"
#  echo ""
#  exit
#fi

################################################################################
# Initialization.
################################################################################

NUM_LARGE_FILES=3
NUM_TRIALS=100000

pass_cnt=0
lzw_runtime=0
avg_lzw_runtime=0
compress_runtime=0
avg_comp_runtime=0

total_test_comp_runtime=0
total_lin_comp_runtime=0
total_test_decomp_runtime=0
total_lin_decomp_runtime=0

total_test_filesize=0
total_lin_filesize=0
total_uncomp_filesize=0

################################################################################
# Check for commands that are required by this test script.
################################################################################

# This command is necessary in order to run all the test cases in sequence.
if ! [ -x "$(command -v seq)" ]; then
	echo ""
	echo " Error: seq command not found. You might see this message if you're"
	echo "        running this script on an old Mac system. Please be sure to"
	echo "        test your final code on Eustis. Aborting test script."
	echo ""
	exit
fi

# This command is necessary for various warning checks.
if ! [ -x "$(command -v grep)" ]; then
	echo ""
	echo " Error: grep command not found. You might see this message if you're"
	echo "        running this script on an old Mac system. Please be sure to"
	echo "        test your final code on Eustis. Aborting test script."
	echo ""
	exit
fi

# This command is necessary for comparison with existing LZW-based Linux Compress.
if ! [ -x "$(command -v compress)" ]; then
	echo ""
	echo " Error: missing compress. Please install by \"running apt install ncompress\"."
	echo ""
	exit
fi

# This command is necessary runtime logging.
if ! [ -x "$(command -v bc)" ]; then
	echo ""
	echo " Error: missing bc. Please install by \"running apt install bc\"."
     echo ""
	exit
fi

################################################################################
# Check that all required files are present.
################################################################################

if [ ! -f $1 ]; then
	echo ""
	echo " Error: You must place $1 in this directory before we can"
	echo "        proceed. Aborting test script."
	echo ""
	exit
fi
for i in `seq -f "%02g" 1 $NUM_LARGE_FILES`;
do
     if [ ! -f large$i ]; then
          echo ""
          echo " Error: You must place large$i in this directory before we can"
          echo "        proceed. Aborting test script."
          echo ""
          exit
     elif [ ! -f largeCompress$i ]; then
          echo ""
          echo " Error: You must place largeCompress$i in this directory before we can"
          echo "        proceed. Aborting test script."
          echo ""
          exit
     fi
done

################################################################################
# Compile LZW Code
################################################################################

# Attempt to compile.
g++ -o Compress.exe $1 2> /dev/null
compile_val=$?
if [[ $compile_val != 0 ]]; then
     echo "fail (failed to compile $1)"
fi
g++ -o Divide.exe divide.cpp 2> /dev/null
compile_val=$?
if [[ $compile_val != 0 ]]; then
     echo "fail (failed to compile divide.cpp)"
fi
################################################################################
# Run test cases with input specified at command line (standard test cases).
################################################################################

echo ""
echo "================================================================"
echo "Running Large test cases..."
echo "================================================================"
echo ""

for i in `seq -f "%02g" 1 $NUM_LARGE_FILES`;
do
     echo ""
     echo "  ++++++++++++++++++++++++++++"  
	echo "  + [Test Case] Large$i +"
     echo "  ++++++++++++++++++++++++++++" 
     echo ""
     echo "  Original File: "
     if [[ $i == 01 ]]; then
          echo "  bible.txt"
     elif [[ $i == 02 ]]; then
          echo "  E.coli"
     elif [[ $i == 03 ]]; then
          echo "  world192.txt"
     else 
          echo "  INVALID FILE"
     fi

     # Compression Trials
     for j in {0..$NUM_TRIALS}
     do
          # Compress with LZW, track runtime.
          ###################################
          start=`date +%s.%N`

          # Run program. Capture return value to check whether it crashes.
          ./Compress.exe -c large$i large$i.Z 2> /dev/null
          execution_val=$?
          end=`date +%s.%N`
          if [[ $execution_val != 0 ]]; then
               echo "fail (program crashed)"
               continue
	     fi

          trial_runtime=$( echo "$end - $start" | bc -l )
          lzw_runtime=$( echo "$lzw_runtime + $trial_runtime" | bc -l )
          ###################################

          # Compress with Linux Compress, track runtime.
          ##############################################
          start=`date +%s.%N`

          # Run program. Capture return value to check whether it crashes.
          compress -f < large$i > largeCompress$i.Z 2> /dev/null
          execution_val=$?
          end=`date +%s.%N`
          if [[ $execution_val != 0 ]]; then
               echo "fail (program crashed)"
               continue
	     fi

          trial_runtime=$( echo "$end - $start" | bc -l )
          compress_runtime=$( echo "$compress_runtime + $trial_runtime" | bc -l )

          # Reset input file.
          ./Compress.exe -d large$i.Z large$i
          ################################################
     done
     
     # Check for difference in compressed files.
     diff large$i.Z largeCompress$i.Z > myoutput.txt
     diff_val=$?
     if [[ $diff_val != 0 ]]; then
          echo "  Possible Error: Compressed Files large$i.Z and largeCompress$i.Z do not match."
     fi

     # Calculate average runtimes.
     echo ""
     echo "  ===================="
     echo "  Compression Runtimes"
     echo "  ====================" 
     echo ""
     echo "  Average LZW runtime for compressing large$i in seconds: "
     ./Divide.exe $lzw_runtime $NUM_TRIALS 
     echo ""
     echo "  Average Linux Compress runtime for compressing large$i in seconds: " 
     ./Divide.exe $compress_runtime $NUM_TRIALS
     echo ""

     # Comparative runtime
     echo "  Relative compression runtime: "
     ./Divide.exe $lzw_runtime $NUM_TRIALS $compress_runtime $NUM_TRIALS
     relative_comp_runtime=$?
     echo ""
     echo ""

     # Keep track of total runtime across all tests. 
     total_test_comp_runtime=$( echo "$total_test_comp_runtime + $lzw_runtime" | bc -l )
     total_lin_comp_runtime=$( echo "$total_lin_comp_runtime + $compress_runtime" | bc -l )

     # Compression ratio
     echo "  =================="
     echo "  Compression Ratios"
     echo "  ==================" 
     echo ""
     input_size=$(stat -c%s "large$i")
     lzw_output_size=$(stat -c%s "large$i.Z")
     comp_output_size=$(stat -c%s "largeCompress$i.Z")
     total_uncomp_filesize=$( echo "$total_uncomp_filesize + $input_size" | bc -l )
     total_test_filesize=$( echo "$total_test_filesize + $lzw_output_size" | bc -l )
     total_lin_filesize=$( echo "$total_lin_filesize + $comp_output_size" | bc -l )

     echo "  Original input size: $input_size bytes"
     echo "  Test  compressed file size: $lzw_output_size bytes"
     echo "  Linux compressed file size: $comp_output_size bytes"
     echo ""
     echo "  Tested compression ratio: " 
     ./Divide.exe $input_size $lzw_output_size
     tested_comp_ratio=$?
     echo ""
     echo "  Linux Compress compression ratio: " 
     ./Divide.exe $input_size $comp_output_size
     linux_comp_ratio=$?
     echo ""

     # Comparative compression ratio
     echo "  Ratio of tested compressed file size to Linux compressed file size: "
     ./Divide.exe $lzw_output_size $comp_output_size
     relative_comp_ratio=$?
     echo ""

     lzw_runtime=0
     compress_runtime=0

     # Decompression Trials
     for k in {0..$NUM_TRIALS}
     do
          # Deompress with LZW, track runtime.
          ###################################
          start=`date +%s.%N`

          # Run program. Capture return value to check whether it crashes.
          ./Compress.exe -d large$i.Z large$i
          execution_val=$?
          if [[ $execution_val != 0 ]]; then
               echo "fail (program crashed)"
               continue
	     fi
          
          end=`date +%s.%N`
          trial_runtime=$( echo "$end - $start" | bc -l )
          lzw_runtime=$( echo "$lzw_runtime + $trial_runtime" | bc -l )
          ###################################

          # Decompress with Linux Compress, track runtime.
          ##############################################
          start=`date +%s.%N`

          # Run program. Capture return value to check whether it crashes.
          compress -d < largeCompress$i.Z > largeCompress$i
          execution_val=$?
          if [[ $execution_val != 0 ]]; then
               echo "fail (program crashed)"
               continue
	     fi
          
          end=`date +%s.%N`
          trial_runtime=$( echo "$end - $start" | bc -l )
          compress_runtime=$( echo "$compress_runtime + $trial_runtime" | bc -l )

          #Reset compressed file
          compress -f < largeCompress$i > largeCompress$i.Z
          ################################################
     done

     compress -d < largeCompress$i.Z > largeCompress$i

     # Calculate average runtimes. 
     echo ""
     echo "  ======================"
     echo "  Decompression Runtimes"
     echo "  ======================" 
     echo ""
     echo "  Average LZW runtime for decompressing large$i.Z in seconds: "
     ./Divide.exe $lzw_runtime $NUM_TRIALS
     echo ""
     echo "  Average Linux Compress runtime for decompressing largeCompress$i.Z in seconds: "
     ./Divide.exe $compress_runtime $NUM_TRIALS
     echo ""

     # Comparative runtime
     echo ""
     echo "  Relative decompression runtime: "
     ./Divide.exe $lzw_runtime $NUM_TRIALS $compress_runtime $NUM_TRIALS
     relative_decomp_runtime=$?
     echo ""

     # Keep track of total runtime across all tests. 
     total_test_decomp_runtime=$( echo "$total_test_decomp_runtime + $lzw_runtime" | bc -l )
     total_lin_decomp_runtime=$( echo "$total_lin_decomp_runtime + $compress_runtime" | bc -l )

     # Check for difference in source and uncompressed files.
     diff large$i largeCompress$i > myoutput.txt
     diff_val=$?
     if [[ $diff_val != 0 ]]; then
          echo "  fail. decompressed file does not match source."
     else
          if [ $relative_comp_runtime -gt 0 ] || [ $relative_decomp_runtime -gt 0 ]; then
               echo "  WARNING: Test runtime longer than control."
          fi
          if [[ $relative_comp_ratio -gt 0 ]]; then
               echo ""
               echo "  /\/\/\/\/\/\/\/\/\//\/\/\/\/\/\/\/\/\/\/\/\/\/"
               echo "  FAILURE: compression ratio inferior to control."
               echo "  /\/\/\/\/\/\/\/\/\//\/\/\/\/\/\/\/\/\/\/\/\/\/"
               echo ""
          else
               echo "  PASS!"
               echo ""
               pass_cnt=`expr $pass_cnt + 1`
          fi
     fi
done

################################################################################
# Final thoughts.
################################################################################

echo ""
echo "================================================================"
echo " Final Report"
echo "================================================================"
echo ""
echo "  ===================="
echo "  Compression Runtimes"
echo "  ====================" 
echo ""
echo "  Total Tested Compression Runtime (seconds): $total_test_comp_runtime"
echo "  Total Linux Compression Runtime (seconds): $total_lin_comp_runtime"
echo ""
echo "  Average Tested Compression Runtime: "
total_trials=$( echo "$NUM_LARGE_FILES * $NUM_TRIALS" | bc -l )
./Divide.exe $total_test_comp_runtime $total_trials
echo ""
echo "  Average Linux Compression Runtime: "
./Divide.exe $total_lin_comp_runtime $total_trials
echo ""
echo "  Relative Compression Runtime Ratio: "
./Divide.exe $total_test_comp_runtime $total_trials $total_lin_comp_runtime $total_trials

echo ""
echo "  ======================"
echo "  Decompression Runtimes"
echo "  ======================" 
echo ""
echo "  Total Tested Decompression Runtime: $total_test_decomp_runtime"
echo "  Total Linux Decompression Runtime: $total_lin_decomp_runtime"
echo ""
echo "  Average Tested Decompression Runtime: "
./Divide.exe $total_test_decomp_runtime $total_trials
echo ""
echo "  Average Linux Decompression Runtime: "
./Divide.exe $total_lin_decomp_runtime $total_trials
echo ""
echo "  Relative Decompression Runtime Ratio: "
./Divide.exe $total_test_decomp_runtime $total_trials $total_lin_decomp_runtime $total_trials
echo ""
echo "  =================="
echo "  Compression Ratios"
echo "  ==================" 
echo ""
echo "  Total Uncompressed File Size in bytes: $total_uncomp_filesize"
echo "  Total Compressed File Size in bytes: $total_test_filesize"
echo "  Total Linux Comp File Size in bytes: $total_lin_filesize"
echo ""
echo "  Total Tested Compression Ratio: "
./Divide.exe $total_uncomp_filesize $total_test_filesize
echo ""
echo "  Total Linux Compression Ratio: "
./Divide.exe $total_uncomp_filesize $total_lin_filesize
echo ""
echo "  Relative Tested Compression: "
./Divide.exe $total_uncomp_filesize $total_test_filesize $total_uncomp_filesize $total_lin_filesize

if [ $pass_cnt -eq $NUM_LARGE_FILES ]; then
     echo ""
     echo " ALL FILES SUCCESSFULLY COMPRESSED AND DECOMPRESSED!"
	echo ""
	echo "              ,)))))))),,,"
	echo "            ,(((((((((((((((,"
	echo "            )\\\`\\)))))))))))))),"
	echo "     *--===///\`_    \`\`\`((((((((("
	echo "           \\\\\\ b\\  \\    \`\`)))))))"
	echo "            ))\\    |     ((((((((               ,,,,"
	echo "           (   \\   |\`.    ))))))))       ____ ,)))))),"
	echo "                \\, /  )  ((((((((-.___.-\"    \`\"((((((("
	echo "                 \`\"  /    )))))))               \\\`)))))"
	echo "                    /    ((((\`\`                  \\((((("
	echo "              _____|      \`))         /          |)))))"
	echo "             /     \\                 |          / ((((("
	echo "            /  --.__)      /        _\\         /   )))))"
	echo "           /  /    /     /'\`\"~----~\`  \`.       \\   (((("
	echo "          /  /    /     /\`              \`-._    \`-. \`)))"
	echo "         /_ (    /    /\`                    \`-._   \\ (("
	echo "        /__|\`   /   /\`                        \`\\\`-. \\ ')"
	echo "               /  /\`                            \`\\ \\ \\"
	echo "              /  /                                \\ \\ \\"
	echo "             /_ (                                 /_()_("
	echo "            /__|\`                                /__/__|"
	echo ""
	echo "                             Legendary."
	echo ""
	echo "                10/10 would run this program again."
	echo ""
	echo "  CONGRATULATIONS! You appear to be passing all the test cases!"
	echo ""
     echo "  Text animals courtesy of Dr. Sean Szumlanski."
else
	echo "                           ."
	echo "                          \":\""
	echo "                        ___:____     |\"\\/\"|"
	echo "                      ,'        \`.    \\  /"
	echo "                      |  o        \\___/  |"
	echo "                    ~^~^~^~^~^~^~^~^~^~^~^~^~"
	echo ""
	echo "                           (fail whale)"
	echo ""
	echo "  The fail whale is friendly and adorable! He is not here to"
	echo "  demoralize you, but rather, to bring you comfort and joy"
	echo "  in your time of need. \"Keep plugging away,\" he says! \"You"
	echo "  can do this!\""
     echo ""
     echo "  Text animals courtesy of Dr. Sean Szumlanski."
fi

################################################################################
# Cleanup phase.
################################################################################
rm -f *.exe
rm -f myoutput.txt
rm -f *.Z
