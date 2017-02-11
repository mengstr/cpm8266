if [ "$LINES" == "" ]; then
	echo "Usage: source $0"
	exit
fi
alias m="make clean && make && make flash && make serial"
alias note="echo $* >> ~/cpm2866/notes.txt"
echo ok, alias m is now setup