0 REM        C H A S E       V 1.0        1/16/77   GCC III
1 REM BY HERMAN DEMONSTOY FEB 77 ISSUE KILOBAUD MAG
5 PRINT CHR$(26):PRINT:PRINT:PRINT:PRINT:PRINT
20 Z2=0
50 PRINT"This is the game of CHASE"
100 PRINT"You are in a high voltage maze with five"
110 PRINT"security machines (+) trying to destroy you (X)."
120 PRINT"You must maneuver the security machines into"
130 PRINT"the high voltage fence posts (O), or into each other"
140 PRINT"in order to survive.   GOOD LUCK.
145 PRINT
160 PRINT"MOVES ARE:    1 2 3"
165 PRINT"              8 * 4     0  IS NO MOVE"
170 PRINT"              7 6 5     9  GIVE UP"
175 PRINT
199 REM ----------------------------------------------> SET UP THE GAME
200 DIM A(10,20),E(21),F(21)
210 G=5:M=0
220 FOR B=1 TO 10
230 FOR C=1 TO 20
240 LET A(B,C)=0
250 IF B=1 THEN A(B,C)=1
260 IF B=10 THEN A(B,C)=1
270 IF C=1 THEN A(B,C)=1
280 IF C=20 THEN A(B,C)=1
290 NEXT C
295 PRINT" . ";:REM -------------> KEEPS SCREEN GOING WHILE TABLE SET UP
300 NEXT B
310 FOR D=1 TO 21
320 B=INT(RND(1)*8)+2
330 C=INT(RND(1)*18)+2:REM --------------------> .......................
340 IF A(B,C)<>0 GOTO 320
350 A(B,C)=1
360 IF D<6 THEN A(B,C)=2
370 IF D=6 THEN A(B,C)=3
380 E(D)=B
390 F(D)=C
400 NEXT D
410 PRINT:PRINT
499 REM ------------------------------------------------> PRINT PATTERN
500 FOR B=1 TO 10
510 FOR C=1 TO 20
520 IF A(B,C)=0 THEN  PRINT" ";
530 IF A(B,C)=1 THEN PRINT"O";
540 IF A(B,C)=2 THEN PRINT"+";
550 IF A(B,C)=3 THEN PRINT"X";
560 NEXT C
565 IF B=3 THEN PRINT"      ";G;"Security machine";:IF G>1 THEN PRINT"s";
566 IF B=3 THEN PRINT" left";
567 IF B=7 THEN PRINT"            1 2 3";
569 IF B=8 THEN PRINT"            8 * 4     0  IS NO MOVE";
571 IF B=9 THEN PRINT"            7 6 5     9  GIVE UP";
573 PRINT
580 NEXT B
599 REM -------------------------------------------------> MAKE MOVE
600 B=E(6)
610 C=F(6)
620 A(B,C)=0
630 INPUT"MOVE";Y
635 M=M+1
640 IF Y=0 GOTO 800
650 ON Y GOTO 660,660,660,690,680,680,680,690,1400
660 B=B-1
670 GOTO 690
680 B=B+1
690 ON Y GOTO 700,800,720,720,720,800,700,700
700 C=C-1
710 GOTO 800
720 C=C+1
799 REM ----------------------------------------> CALCULATE THE RESULTS
800 IF A(B,C)=1 GOTO 1500
810 IF A(B,C)=2 GOTO 1600
820 A(B,C)=3
830 E(6)=B
840 F(6)=C
850 FOR D=1 TO 5
860 IF A(E(D),F(D))<>2 GOTO 960
870 A(E(D),F(D))=0
880 IF E(D)<B THEN E(D)=E(D)+1
890 IF E(D)>B THEN E(D)=E(D)-1
900 IF F(D)<C THEN F(D)=F(D)+1
910 IF F(D)>C THEN F(D)=F(D)-1
920 IF A(E(D),F(D))=3 GOTO 1600
930 IF A(E(D),F(D))<>0 THEN G=G-1
940 IF A(E(D),F(D))=0 THEN A(E(D),F(D))=2
950 IF G=0 GOTO 1700
960 NEXT D
970 GOTO 500
1000 PRINT"Type   Y    or    N   please...":GOTO 980
1400 PRINT:PRINT"Sorry to see you quit"
1405 PRINT"after only";M;"move";:IF M>1 THEN PRINT"s";:PRINT"."
1410 PRINT:GOTO 1710
1500 PRINT:PRINT"You touched the fence !!!!"
1505 PRINT" After only";M;"moves."
1510 GOTO 1710
1600 PRINT:PRINT"** You have been destroyed by a lucky computer **"
1605 PRINT"after only";M;"moves.   Too bad."
1610 GOTO 1710
1700 PRINT:PRINT"You are lucky * * You destroyed all the enemy * *"
1705 PRINT"in only";M;"moves."
1707 Z2=Z2+1
1710 PRINT"So far you have won";Z2;"game(s)!!!"
1715 PRINT:PRINT:PRINT"Do you want to play again (Y OR N)"
1720 INPUT Y$
1725 PRINT:PRINT
1730 IF LEFT$(Y$,1)="Y" GOTO 210
1735 IF LEFT$(Y$,1)="y" GOTO 210
1740 PRINT:PRINT"Hope you don't feel fenced in."
1750 PRINT"Try again sometime."
1760 PRINT"This time you won";Z2;"game(s)."
1765 Z2=Z2+1
1770 PRINT"Next time you may win";Z2;", or more!!!!!"
2000 SYSTEM
1760 PRINT"This time you won";Z2;"game(s)."
1765 Z2=Z2+1
1770 PRINT"Next time you may