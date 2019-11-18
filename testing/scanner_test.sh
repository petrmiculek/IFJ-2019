#! /bin/bash

proj=../test


inputs_scanner=./tests/scanner/input/*
i=0 # counter

echo "----------SCANNER TESTS ----------"
# testy na  chyby ve scanneru
for f in $inputs_scanner
do
    
    cat $f >input
    test_output=$(./$proj) # test output
    
    proper_output=$(cat ./tests/scanner/output/out$((i+1)))
    
	i=$(($i + 1)) # test number
    
    printf "\nTEST $i:"
    if [ "$test_output" == "$proper_output" ]; then
        printf "OK"
    else
        printf "ERROR\n"
        diff <( echo "$proper_output" ) <( echo "$test_output" )
        #cat $f
   fi 
done



