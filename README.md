# What is SKBasic
SKBasicはKarl Lunt氏の"KLBasic"をSTM32/MbedOS環境に移植したBASICインタプリタです。マイコンプログラムがもっと手軽に素早く開発できるようになると良いと思い公開いたしました。組み込み向けに簡単な機能拡張もいくつか追加しています。

## 実行環境
STマイクロのNucleoボードとARM MbedOS 5の組み合わせで動作させる想定になっています。インタプリタ自体は環境依存性が小さいので、他のMCUにも移植は容易です。

## インタプリタの起動と終了
SKBasic for STM32/MbedOS には、コマンドモードとBasicモードがあります。起動直後はコマンドモードになっており、シリアル入力から以下のように入力することでBasicモードへ遷移します。ここからBASICプログラムの入力、修正、デバッグ、コードスニペットの対話的な実行などが行なえます。
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

## LEDの制御
led1, led2が予約語として組み込まれています。

ターゲットボード上のLED1を点灯：
```
>led1 = 1

READY
>print led1
1

READY
>
```

他にも多くのコマンドが用意されています。

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

## Lチカ
```
9000 while 1
9010   led1 = 1
9020   wait 1000
9030   led1 = 0
9040   wait 1000
9050 endwh
```

# もっと詳しく
https://github.com/umeda-skyley/SKBasic-for-STM32-MbedOS/wiki
