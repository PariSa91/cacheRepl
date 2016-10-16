#!/bin/csh
if ( $1 == "" ) then
    printf "Usage: $0 <traces-directory>\n"
    exit 1
endif
setenv DAN_POLICY 1


foreach assoc (2 4 8 16 32 64 )
   rm -f $assoc.ipc
   rm -f bc.in
   setenv DAN_ASSOC $assoc
   printf "assoc is $assoc\n"
   ./efectiu $1/*.trace.gz |& tee assoc$assoc.out  
	set ipc = `tail assoc$assoc.out | grep "core.*IPC" | sed -e 's/^.*: //' |    sed -e 's/ IPC/ /'`
        echo  $ipc > assoc$assoc.ipc

set N = `wc -w < assoc$assoc.ipc`
set i = 1
printf "scale = 10\na = " > bc.in
foreach x ( `cat assoc$assoc.ipc` )
    printf "$x " >> bc.in
    if ( $i != $N ) printf "+ " >> bc.in
    @ i = $i + 1
end
printf "\nb = 1 / $N\ne(b * l(a))\n" >> bc.in
printf " mean IPCs is "
bc -l < bc.in
set mean = ` bc -l < bc.in `
printf  "$assoc\t$mean\n" >> outs.txt


