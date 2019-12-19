# What is SKBasic
SKBasicは組み込み用のBASICインタプリタです。STM32/MbedOS 5環境で実装されています。

#起動と終了
コマンドモードとBasicモードがあります。起動直後はコマンドモードになっており、以下のように入力することでBasicモードへ遷移します。

 basic(enter)
 
 SKBasic v0.9
 READY
 >

Basicモードでexit命令を実行するとコマンドモードへ戻ります。

 > exit(enter)
 OK

#ダイレクトコマンド
Basicモードのプロンプトから実行可能な命令をダイレクトコマンドと言います。主にBasicプログラムに対する表示、実行、保存などの命令があります。
コマンドは大文字・小文字の区別がありません。
コマンドサンプル中の"(enter)"はEnterキーの押下を表します。

**list [#bbc5e156]
プログラムリストの表示
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
***list vars [#d441705d]
使用中のプログラム変数の表示
 >list vars(enter)
 
 List of variables --
 arr()               failed              c                   n
 x
 
 >
***list ports [#c841e60d]
全組み込み変数の表示
 >list ports(enter)
 
 List of ports --
 s01           s02           s03           s04           s05
 s06           s07           s08           s09           s0a
 s0b           s0c           s0d           s0e           s0f
 s10           s11           s12           s13           s14
 s15           s16           s17           s18           s19
 s1a           s1b           s1c           s1d           s1e
 s1f           s20           s21           s22           s23
 s24           s25           s26           s27           s28
 sff           timer         erxorigin     erxdst        erxmsgid
 erxselector   erxrssi       erxlen        eackstatus    eackdst
 eackmsgid
 
 
 >
**run [#ae038c69]
プログラムの実行
**new [#d4a26ef4]
プログラムの消去
 >new(enter)
 
 This will erase the current program!  Are you sure? y
 
 Program erased.
 >
**cont [#k68287fc]
STOP文で中断されたプログラムを再開します。
**clear [#l29268c1]
変数の内容を消去します。
 >list vars(enter)
 
 List of variables --
 arr()               failed              c                   n
 x
 
 >? c(enter)
 2
 
 READY
 >? n(enter)
 10
 
 READY
 >? x(enter)
 5
 
 READY
 >clear(enter)
 
 >list vars(enter)
 
 List of variables --
 arr()               failed              c                   n
 x
 
 >? c(enter)
 0
 
 READY
 >? n(enter)
 0
 
 READY
 >? x(enter)
 0
 
 READY
 >
**free [#z0063ca7]
空きメモリの表示
 >free(enter)
 
 Program memory: 8191 bytes free
 Variable memory: 2047 bytes free
 Dynamic memory pool (arrays): 1000 bytes free
 
 >
**load [#gc6cf37f]
flashディスクに保存されているプログラムの読み込み
 >load label_test1.bas(enter)
 
 >

**save [#fe59b29c]
flashディスクにプログラムを保存します
ファイル名をautorun.basで保存した場合、次回SKBasic起動時に自動的に実行します。
 >save SKBasicTest.bas(enter)
 >save autorun.bas(enter)
**delete [#kba91d17]
flashディスクに保存されているプログラムの削除
 >delete autorun.bas(enter)
**format [#hcee3a8a]
flashディスクの消去
**files [#e2a27aeb]
flashディスクに保存されているプログラムの表示
 >files(enter)
 
 SKBasicTest2.bas
 SKBasicTest3.bas
 rxdata_ack_test2.bas
 SKBasicTest4.bas
 SKBasicTest5.bas
 autorun.bas
 wait_test.bas
 label_test1.bas
 SKBasicTestLabel.bas
 
 95232 bytes available.
 >

**SKコマンド　[#t2a8baf5]
既存のSKコマンドは従来の書式そのままで入力を受け付けます。

 >SKINFO(enter)
 EINFO 0002 FFFF EA60 21


*プログラムコマンド [#necc0a29]
Basicプログラム中で実行可能な命令をプログラムコマンドと呼びます。

**data [#k552cd93]
プログラム中に数値データを列挙します。~
後述のread文で読み込みます。
 10 data 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
 20 restore
 30 read first
 40 print "first =";first
 50 read second
 60 print "second =";second

**read [#fe2d9b3f]
data文で列挙したデータを変数に読み込みます。~
読み込み前に必ずrestoreする必要があります。

**restore [#j69b83c2]
data文で列挙したデータの読み込み位置を先頭にセットします。~
restore文実行後は必ずdata文の先頭から読み込むのでプログラム中に何回でも再読み込みが可能です。

**label [#f2c68480]
行番号にラベルを付与します。GOTO文やGOSUB文の参照先に指定出来ます。

 100 label "jump_to"

**goto [#x7e68e93]
プログラムの実行位置を指定された行番号・またはラベル名に変更します。

 10 goto 100
 10 goto "jump_to"

**gosub [#r3a04f60]
プログラムの現在の実行位置をスタックに置いた上で実行位置を指定された行番号・またはラベル名に変更します。(サブルーチンコール)~

 10 gosub 9000

**return [#p52d21fa]
サブルーチンから戻ります。

 9000 return

**if then else [#rb2389c3]
条件分岐をします。

:if 条件式 then 行番号 else 行番号|
:if 条件式 then 命令文|

 >list
 
 100 for i=1 to 10
 110   if (i - ((i / 2) * 2)) = 0 then print "num ";i;" is even"
 120 next i
 
 >run
 
 num 2  is even
 num 4  is even
 num 6  is even
 num 8  is even
 num 10  is even

**input [#tbda8f32]
コンソールから数値を入力し変数に格納します。

 >input a(enter)
 ? 3
 
 
 READY
 >? a(enter)
 3
 
 READY
 >


**print [#s8ac59b0]
文字列や変数値をコンソールに出力します。セミコロン(;)を使用する事により連結出力が可能です。

 print "a = ";a

**? [#idcb4a30]
printの省略形です

 ? "a =";a

**for next [#s8fb626f]
開始値から終了値まで繰り返します。~
next文の変数は省略出来ません。

 10 'test1
 20 for i = 1 to 10
 30   sum = sum + i
 40 next i
 50 print "sum = ";sum

**stop [#i6d56a49]
実行中のプログラムを中断します。CONTコマンドにより再開します。
**end [#m201d88e]
プログラムの終了を宣言します。
**rem [#p675dde7]
プログラム中にコメントを記述します。プログラムの実行に何の影響も与えません。
**' [#c34c0421]
remの省略形です。
 10 'test1
 20 for i = 1 to 10
 30   sum = sum + i
 40 next i
 50 print "sum = ";sum
**dim [#k35873de]
数値変数の配列を宣言します。

 10 dim attr(10)
 20 for i=1 to 10
 30   attr(i)=i
 40 next i

**tron [#u9ba3885]
現在実行しているプログラムの行番号を逐一表示します。
**troff [#rb1a9c63]
プログラムの行番号表示を停止します。
 >list
 
 10 tron
 20 for i = 1 to 10
 30   sum = sum + i
 40 next i
 50 troff
 
 >run
 
 [20] [30] [40] [30] [40] [30] [40] [30] [40] [30] [40] [30] [40] [30] [40] [30] [40] [30] [40] [30] [40] [50]
 
 >
**while endwh [#h047c9bd]
条件式が偽になるまで繰り返します。

 >list
 
 10 c=0
 20 while c < 10
 30   input c
 40 endwh
 
 >run
 
 ? 1
 
 ? 5
 
 ? 10
 
 
 
 >

**wait [#h36d2042]
指定した時間だけプログラムの実行を待機します（単位はミリ秒で精度は10ミリ秒）。~
待機時間中もプロトコルは動作を継続していますので、途中でデータを受信した場合はRXDATAハンドラが呼ばれます。~

 10 on RXDATA gosub 9000
 20 wait 10000
 30 print "receive ";count;"packets"
 40 end
 9000 '
 9010 print "rxdata ";erxmsgid
 9020 count = count + 1
 9030 return

**sleep [#z0476378]
指定時間のスリープをします（単位は秒）~
スリープ中はプロトコルも動作が停止し、送受信は行われなくなります。~

 10 print "sleep 10 secs"
 20 sleep 10
 30 print "wake"

**strcat [#wfc99739]
文字列の連結をします

**strind [#g8977be0]
文字列の指定バイト目の値を設定します。

 >a$="abcdefghijklmn"
 
 READY
 >strind(a$, 5)=103
 
 READY
 >? a$
 abcdegghijklmn
 
 READY
 >

**on gosub [#m4944068]
割り込みサブルーチンを指定します。~
現在2つの識別子をサポートしています。

***on rxdata gosub [#r98f69b0]
データ受信割り込みのサブルーチンを指定します。

***on ack gosub [#v9533123]
ACK受信割り込みのサブルーチンを指定します。

**SKSEND [#rade4fd4]

:SKSEND|
:  <ACK>|1=送信相手からのAckを要求します。~
0=Ackを要求しません。
:  <SELECTOR>|セレクタ番号~
$3e8(=1000)以上の値が指定可能
:  <ADDR>|送信相手先ID($0001 - $ffef)
:  <DATALEN>|送信データ長(省略可能)
:  <DATA>|送信データ

 100 msgid = SKSEND 1 1001 3 "abc"
 100 SKSEND 1 1001 "abc"

**SKBC [#r687141e]

:SKBC|
:  <RADIUS>|ブロードキャストの最大転送ホップ数(0 - 15)
:  <SELECTOR>|セレクタ番号~
$3e8(=1000)以上の値が指定可能
:  <DATALEN>|送信データ長(省略可能)
:  <DATA>|送信データ

 100 msgid = SKBC 2 1001 3 "abc"
 100 SKBC 2 1001 "abc"

**SKFLASH [#g8ec157c]

: SKFLASH|
:  <SELECTOR>|セレクタ番号~
$3e8(=1000)以上の値が指定可能
:  <ADDR>|送信相手先ID($0001 - $ffef)
:  <DATALEN>|送信データ長(省略可能)
:  <DATA>|送信データ

''例:$0003へ5バイトのデータを委譲送信、$0003から送信確認を受諾''
 >skflash 1001 $0003 "abcde"
 6F59 OK
 READY
 >ERXDATA 0003 0002 6F59 03E9 50 00
 OK


**SKINQ [#w11d9d8f]
周辺の端末を能動的に探索して発見します。発見された端末はEDETECTイベントで通知されます。

**SKPAIR [#n68208c0]

:SKPAIR|
:  <ADDR>|ペアリングを実行する相手ID($0001 – $FFEF、または$FFFF)~
$FFFFを指定するとワイルドカード

**SKUNPAIR [#u11962be]

:SKUNPAIR|
:  <ADDR>|ペアリングを実行する相手ID($0001 – $FFEF、または$FFFF)

**SKNOW [#pac9c74a]

 >sknow(enter)
 ECLOCK 3 9 24 700 0


*関数 [#p3d60d1c]
**abs() [#q468dfaf]
整数の絶対値を計算します。

**sqrt() [#q4681d1f]
整数の平方根を計算します。~
結果は四捨五入された整数で返されます。

 >? sqrt(2)
 1
 
 READY
 >? sqrt(4)
 2
 
 READY
 >? sqrt(3)
 2
 
 READY
 >

**rnd() [#f5eff6dd]
指定値未満の乱数値を返します。

 >? rnd(65535)
 12134
 
 READY
 >? rnd(65535)
 20941
 
 READY
 >? rnd(65535)
 40891
 
 READY
 >

**sgn() [#c8e2a42b]
整数の符号を返します。

**tab() [#td867d79]
TAB文字を印字します。引数が必ず必要ですが使っていません。

 >? tab(1);";"
         ;
 
 READY
 >? tab(2);";"
         ;
 
 READY
 >
**strcmp() [#p5dcdb31]
二つの文字列を比べます。

**strind() [#xba017af]
文字列の指定バイト目の値を参照します。

 >a$="abcdefghijklmn"
 
 READY
 >? strind(a$, 4)
 101
 
 READY
 >? strind(a$, 5)
 102
 
 READY
 >

**strlen() [#s3facda2]
文字列の長さを返します。

**chr$() [#g2ca6762]
数値の文字を印字します。PRINT文でのみ使用可能です。
```
 >? chr$(105)
 i
 
 READY
 >? chr$(106)
 j
 
 READY
 >
```

**hex()
数値を16進数の8桁で印字します。PRINT文でのみ使用可能です。

**hex2() 
数値を16進数の2桁で印字します。PRINT文でのみ使用可能です。

**hex4() 
数値を16進数の4桁で印字します。PRINT文でのみ使用可能です。
```
 >? hex($12345678)
 12345678
 
 READY
 >? hex2($12345678)
 78
 
 READY
 >? hex4($12345678)
 5678
 
 READY
 >
```

*組み込み変数
**ペリフェラル
Basicプログラム内からペリフェラルへのアクセスが可能です。
LED1を点灯:
```
 READY
 >led1 = 1
 
 READY
 >print led1
 1
```

*定数
**10進数
10
**16進数
$10
**文字列
"skbasic"
**16進データ
$"023456789abcdef"

*サンプルコード
**for next, data, read, restore
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

**print, hex(), hex2, hex4(), gosub, label
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

##基本コマンドを網羅したテストコード
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
