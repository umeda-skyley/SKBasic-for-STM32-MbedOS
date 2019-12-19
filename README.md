# What is SKBasic
SKBasicは、"KLBasic"をSTM32/MbedOS環境に移植したBASICインタプリタです。
マイコンプログラムをもっと手軽で素早く開発できるよう、いくつかの機能拡張を行っています。

## 起動と終了
コマンドモードとBasicモードがあります。起動直後はコマンドモードになっており、以下のように入力することでBasicモードへ遷移します。
```
 basic(enter)
 
 SKBasic for STM32/Mbed5 v0.9
 READY
 >
```

Basicモードでexit命令を実行するとコマンドモードへ戻ります。
```
 > exit(enter)
 OK
```

# コマンドの紹介
## list
プログラムリストの表示
```
 >list(enter)
 
 110 '  Test of KLBasic functions and operators
 120 '
 1020 dim arr(10)
 1030 failed = 0
 1100 data -5, -4, -3, -2, -1, 0
 1105 data 1, 2, 3, 4, 5
 1110 data 6, 7, 8, 9, 10, 11, 12, -100
 .
 .
 .
  OK
```

## list vars
使用中のプログラム変数の表示
```
 >list vars(enter)
 
 List of variables --
 arr()               failed              c                   n
 x
 
 >
```
 
## list ports
全組み込み変数の表示
```
 >list ports(enter)
 
 List of ports --
led1          led2          timer1        timer2
 
 
 >
```
 
## run
プログラムの実行

## new
プログラムの消去
```
 >new(enter)
 
 This will erase the current program!  Are you sure? y
 
 Program erased.
 >
```

他にも多くのコマンドが用意されています。詳しくはwikiを参照してください。
https://github.com/umeda-skyley/SKBasic-for-STM32-MbedOS/wiki

# サンプルコード
## or next, data, read, restoreの使い方
```
 >list
 
 10 data 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
 20 sum=0:fact=1
 30 restore
 40 for i=1 to 10
 50   read x
 60   sum = sum + x
 70 next i
 80 print "sum =";sum
 90 restore
 100 for i=1 to 10
 110   read x
 120   fact = fact * x
 130 next i
 140 print "fact =";fact
 
 >run
 
 sum =55
 fact =3628800
 
 
 >
```

## print, hex(), hex2, hex4(), gosub, labelの使い方
```
 >list
 
 2100 c = 1 : gosub "print_hex"
 2110 c = -1 : gosub "print_hex"
 2120 c = $7fffffff : gosub "print_hex"
 2130 c = $7fffffff + 1 : gosub "print_hex"
 2140 c = $12345678 : gosub "print_hex"
 8900 end
 9300 '
 9310 ' print c in various forms of hex
 9320 label "print_hex"
 9330 '
 9350 print "c = ";c; " HEX() = "; hex(c); " HEX4() = "; hex4(c); " HEX2() = "; hex2(c)
 9360 return
 
 >run
 
 c = 1  HEX() = 00000001  HEX4() = 0001  HEX2() = 01
 c = -1  HEX() = FFFFFFFF  HEX4() = FFFF  HEX2() = FF
 c = 2147483647  HEX() = 7FFFFFFF  HEX4() = FFFF  HEX2() = FF
 c = -2147483648  HEX() = 80000000  HEX4() = 0000  HEX2() = 00
 c = 305419896  HEX() = 12345678  HEX4() = 5678  HEX2() = 78
```

