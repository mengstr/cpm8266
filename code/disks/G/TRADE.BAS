5 REM                   STAR TRADERS
10 REM - MODIFIED FOR 'ALTAIR BASIC 4.0' BY - S J SINGER
20 REM
30 REM
35 DEFINT C
36 DEFINT I
40 DIM M(10,13),S(5,4),N$(5),D1(5),S1(5),Q(5),M$(12),C$(25)
50 DIM C1$(25),C2$(25)
60 DATA 1,"`Altair Starways'"
70 DATA 2,"`Betelgeuse,LTD.'"
80 DATA 3,"`Capella Freight Co.'"
90 DATA 4,"`Denebola Shippers'"
100 DATA 5,"`Eridani Expediters'"
110 Z1$=CHR$(26)+CHR$(0)
120 PRINT Z1$
130   FOR I=1 TO 5
140    FOR J=1 TO 4
150 S(I,J)=0: D1(I)=0: S1(I)=100: Q(I)=0: B(I)=6000
160    NEXT J
170   NEXT I
180 L$=".+*ABCDE"
190 M$="ABCDEFGHIJKL"
195 PRINT"              **********   STAR TRADERS   **********":PRINT
197 PRINT:PRINT
200 INPUT "Type a 3 digit number  ";R1
220 R1=RND(-R1/1000)
230   FOR I=1 TO 9
240    FOR J=1 TO 12
250 IF INT(20*RND(R1)+1)<>10 THEN M(I,J)=1 ELSE M(I,J)=3
260   NEXT J
270  NEXT I
280 PRINT Z1$
290 INPUT "How many players  (1-4)  ";P1
300 PRINT:PRINT
310 INPUT "Does any player need instructions  (Type Y or N)";ANS$
320 IF LEFT$(ANS$,1)="Y" THEN GOSUB 3410
321 IF LEFT$(ANS$,1)="y" THEN GOSUB 3410
330 PRINT Z1$
340   FOR I=1 TO P1
350 PRINT "Player",I,
370 INPUT " what is your name  ";P$
380 IF I=1 THEN P1$=P$
390 IF I=2 THEN P2$=P$
400 IF I=3 THEN P3$=P$
410 IF I=4 THEN P4$=P$
420   NEXT I
430 PRINT Z1$
440 PRINT TAB(10),"...Now I will decide who goes first...":: PRINT:PRINT
445 PRINT:PRINT
450 PRINT:PRINT:PRINT
460 I=INT(P1*RND(R1)+1)
470 GOSUB 490
480 GOTO 550
490 PRINT
500 ON I GOTO 510,520,530,540: REM - IDENTIFY PLAYER
510 PRINT P1$;:P5$=P1$: RETURN
520 PRINT P2$;:P5$=P2$: RETURN
530 PRINT P3$;:P5$=P3$: RETURN
540 PRINT P4$;:P5$=P4$: RETURN
550 PRINT " is the first player to move."
560 FOR W=1 TO 2000: NEXT
570 K=0
580 P=I:GOTO 610
590 K=K+1: IF K=48 THEN 4450
600 P=P+1: IF P=P1+1 THEN P=1
610  FOR I=1 TO 5: REM ----------------> SELECT 5 LEGAL MOVES
620 R(I)=INT(9*RND(R1)+1)
630 C(I)=INT(12*RND(R1)+1)
640    FOR I1=I-1 TO 0 STEP -1
650 IF R(I)=R(I1) AND C(I)=C(I1) THEN  620
660    NEXT I1
670 IF M(R(I),C(I))>1 THEN 620
680    FOR I1=1 TO 5
690 IF Q(I1)=0 THEN  870
700    NEXT I1
710 IF M(R(I),C(I)+1)>3 THEN 870
720 IF M(R(I),C(I)-1)>3 THEN 870
730 IF M(R(I)+1,C(I))>3 THEN 870
740 IF M(R(I)-1,C(I))>3 THEN 870
750 A1=M(R(I),C(I)+1)
760 A2=M(R(I),C(I)-1)
770 A3=M(R(I)+1,C(I))
780 A4=M(R(I)-1,C(I))
790 IF A1=2 AND A2<4 AND A3<4 AND A4<4 THEN  620
800 IF A2=2 AND A1<4 AND A3<4 AND A4<4 THEN  620
810 IF A3=2 AND A1<4 AND A2<4 AND A4<4 THEN  620
820 IF A4=2 AND A1<4 AND A2<4 AND A3<4 THEN  620
830 IF A1=3 AND A2<4 AND A3<4 AND A4<4 THEN  620
840 IF A2=3 AND A1<4 AND A3<4 AND A4<4 THEN  620
850 IF A3=3 AND A1<4 AND A2<4 AND A4<4 THEN  620
860 IF A4=3 AND A1<4 AND A2<4 AND A3<4 THEN  620
870   NEXT I
880 GOSUB 2060
890 PRINT
900 I=P
910 GOSUB 490
920 PRINT ", Here are your legal moves for this turn"
930 PRINT
940   FOR I=1 TO 5
950 PRINT R(I);MID$(M$,C(I),1);" ";
960   NEXT I
970 PRINT:PRINT
980 INPUT "What is your move ";R$
990 IF LEN(R$)=0 THEN R$="S"
1000 IF LEFT$(R$,1)="M" THEN R$="" ELSE 1030
1010 GOSUB 2060
1020 GOTO 900
1030 IF LEFT$(R$,1)="S" THEN R$="" ELSE 1060
1040 GOSUB 3230
1050 GOTO 900
1060 IF LEN(R$)<>2 THEN 1110
1070 IF ASC(MID$(R$,2,1))-64<1 THEN 1110
1080 IF ASC(MID$(R$,2,1))-64>12 THEN 1110 ELSE 1120
1090 IF VAL(R$)<1 THEN 1110
1100 IF VAL(R$)>9 THEN 1110
1110 PRINT "I didn't understand that - Try again ": GOTO 980
1120 R=VAL(LEFT$(R$,1))
1130 C=ASC(RIGHT$(R$,1))-64
1140   FOR I= 1 TO 5
1150 IF R=R(I) AND C=C(I) THEN  1190
1160   NEXT I
1170 PRINT "That space was not included in the list..."
1180 GOTO 980
1190 A1=M(R-1,C)
1200 A2=M(R+1,C)
1210 A3=M(R,C+1)
1220 A4=M(R,C-1)
1230 IF A1<=1 AND A2<=1 AND A3<=1 AND A4<=1 THEN M(R,C)=2 ELSE 1250
1240 GOTO 1760
1250 IF A1>3 AND A2>3 AND A2<>A1 THEN GOSUB 2220:REM - LINE 2090 IS
1260 IF A1>3 AND A3>3 AND A3<>A1 THEN GOSUB 2220\REM   THE MERGER SUB.
1270 IF A1>3 AND A4>3 AND A4<>A1 THEN GOSUB 2220
1280 IF A2>3 AND A3>3 AND A3<>A2 THEN GOSUB 2220
1290 IF A2>3 AND A4>3 AND A4<>A2 THEN GOSUB 2220
1300 IF A3>3 AND A4>3 AND A4<>A3 THEN GOSUB 2220
1310 IF A1<4 AND A2<4 AND A3<4 AND A4<4 THEN 1410
1320 IF M(R,C)>3 THEN 1760
1330 IF A1>3 THEN I=A1-3
1340 IF A2>3 THEN I=A2-3
1350 IF A3>3 THEN I=A3-3
1360 IF A4>3 THEN I=A4-3
1370 Q(I)=Q(I)+1
1380 S1(I)=S1(I)+100
1390 M(R,C)=I+3
1400 GOTO 1570
1410   FOR I=1 TO 5
1420 IF Q(I)=0 THEN 1460
1430   NEXT I
1440 IF M(R,C)<3 THEN M(R,C)=2
1450 GOTO 1760
1460 PRINT Z1$
1470 GOSUB 3370
1480 PRINT "A new shipping company has been formed !"
1490 PRINT "It's name is ",
1500 RESTORE
1510 READ N,C$
1520 IF I<>N THEN 1510
1530 PRINT C$
1540 S(I,P)=S(I,P)+5
1550 Q(I)=1
1560 PRINT:PRINT:PRINT:PRINT
1570 IF A1=3 THEN S1(I)=S1(I)+500
1580 IF A2=3 THEN S1(I)=S1(I)+500
1590 IF A3=3 THEN S1(I)=S1(I)+500
1600 IF A4=3 THEN S1(I)=S1(I)+500
1610 IF A1=2 THEN S1(I)=S1(I)+100 ELSE 1640
1620 Q(I)=Q(I)+1
1630 M(R-1,C)=I+3
1640 IF A2=2 THEN S1(I)=S1(I)+100 ELSE 1670
1650 Q(I)=Q(I)+1
1660 M(R+1,C)=I+3
1670 IF A3=2 THEN S1(I)=S1(I)+100 ELSE 1700
1680 Q(I)=Q(I)+1
1690 M(R,C+1)=I+3
1700 IF A4=2 THEN S1(I)=S1(I)+100 ELSE 1730
1710 Q(I)=Q(I)+1
1720 M(R,C-1)=I+3
1730 IF S1(I)>=3000 THEN T1=I ELSE 1750
1740 GOSUB 3100
1750 M(R,C)=I+3
1760  FOR I=1 TO 5
1770 B(P)=B(P)+INT(.05*S(I,P)*S1(I))
1780  NEXT I
1790  FOR I=1 TO 5
1800 IF Q(I)=0 THEN 2040
1810 PRINT:PRINT   "Your current cash= $";B(P)
1820 PRINT:PRINT   "Buy how many shares of ";
1830 RESTORE
1840 READ N,C$
1850 IF I<>N THEN 1840
1860 PRINT C$;
1870 PRINT " at $";S1(I)
1880 PRINT TAB(5); "You now own ";S(I,P);
1890 INPUT R3$:IF LEN(R3$)=0 THEN R3$="0"
1900 IF R3$(1,1)="M" THEN R3$="" ELSE 1930
1910 GOSUB 2060
1920 GOTO 1810
1930 IF R3$(1,1)="S" THEN R3$="" ELSE 1960
1940 GOSUB 3230
1950 GOTO 1810
1960 R3=VAL(R3$)
1970 R3$=""
1980 IF R3*S1(I)<=B(P) THEN 2010
1990 PRINT "You only have $";B(P);" - Try again"
2000 GOTO 1810
2010 IF R3=0 THEN 2040
2020 S(I,P)=S(I,P)+R3
2030 B(P)=B(P)-(R3*S1(I))
2040   NEXT I
2050 GOTO 590
2060 PRINT Z1$: REM             SUBROUTINE - PRINT MAP
2070 PRINT TAB(22);"MAP OF THE GALAXY"
2080 PRINT TAB(21);"*******************"
2090 PRINT TAB(13);" A  B  C  D  E  F  G  H  I  J  K  L"
2100   FOR R2=1 TO 9
2110 PRINT"         ";R2;" ";
2120 FOR C2=1 TO 12
2130 PRINT" ";
2140 Z2=M(R2,C2)
2150 IF Z2=0 THEN Z2=Z2+1
2160 PRINT MID$(L$,Z2,1)" ";
2180 NEXT
2190 PRINT
2200 NEXT
2210 RETURN
2220 F1=A1-3: IF F1<0 THEN F1=0:REM  SUBROUTINE - CALCULATES THE
2230 F2=A2-3: IF F2<0 THEN F2=0:REM     SURVIVOR IN THE EVENT
2240 F3=A3-3: IF F3<0 THEN F3=0:REM        OF A MERGER
2250 F4=A4-3: IF F4<0 THEN F4=0
2260 T=Q(F1)
2270 T1=F1
2280 IF Q(F2)>Q(F1) THEN T=Q(F2) ELSE 2300
2290 T1=F2
2300 IF Q(F3)>T THEN T=Q(F3) ELSE 2320
2310 T1=F3
2320 IF Q(F4)>T THEN T=Q(F4) ELSE 2340
2330 T1=F4
2340 IF F1=T1 OR A1<4 THEN 2370
2350 X=F1
2360 GOSUB 2470
2370 IF F2=T1 OR A2<4 THEN 2400
2380 X=F2
2390 GOSUB 2470
2400 IF F3=T1 OR A3<4 THEN 2430
2410 X=F3
2420 GOSUB 2470
2430 IF F4=T1 OR A4<4 THEN 2460
2440 X=F4
2450 GOSUB 2470
2460 RETURN
2470 PRINT Z1$
2480 GOSUB 3370: REM     SUBROUTINE - PERFORMS CALCULATIONS
2490 RESTORE: REM         TO ACCOMPLISH A MERGER
2500 READ N,C$
2510 IF X<>N THEN 2500
2520 C1$=C$
2530 PRINT C1$;
2540 PRINT " has just been merged into ";
2550 RESTORE
2560 READ N,C$
2570 IF T1<>N THEN 2560
2580 C2$=C$
2590 PRINT C2$;"!"
2610 PRINT "Please note the following transactions."
2620 PRINT
2630 PRINT TAB(3);"OLD STOCK = ";C1$;"      NEW STOCK = ";
2640 PRINT C2$
2650 PRINT
2660 PRINT "PLAYER";TAB(10);"OLD STOCK";TAB(22);"NEW STOCK";
2670 PRINT TAB(34);"TOTAL HOLDINGS";TAB(53);"BONUS PAID"
2680   FOR I=1 TO P1
2690 GOSUB 490
2700 PRINT TAB(10);S(X,I);TAB(22);INT((.5*S(X,I))+.5);
2710 PRINT TAB(34);S(T1,I)+INT((.5*S(X,I))+.5);
2720 X1=0
2730    FOR I1=1 TO P1
2740 X1=X1+S(X,I1)
2750    NEXT 
2760 PRINT TAB(53);" $";INT(10*((S(X,I)/X1)*S1(X)))
2770   NEXT I
2780   FOR I=1 TO P1
2790 S(T1,I)=S(T1,I)+INT((.5*S(X,I))+.5)
2800 B(I)=B(I)+INT(10*((S(X,I)/X1)*S1(X)))
2810   NEXT I
2820   FOR I=1 TO 9
2830    FOR J=1 TO 12
2840 IF M(I,J)=X+3 THEN M(I,J)=T1+3
2850    NEXT J
2860   NEXT I
2870 A1=M(R-1,C)
2880 A2=M(R+1,C)
2890 A3=M(R,C+1)
2900 A4=M(R,C-1)
2910 F1=A3-3
2920 IF F1<0 THEN F1=0
2930 F2=A2-3
2940 IF F2<0 THEN F2=0
2950 Q(T1)=Q(T1)+Q(X)
2960 S1(T1)=S1(T1)+S1(X)
2970 IF S1(T1)=>3000 THEN GOSUB 3100
2980 F3=A3-3
2990 IF F3<0 THEN F3=0
3000 F4=A4-3
3010 IF F4<0 THEN F4=0
3020 S1(X)=100
3030 Q(X)=0
3040   FOR I=1 TO P1
3050 S(X,I)=0
3060   NEXT I
3070 PRINT:PRINT
3080 M(R,C)=T1+3
3090 RETURN
3100 GOSUB 3370: REM        SUBROUTINE - CALCULATES STOCK SPLITS
3110 PRINT "The stock of ",
3120 RESTORE
3130 READ N,C$
3140 IF T1<>N THEN 3130
3150 PRINT C$,
3160 PRINT " has split 2 for 1 !"
3170 S1(T1)=INT(S1(T1)/2)
3180 PRINT:PRINT
3190   FOR I1=1 TO P1
3200 S(T1,I1)=2*S(T1,I1)
3210   NEXT I1
3220 RETURN
3230 PRINT Z1$
3240 PRINT
3250 PRINT "STOCK";TAB(30);"PRICE PER SHARE";
3260 PRINT TAB(50);"YOUR HOLDINGS"
3270   FOR I3=1 TO 5
3280 IF S1(I3)=100 THEN 3340
3290 RESTORE
3300 READ N,C$
3310 IF I3<>N THEN 3300
3320 PRINT C$,
3330 PRINT TAB(30);S1(I3);TAB(50);S(I3,P)
3340   NEXT I3
3350 RESTORE
3360 RETURN
3370 PRINT CHR$(7)
3380 PRINT TAB(22);"SPECIAL ANNOUNCEMENT !!!":PRINT
3390 PRINT
3400 RETURN
3410 PRINT Z1$
3420 PRINT "   Star lanes is a game of interstellar trading."
3430 PRINT "The object of the  game is to amass the  greatest  amount"
3440 PRINT "of money.  This  is  accomplished  by  establishing vast,"
3450 PRINT "interstellar  shipping  lanes,  and  purchasing stock  in"
3460 PRINT "the  companies that  control those  trade routes.  During"
3470 PRINT "the  course of  the game,  stock appreciates  in value as"
3480 PRINT "the shipping  companies  become  larger.   Also,  smaller"
3490 PRINT "companies  can  be  merged  into  larger ones,  and stock"
3500 PRINT "in the smaller firm is converted into stock in the larger"
3510 PRINT "one as described below.":PRINT
3520 PRINT "Each turn,  the  computer will present  the  player  with"
3530 PRINT "five  prospective  spaces  to occupy on a  9 X 12  matrix"
3540 PRINT "(rows 1-9,  columns A-L). The player, after examining the"
3550 PRINT "map of the galaxy to decide which space  he or she wishes"
3560 PRINT "to  occupy,  responds  with the  row and column  of  that"
3570 PRINT "space (I.E., 7E,  8A, ETC). There are five possible moves"
3580 PRINT "a player can make.":PRINT:PRINT
3590 PRINT
3600 GOSUB 3620
3610 GOTO 3660
3620 INPUT "PRESS <RETURN> TO CONTINUE";X2$:PRINT CHR$(26)
3650 RETURN
3660 PRINT Z1$
3670 PRINT "   1. A player  can  establish an  unattached outpost - if"
3680 PRINT "he/she  selects a  space  that is not  adjacent to a star,"
3690 PRINT "another unattached outpost,  or an existing shipping lane,"
3700 PRINT "that space will be  designated  with  a `+'.  He/she  will"
3710 PRINT "then proceed with stock transactions, as listed below.":PRINT
3720 PRINT "   2. A player  can add to an  existing lane  -  if he/she"
3730 PRINT "selects  space that  is  adjacent to  one - and only one -"
3740 PRINT "existing shipping lane,  the space  he/she selects will be"
3750 PRINT "that shipping lane  and will be designated with  the first"
3760 PRINT "letter of the  company that owns that lane.  if there  are"
3770 PRINT "any  stars  or  unattached  outposts also adjacent to  the"
3780 PRINT "selected space,  they, too,  will be incorporated into the"
3790 PRINT "existing  lane.  Each new square  adjacent to a star  adds"
3800 PRINT "$500 per share,  and each new outpost  adds $100 per share"
3810 PRINT "to the market value of the stock of that company."
3820 PRINT:PRINT
3830 GOSUB 3620
3840 PRINT Z1$
3850 PRINT "   3. He/she may establish a new shipping lane - if there"
3860 PRINT "are five or less existing shipping lanes established."
3870 PRINT "The player may, given the proper space to play, establish"
3880 PRINT "a new shipping lane.  He/she  may do this by  occupying a"
3890 PRINT "space adjacent to a  star or  another unattached outpost,"
3900 PRINT "but not adjacent to an existing shipping lane.  If he/she"
3910 PRINT "establishes a new shipping lane, he/she is  automatically"
3920 PRINT "issued 5 shares of the  new company  as a reward.  He/she"
3930 PRINT "may then proceed  to buy stock  in  any  active  company,"
3940 PRINT "including the one just formed, as described below.":PRINT
3950 PRINT "The market value of the  new stock is established  by the"
3960 PRINT "number of stars  and occupied spaces  as described  in #2"
3970 PRINT "above."
3980 PRINT:PRINT
3990 GOSUB 3620
4000 PRINT Z1$
4010 PRINT "   4.  He/she may merge two existing companies - if a player"
4020 PRINT "selects a space  adjacent to two existing shipping lanes,  a"
4030 PRINT "merger occurs.  The larger company takes  over  the  smaller"
4040 PRINT "company.  (If both companies  are the same size prior to the"
4050 PRINT "merger, then the survivor is determined by alphabetical order"
4060 PRINT "of the two company names - the earlier survives.)"
4070 PRINT "The stock of the  surviving company  is increased  in  value"
4080 PRINT "according to  the number of  spaces and stars  added to  its"
4090 PRINT "lane. Each players stock in the defunct company is exchanged"
4100 PRINT "for shares in the survivor on a ratio of 2 for 1. Also, each"
4120 PRINT "player is paid a cash bonus proportional  to the  percentage"
4130 PRINT "of outstanding stock he held in the defunct company."
4140 PRINT "NOTE:  After a company becomes defunct  through  the  merger"
4150 PRINT "process, it can reappear elsewhere on the board when, and if"
4160 PRINT "a new company is established."
4170 PRINT:PRINT
4180 GOSUB 3620
4190 PRINT Z1$
4200 PRINT "   Next the computer adds stock dividends to the player's"
4210 PRINT "cash on hand  (5% of the market value of the stock in his"
4220 PRINT "possession),  and offers him the opportunity to  purchase"
4230 PRINT "stock in any of the active companies on the board."
4240 PRINT "Stock may not be sold,  but  the  market  value  of  each"
4250 PRINT "player's stock  is taken into account  at the end  of the"
4260 PRINT "game  to determine  the winner.  If the market value of a"
4270 PRINT "given stock  exceeds  $3000  at any time during the game,"
4280 PRINT "that stock splits 2 for 1.  The price is cut in half, and"
4290 PRINT "the number of shares owned by each player is doubled."
4300 PRINT
4310 PRINT "NOTE:   The player  may look at his/her  portfolio at any"
4320 PRINT "time during the course of his/her turn by responding with"
4330 PRINT "`STOCK' to an input statement. Likewise, he/she can review"
4340 PRINT "the map of the galaxy by typing `MAP' to an input statement."
4350 PRINT:PRINT
4360 GOSUB 3620
4370 PRINT Z1$
4380 PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT
4390 PRINT TAB(16);"** GAME ENDS AFTER 48 MOVES **"
4400 PRINT:PRINT:PRINT:PRINT
4410 PRINT "The player with the greatest net worth at that point is the winner."
4420 PRINT:PRINT
4430 FOR W=1 TO 2000:NEXT W
4440 RETURN
4450 PRINT Z1$
4460 GOSUB 3370
4470 FOR W=1 TO  500:NEXT W:PRINT CHR$(7)
4480 FOR W=1 TO 500:NEXT W:PRINT CHR$(7)
4490 PRINT TAB(10)," THE GAME IS OVER - HERE ARE THE FINAL STANDINGS"
4500 PRINT:PRINT:PRINT:PRINT
4510 PRINT CHR$(7)
4520 PRINT "PLAYER";TAB(10);"CASH VALUE OF STOCK";TAB(33);"CASH ON HAND",
4530 PRINT TAB(50);"NET WORTH"
4540 PRINT
4550   FOR I=1 TO P1
4560    FOR J=1 TO 5
4570 D1(I)=D1(I)+(S1(J)*S(J,I))
4580    NEXT J
4590   NEXT I
4600   FOR I=1 TO P1
4610 GOSUB 490
4620 PRINT TAB(10);"$";D1(I);TAB(33);"$";B(I);
4630 PRINT TAB(50);"$";D1(I)+B(I)
4640   NEXT I
4650 PRINT:PRINT:PRINT:PRINT
4655 INPUT "Do you want to play again (type Y or N)";ANS$
4660 IF LEFT$(ANS$,1)="N" THEN SYSTEM
4670 IF LEFT$(ANS$,1)="n" THEN SYSTEM
4680 RESET
4685 PRINT CHR$(26)
4690 GOTO 195

4660 IF LEFT$(ANS$,1)="N" THEN SYSTEM
4670 IF LEFT$(ANS$,1)="n" THEN SYSTEM
4680 RESET
4685 PRINT CHR$(26)
4690 GOT