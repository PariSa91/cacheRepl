#!/bin/csh
if ( $1 == "" ) then
    printf "Usage: $0 <traces-directory>\n"
    exit 1
endif
setenv DAN_POLICY 1
rm -f speedups.out
rm -f outs.txt
rm *.out


 setenv DAN_ASSOC 16
  ./efectiu $1/*.trace.gz |& tee assoc16.out  
  set ipc = `tail efectiu16.out | grep "core.*IPC" | sed -e '/^.*: /s///' | sed -e '/ IPC/s///'`
  echo $ipc > 16.ipc

foreach assoc (2  4   )
   rm -f bc.in
   setenv DAN_ASSOC $assoc
   printf "assoc is $assoc\n"
  
foreach benchmark ( $1/*.trace.gz )
#        rm -f 16.ipc
 #      rm -f $assoc.ipc
 #       rm -f efectiu.out
 #       rm -f efectiuAssoc.out
        ./efectiu $benchmark |& tee efectiuAssoc.out
        set ipc = `tail efectiuAssoc.out | grep "core.*IPC" | sed -e '/^.*: /s///' | sed -e '/ IPC/s///'`
        echo $ipc > $assoc.ipc
        printf "ipc is $ipc\n" 
#        rm -f efectiuAssoc.out
        
    #    setenv DAN_ASSOC 16
    #   ./efectiu $benchmark |& tee efectiu16.out
        set ipc = `tail efectiu16.out | grep "core.*IPC" | sed -e '/^.*: /s///' | sed -e '/ IPC/s///'`
    #    echo $ipc > 16.ipc
   #    printf "scale=10 ; %s / %s\n" `cat $assoc.ipc` `cat 16.ipc` | bc >> assoc$assoc.out
      
  end

set N = `wc -w < assoc$assoc.out`
set i = 1
printf "scale = 10\na = " > bc.in
foreach x ( `cat assoc$assoc.ipc` )
    printf "$x " >> bc.in
    if ( $i != $N ) printf "* " >> bc.in
    @ i = $i + 1
end
printf "\nb = 1 / $N\ne(b * l(a))\n" >> bc.in
printf " mean IPCs is "
bc -l < bc.in
set mean = ` bc -l < bc.in `
printf  "$assoc\t$mean\n" >> outs.txt

end
