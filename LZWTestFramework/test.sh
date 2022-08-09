#!/bin/bash

# Martin de Salterain

# ============================
# LZW Framework: test.h
# ============================
# You can run this script at the command line like so:
#
#   bash test.sh


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

NUM_CANTBRY_FILES=11
NUM_TRIALS=10000
pass_cnt=0
lzw_runtime=0
avg_lzw_runtime=0
compress_runtime=0
avg_comp_runtime=0

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

if [ ! -f LZW.cpp ]; then
	echo ""
	echo " Error: You must place PotionMaster.java in this directory before we can"
	echo "        proceed. Aborting test script."
	echo ""
	exit
:'elif [ ! -d test_files ]; then
	echo ""
	echo " Error: You must place the sample_output folder in this directory"
	echo "        before we can proceed. Aborting test script."
	echo ""
	exit
elif [ ! -d test_files\canterbury ]; then
	echo ""
	echo " Error: You must place the caterbury folder in the sample_files directory"
	echo "        before we can proceed. Aborting test script."
	echo ""
	exit
elif [ ! -d test_files\calgary ]; then
	echo ""
	echo " Error: You must place the calgary folder in the sample_files directory"
	echo "        before we can proceed. Aborting test script."
	echo ""
	exit
elif [ ! -d test_files\artificial ]; then
	echo ""
	echo " Error: You must place the artificial folder in the sample_files directory"
	echo "        before we can proceed. Aborting test script."
	echo ""
	exit
elif [ ! -d test_files\large ]; then
	echo ""
	echo " Error: You must place the large folder in the sample_files directory"
	echo "        before we can proceed. Aborting test script."
	echo ""
	exit
elif [ ! -d test_files\misc ]; then
	echo ""
	echo " Error: You must place the misc folder in the sample_files directory"
	echo "        before we can proceed. Aborting test script."
	echo ""
	exit'
fi

################################################################################
# Compile LZW Code
################################################################################

# Attempt to compile.
g++ -o Compress.exe $1 2> /dev/null
compile_val=$?
if [[ $compile_val != 0 ]]; then
     echo "fail (failed to compile LZW.cpp)"
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
echo "Running Canterbury test cases..."
echo "================================================================"
echo ""

for i in `seq -f "%02g" 1 $NUM_CANTBRY_FILES`;
do
     echo ""
     echo "  ++++++++++++++++++++++++++++"  
	echo "  + [Test Case] Canterbury$i +"
     echo "  ++++++++++++++++++++++++++++" 
     echo ""

     # Compression Trials
     for j in {0..$NUM_TRIALS}
     do
          # Compress with LZW, track runtime.
          ###################################
          start=`date +%s.%N`

          # Run program. Capture return value to check whether it crashes.
          ./Compress.exe -c canterbury$i canterbury$i.Z 2> /dev/null
          execution_val=$?
          end=`date +%s.%N`
          if [[ $execution_val != 0 ]]; then
               echo "fail (program crashed)"
               continue
	     fi

          lzw_runtime+=$( echo "$end - $start" | bc -l )
          ###################################

          # Compress with Linux Compress, track runtime.
          ##############################################
          start=`date +%s.%N`

          # Run program. Capture return value to check whether it crashes.
          compress -f < canterbury$i > canterburyCompress$i.Z 2> /dev/null
          execution_val=$?
          end=`date +%s.%N`
          if [[ $execution_val != 0 ]]; then
               echo "fail (program crashed)"
               continue
	     fi

          compress_runtime+=$( echo "$end - $start" | bc -l )

          # Reset input file.
          ./Compress.exe -d canterbury$i.Z canterbury$i
          ################################################
     done
     
     # Check for difference in compressed files.
     diff canterbury$i.Z canterburyCompress$i.Z > myoutput.txt
     diff_val=$?
     if [[ $diff_val != 0 ]]; then
          echo "  Possible Error: Compressed Files canterbury$i.Z and canterburyCompress$i.Z do not match."
     fi

     # Calculate average runtimes.
     echo ""
     echo "  ===================="
     echo "  Compression Runtimes"
     echo "  ====================" 
     echo ""
     echo "  Average LZW runtime for compressing canterbury$i in seconds: "
     ./Divide.exe $lzw_runtime $NUM_TRIALS 
     echo ""
     echo "  Average Linux Compress runtime for compressing canterbury$i in seconds: " 
     ./Divide.exe $compress_runtime $NUM_TRIALS
     echo ""

     # Comparative runtime
     echo "  Relative compression runtime: "
     ./Divide.exe $lzw_runtime $NUM_TRIALS $compress_runtime $NUM_TRIALS
     relative_comp_runtime=$?
     echo ""
     echo ""

     # Compression ratio
     echo "  =================="
     echo "  Compression Ratios"
     echo "  ==================" 
     echo ""
     input_size=$(stat -c%s "canterbury$i")
     lzw_output_size=$(stat -c%s "canterbury$i.Z")
     comp_output_size=$(stat -c%s "canterburyCompress$i.Z")
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
          ./Compress.exe -d canterbury$i.Z canterbury$i
          execution_val=$?
          if [[ $execution_val != 0 ]]; then
               echo "fail (program crashed)"
               continue
	     fi
          
          end=`date +%s.%N`
          lzw_runtime+=$( echo "$end - $start" | bc -l )
          ###################################

          # Decompress with Linux Compress, track runtime.
          ##############################################
          start=`date +%s.%N`

          # Run program. Capture return value to check whether it crashes.
          compress -d < canterburyCompress$i.Z > canterburyCompress$i
          execution_val=$?
          if [[ $execution_val != 0 ]]; then
               echo "fail (program crashed)"
               continue
	     fi
          
          end=`date +%s.%N`
          compress_runtime+=$( echo "$end - $start" | bc -l )

          #Reset compressed file
          compress -f < canterburyCompress$i > canterburyCompress$i.Z
          ################################################
     done

     compress -d < canterburyCompress$i.Z > canterburyCompress$i

     # Calculate average runtimes. 
     echo ""
     echo "  ======================"
     echo "  Decompression Runtimes"
     echo "  ======================" 
     echo ""
     echo "  Average LZW runtime for decompressing canterbury$i.Z in seconds: "
     ./Divide.exe $lzw_runtime $NUM_TRIALS
     echo ""
     echo "  Average Linux Compress runtime for decompressing canterburyCompress$i.Z in seconds: "
     ./Divide.exe $compress_runtime $NUM_TRIALS
     echo ""

     # Comparative runtime
     echo ""
     echo "  Relative decompression runtime: "
     ./Divide.exe $lzw_runtime $NUM_TRIALS $compress_runtime $NUM_TRIALS
     relative_decomp_runtime=$?
     echo ""

     # Check for difference in source and uncompressed files.
     diff canterbury$i canterburyCompress$i
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
# Cleanup phase.
################################################################################
rm -f *.exe
rm -f myoutput.txt
rm -f *.Z

################################################################################
# Final thoughts.
################################################################################

echo ""
echo "================================================================"
echo "Final Report"
echo "================================================================"

if [ $pass_cnt -eq $NUM_CANTBRY_FILES ]; then
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
