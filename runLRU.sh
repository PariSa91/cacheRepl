#!/bin/csh
if ( $1 == "" ) then
    printf "Usage: $0 <traces-directory>\n"
    exit 1
endif
#rm -f speedups.out
setenv DAN_POLICY 1


foreach assoc (64 128)
   rm -f $assoc.ipc
   setenv DAN_ASSOC $assoc
   printf "assoc is $assoc\n"
   ./efectiu $1/*.trace.gz |& tee assoc$assoc.out  
 #   foreach benchmark ( $1/*.trace.gz )

#        ./efectiu $benchmark |& tee assoc$assoc.out
        
	set ipc = `tail assoc$assoc.out | grep "core.*IPC" | sed -e '/^.*: /s///' | sed -e '/ IPC/s///'`
        echo $ipc > assoc$assoc.ipc
#    end
#    printf "scale=10 ; %s / %s\n" `cat 4.ipc` `cat 2.ipc` | bc >> speedups.out
end
#set N = `wc -l < speedups.out`
#set i = 1
#printf "scale = 10\na = " > bc.in
#foreach x ( `cat speedups.out` )
#    printf "$x " >> bc.in
#    if ( $i != $N ) printf "* " >> bc.in
#    @ i = $i + 1
#end
#printf "\nb = 1 / $N\ne(b * l(a))\n" >> bc.in
#printf "Geometric mean speedup is "
#bc -l < bc.in
