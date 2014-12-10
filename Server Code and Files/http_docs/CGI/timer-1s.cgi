#!/bin/bash
echo "Content-Type:text/html"
echo ""
#conteudo a partir daqui
init=`date +%s%N | cut -b1-13`
a=`date +%s%N | cut -b1-13`
echo $(($a-$init))
echo '<br>'
for i in {1..10}
do
   sleep 0.1
   a=`date +%s%N | cut -b1-13`
   echo $(($a-$init))
   echo '<br>'
done


