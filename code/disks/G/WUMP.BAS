1 PRINT CHR$(26):PRINT:PRINT:PRINT:PRINT:PRINT
5 REM                                          *** HUNT THE WUMPUS ***
10 DIM P(5)
15 INPUT "Do you want instructions (type Y or N)";ANS$
17 IF LEFT$(ANS$,1)="N" THEN 35
18 IF LEFT$(ANS$,1)="n" THEN 35
22 PRINT:PRINT:PRINT
30 GOSUB 375
35 GOTO 80
80 REM                    *** SET UP CAVE (DODECAHEDRAL NODE LIST) ***
85 DIM S(20,3)
90 FOR J=1 TO 20
95 FOR K=1 TO 3
100 READ S(J,K)
105 NEXT K
110 NEXT J
115 DATA 2,5,8,1,3,10,2,4,12,3,5,14,1,4,6
120 DATA 5,7,15,6,8,17,1,7,9,8,10,18,2,9,11
125 DATA 10,12,19,3,11,13,12,14,20,4,13,15,6,14,16
130 DATA 15,17,20,7,16,18,9,17,19,11,18,20,13,16,19
135 DEF FNA(X)=INT(20*RND(1))+1
140 DEF FNB(X)=INT(3*RND(1))+1
145 DEF FNC(X)=INT(4*RND(1))+1
150 REM                                   *** LOCATE L ARRAY ITEMS ***
155 REM                    *** 1-YOU, 2-WUMPUS, 3&4-PITS, 5&6-BATS ***
160 DIM L(6)
165 DIM M(6)
170 FOR J=1 TO 6
175 L(J)=FNA(0)
180 M(J)=L(J)
185 NEXT J
190 REM               *** CHECK FOR CROSSOVERS (IE L(1)=L(2), ETC) ***
195 FOR J=1 TO 6
200 FOR K=1 TO 6
205 IF J=K THEN 215
210 IF L(J)=L(K) THEN 170
215 NEXT K
220 NEXT J
225 REM                                      *** SET NO. OF ARROWS ***
230 A=5
235 L=L(1)
240 REM                                           *** RUN THE GAME ***
245 PRINT:PRINT:PRINT "HUNT THE WUMPUS"
250 REM                            *** HAZARD WARNING AND LOCATION ***
255 GOSUB 585
260 REM                                          *** MOVE OR SHOOT ***
265 GOSUB 670
270 ON O GOTO 280,300
275 REM                                                  *** SHOOT ***
280 GOSUB 715
285 IF F=0 THEN 255
290 GOTO 310
295 REM                                                   *** MOVE ***
300 GOSUB 975
305 IF F=0 THEN 255
310 IF F>0 THEN 335
315 REM                                                   *** LOSE ***
320 PRINT:PRINT "HA HA HA - You lose!"
325 GOTO 340
330 REM                                                   *** MOVE ***
335 PRINT:PRINT "HEE HEE HEE - The wumpus will get you next time!!"
340 FOR J=1 TO 6
345 L(J)=M(J)
350 NEXT J
355 PRINT:PRINT:INPUT "Do you want to play again (type Y or N)";A$
356 IF LEFT$(A$,1)="N" THEN 1150
357 IF LEFT$(A$,1)="n" THEN 1150
360 PRINT:INPUT "Do you want the same setup (type Y or N)";AN$
361 IF LEFT$(AN$,1)="N" THEN 170
362 IF LEFT$(AN$,1)="n" THEN 170
370 GOTO 230
375 REM                                           *** INSTRUCTIONS ***
377 PRINT CHR$(26):PRINT:PRINT
380 PRINT "Welcome to `HUNT THE WUMPUS'"
385 PRINT "  The wumpus lives in a cave of 20 rooms. Each room"
390 PRINT "has 3 tunnels leading to other rooms. (Look at a"
395 PRINT "dodecahedron to see how this works - if you don't know"
400 PRINT "what a dodecahedron is, ask someone)"
405 PRINT
410 PRINT "     HAZARDS:"
415 PRINT " BOTTOMLESS PITS - Two rooms have bottomless pits in them."
420 PRINT "     If you go there, you fall into the pit (and lose!)"
425 PRINT " SUPER BATS - Two other rooms have super bats. If you"
430 PRINT "     go there, a bat grabs you and takes you to some other"
435 PRINT "     room at random. (Which may be troublesome.)"
440 PRINT:INPUT "TYPE <RETURN> TO CONTINUE ";W9:PRINT CHR$(26)
441 PRINT:PRINT
445 PRINT "     WUMPUS:"
450 PRINT " The wumpus is not bothered by hazards (he has sucker"
455 PRINT " feet and is too big for a bat to lift).  Usually"
460 PRINT " he is asleep.  Two things wake him up - You shooting an"
465 PRINT " arrow or you entering his room."
470 PRINT "     If the wumpus wakes he moves (75% chance) one room"
475 PRINT " or stays still (25% chance).  After that, if he is where you"
480 PRINT " are, he eats you up and you lose!"
485 PRINT
490 PRINT "     YOU:"
495 PRINT " Each turn you may move or shoot a crooked arrow"
500 PRINT "   MOVING:  You can move one room (thru one tunnel)"
505 PRINT "   ARROWS:  You have 5 arrows.  You lose when you run out"
510 PRINT "   Each arrow can go from 1 to 5 rooms. You aim by telling"
515 PRINT "   the computer the room numbers you want the arrow to go to."
520 PRINT "   If the arrow can't go that way (if no tunnel), it moves"
525 PRINT "   at random to the next room."
530 PRINT "     If the arrow hits the wumpus, you win."
535 PRINT "     If the arrow hits you, you lose."
540 PRINT:INPUT "TYPE <RETURN> TO CONTINUE ";W9:PRINT CHR$(26)
545 PRINT "    WARNINGS:"
550 PRINT "     When you are one room away from a wumpus or hazard,"
555 PRINT "     the computer says:"
560 PRINT " WUMPUS:  `I smell a wumpus!'"
565 PRINT " BAT   :  `Bats nearby!'"
570 PRINT " PIT   :  `I feel a draft!'"
575 PRINT
580 RETURN
585 REM                        *** PRINT LOCATION & HAZARD WARNINGS ***
590 PRINT
595 FOR J=2 TO 6
600 FOR K=1 TO 3
605 IF S(L(1),K)<>L(J) THEN 640
610 ON J-1 GOTO 615,625,625,635,635
615 PRINT "I smell a wumpus!"
620 GOTO 640
625 PRINT "I feel a draft!"
630 GOTO 640
635 PRINT "Bats nearby!"
640 NEXT K
645 NEXT J
650 PRINT "You are in room "L(1)
655 PRINT "Tunnels lead to "S(L,1);S(L,2);S(L,3)
660 PRINT
665 RETURN
670 REM                                         *** CHOOSE OPTION ***
675 PRINT:PRINT "Shoot or move "
680 INPUT "(Type 0 for SHOOT or 1 for MOVE)";I
685 IF I<>0 THEN 700
690 O=1
695 RETURN
700 IF I<>1 THEN PRINT CHR$(7):GOTO 675
705 O=2
710 RETURN
715 REM                                         *** ARROW ROUTINE ***
720 F=0
725 REM                                         *** PATH OF ARROW ***
735 PRINT "No. of rooms (1-5)";
740 INPUT J9
745 IF J9<1 THEN 735
750 IF J9>5 THEN 735
755 FOR K=1 TO J9
760 PRINT "Room #";
765 INPUT P(K)
770 IF K<=2 THEN 790
775 IF P(K)<>P(K-2) THEN 790
780 PRINT "Arrows aren't that crooked - try another room"
785 GOTO 760
790 NEXT K
795 REM                                           *** SHOOT ARROW ***
800 L=L(1)
805 FOR K=1 TO J9
810 FOR K1=1 TO 3
815 IF S(L,K1)=P(K) THEN 895
820 NEXT K1
825 REM                                   *** NO TUNNEL FOR ARROW ***
830 L=S(L,FNB(1))
835 GOTO 900
840 NEXT K
845 PRINT "Missed"
850 L=L(1)
855 REM                                           *** MOVE WUMPUS ***
860 GOSUB 935
865 REM                                            *** AMMO CHECK ***
870 A=A-1
875 IF A>0 THEN 885
880 F=-1
885 RETURN
890 REM                         *** SEE IF ARROW IS AT L(1) OR AT L(2)
895 L=P(K)
900 IF L<>L(2) THEN 920
905 PRINT:PRINT "Aha! You got the wumpus!"
910 F=1
915 RETURN
920 IF L<>L(1) THEN 840
925 PRINT:PRINT "Ouch! Arrow got you!"
930 GOTO 880
935 REM                                   *** MOVE WUMPUS ROUTINE ***
940 K=FNC(0)
945 IF K=4 THEN 955
950 L(2)=S(L(2),K)
955 IF L(2)<>L THEN 970
960 PRINT:PRINT "Tsk tsk tsk - Wumpus got you!"
965 F=-1
970 RETURN
975 REM                                         *** MOVE ROUTINE ***
980 F=0
985 PRINT "Where to";
990 INPUT L
995 IF L<1 THEN 985
1000 IF L>20 THEN 985
1005 FOR K=1 TO 3
1010 REM                                 *** CHECK IF LEGAL MOVE ***
1015 IF S(L(1),K)=L THEN 1045
1020 NEXT K
1025 IF L=L(1) THEN 1045
1030 PRINT "Not possible -";
1035 GOTO 985
1040 REM                                   *** CHECK FOR HAZARDS ***
1045 L(1)=L
1050 REM                                              *** WUMPUS ***
1055 IF L<>L(2) THEN 1090
1060 PRINT:PRINT "... Oops! Bumped a wumpus!"
1065 REM                                         *** MOVE WUMPUS ***
1070 GOSUB 940
1075 IF F=0 THEN 1090
1080 RETURN
1085 REM                                                 *** PIT ***
1090 IF L=L(3) THEN 1100
1095 IF L<>L(4) THEN 1120
1100 PRINT:PRINT "YYYYIIIIEEEE . . . Fell in pit"
1105 F=-1
1110 RETURN
1115 REM                                                *** BATS ***
1120 IF L=L(5) THEN 1130
1125 IF L<>L(6) THEN 1145
1130 PRINT:PRINT "Zap -- Super Bat Snatch! Elsewhereville for you!"
1135 L=FNA(1)
1140 GOTO 1045
1145 RETURN
1150 SYSTEM

1130 PRINT:PRINT "Zap -- Super Bat Snatch! Elsewhereville for you!"
1135 L=FNA(1)
1140 GOTO 10