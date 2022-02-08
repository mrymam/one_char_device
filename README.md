# one char device driver
zero デバイスドライバという0を出力するドライバがLinuxカーネルに一般的にありますが、本デバイスは1を出力するドライバです。
バッファに0埋めした後、ビット反転をして、2進数で全て1を出力するように実装してあります。

# useage

compile and install device driver
```bash
$ sudo make
$ sudo insmod myone.ko
```

# experiment

```bash
$ sudo insmod myone.ko 
$ sudo dd if=/dev/myone of=out bs=3 count=1
1+0 records in
1+0 records out
3 bytes copied, 0.000204596 s, 14.7 kB/s
$ xxd -g 1 -b out
00000000: 11111111 11111111 11111111                             ...
$ sudo dd if=/dev/myone of=out bs=4 count=1
1+0 records in
1+0 records out
4 bytes copied, 0.000321123 s, 12.5 kB/s
$ xxd -g 1 -b out
00000000: 11111111 11111111 11111111 11111111                    ....
```

# environment
```
$ uname -r 5.11.0-49-generic
$ hostnamectl
  Static hostname: myone-VirtualBox
         Icon name: computer-vm
           Chassis: vm
        Machine ID: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
           Boot ID: aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    Virtualization: oracle
  Operating System: Ubuntu 21.04
            Kernel: Linux 5.11.0-49-generic
      Architecture: x86-64
```
