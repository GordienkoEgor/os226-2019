#!/bin/bash

most_common_value=0
most_common_value_index=0	

while read value
do
	let freq_arr[value]++
	if (( $most_common_value_index < ${freq_arr[value]} )) 
	then
		most_common_value_index=${freq_arr[value]}
		most_common_value=$value
	fi
done

echo $most_common_value