## 基本コマンドを網羅したテストコード
```
 110 '  Test of KLBasic functions and operators
 120 '
 1020 dim arr(10)
 1030 failed = 0
 1100 data -5, -4, -3, -2, -1, 0
 1105 data 1, 2, 3, 4, 5
 1110 data 6, 7, 8, 9, 10, 11, 12, -100
 1120 if 3000 * 2 <> 6000 then print "Fail in multiplication" : gosub "fail"
 1130 if 3000 / 2 <> 1500 then print "Fail in division" : gosub "fail"
 1140 if -3000 * 2 <> -6000 then print "Fail in multiplication (2)" : gosub "fail"
 1150 if -3000 / 2 <> -1500 then print "Fail in division (2)" : gosub "fail"
 1160 if 67 mod 10 <> 7 then print "Fail in MOD" : gosub "fail"
 1170 if -67 mod 10 <> -7 then print "Fail in MOD (2)" : gosub "fail"
 1200 if $40 <> 64 then print "Fail in hex notation" : gosub "fail"
 1210 if ($55 and $ff) <> $55 then print "Fail in AND" : gosub "fail"
 1220 if ($55 and $00) <> $00 then print "Fail in AND (2)" : gosub "fail"
 1230 if ($55 or $ff) <> $ff then print "Fail in OR" : gosub "fail"
 1240 if ($55 or $00) <> $55 then print "Fail in OR (2)" : gosub "fail"
 1250 if ($55 eor $ff) <> $aa then print "Fail in EOR" : gosub "fail"
 1260 if ($55 eor $00) <> $55 then print "Fail in EOR (2)" : gosub "fail"
 1280 if not $55 <> $ffffffaa then print "Fail in NOT" : gosub "fail"
 1300 if -$55 <> $ffffffab then print "Fail in negation" : gosub "fail"
 1400 c = 0 : for n = 1 to 10 : c = c + n : next n
 1410 if c <> 55 then print "Fail in FOR loop; c = "; c : gosub "fail"
 1420 c = 0 : for n = 10 to 1 step -1 : c = c + n : next n
 1430 if c <> 55 then print "Fail in FOR loop, negative STEP" : gosub "fail"
 1440 c = 0 : for n = -1 to -10 step -1 : c = c + n : next n
 1450 if c <> -55 then print "Fail in FOR loop, negative STEP (2)" : gosub "fail"
 1500 c = 0 : n = 1
 1502 while n < 11
 1504   c = c + n : n = n + 1
 1506 endwh
 1510 if c <> 55 then print "Fail in WHILE loop" : gosub "fail"
 1600 print "Using GOSUB to add values in a loop."
 1605 c = 0 : for n = 1 to 10 : gosub "sub"
 1610 next n : print
 1630 if c <> 55 then print "Fail in FOR/GOSUB loop" : gosub "fail"
 1700 print "Restoring data (first time)..." : restore
 1702 print "Reading data and summing values"
 1705 c = 0 : for n = 0 to 10 : read x : c = c + x : next n
 1710 if n <> 11 then print "Fail in FOR/READ loop, index is wrong" : gosub "fail"
 1720 if c <> 0 then print "Fail in FOR/READ loop, sum is wrong" : gosub "fail"
 1800 print "Restoring data (second time)..." : restore
 1802 print "Reading data and summing values"
 1810 c = 0 : for n = 0 to 10 : read x : c = c + x : next n
 1820 if n <> 11 then print "Fail in RESTORE/FOR/READ loop, index is wrong" : gosub "fail"
 1830 if c <> 0 then print "Fail in RESTORE/FOR/READ loop, sum is wrong" : gosub "fail"
 1900 print "Turning on trace"
 1910 tron
 1920 for n = 0 to 3
 1930   c = 0
 1940 next n
 1950 troff
 1960 print : print "Trace is now off"
 2000 print "Testing ON-GOTO"
 2010 print "ON-GOTO not implemented, test skipped"
 2100 c = 1 : gosub "print_hex"
 2110 c = -1 : gosub "print_hex"
 2120 c = $7fffffff : gosub "print_hex"
 2130 c = $7fffffff + 1 : gosub "print_hex"
 2140 c = $12345678 : gosub "print_hex"
 2400 print "Filling array with 0-9"
 2410 for n = 0 to 9 : arr(n) = n : next n
 2420 for n = 0 to 9 : print arr(n); : next n
 2430 print
 2500 print "Input a number"; : input c
 2510 print "You entered "; c
 2520 input "(Using input() with string) Input a number", c
 2530 print "You entered "; c
 2600 print "Program will now stop; enter CONT to continue."
 2610 stop
 2620 print "Program resumes..."
 2700 if abs(1) <> 1 then print "Fail in ABS(1)" : gosub "fail"
 2710 if abs(-1) <> 1 then print "Fail in ABS(-1)" : gosub "fail"
 2720 if abs(-$7fffffff) <> 1 then print "Fail in ABS(-$7fffffff)" : gosub "fail"
 2730 if abs(-555) <> 555 then print "Fail in ABS(-555)" : gosub "fail"
 2800 if sgn(0) <> 0 then print "Fail in SGN(0)" : gosub "fail"
 2810 if sgn(20) <> 1 then print "Fail in SGN(20)" : gosub "fail"
 2820 if sgn(-33) <> -1 then print "Fail in SGN(-33)" : gosub "fail"
 2900 print "Sleep 10 secs start"
 2910 sleep 10
 2930 print "Sleep end"
 8000 '
 8010 ' end of mainline program; put subroutines below here
 8020 '
 8910 print "Test is complete, fail count = "; failed
 8999 end
 9000 '
 9010 ' Common code for counting failures
 9020 label "fail"
 9100 failed = failed + 1
 9110 return
 9200 '
 9210 ' helper subroutine for testing loops
 9220 label "sub"
 9230 c = c + n
 9235 print ".";
 9240 return
 9300 '
 9310 ' print c in various forms of hex
 9320 label "print_hex"
 9350 print "c = ";c; " HEX() = "; hex(c); " HEX4() = "; hex4(c); " HEX2() = "; hex2(c)
 9360 return
```
