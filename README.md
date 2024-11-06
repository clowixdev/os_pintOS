# Laboratory course on "Operating Systems". 2 course, 1 semester, IC of SPBSTU

There will be listed some labs that I need to complete during OS course.

## About

All the labs and reports that will be completed during this course will be located in the related branches. In this branches you can check pintOS source code in the exact state of completing OS course during semester.

## Installing PintOS (using **Ubuntu 16.04 x64**)

---

### *ATTENTION!*

#### I strongly recommend you to use Ubuntu `16.04` as VM to install PintOS on, because all the needed version of software will be installed automatically, and PintOS will launch without any bugs

---

*You can check step-by-step guide in Russian [here](./pintos_install.pdf).*

**1. Gathering all needed packages with command:**

```console
user@host:~$ sudo apt-get install gcc-multilib make perl qemu
```

&emsp;&emsp;you can use:

- `qemu-system` instead of `qemu` if qemu is not found.
- `gcc` instead of `gcc-multilib` if you have `x86` system.

**2. Creating a link to ease emulation process:**

```console
user@host:~$ ln /usr/bin/qemu-system-x86_64 /usr/bin/qemu
```

If you have any problems with this step, check the location where `qemu-system` is installed and edit command using your installation location. P.S. for `x86` use `qemu-system-x86`

**3. Unpacking PintOS source files:**

```console
user@host:~$ tar –zxf pintos.tar.gz
```

Extract all the source files where it will be handy for you to edit them

**4. Preparing scripts to compile and run PintOS:**

```console
user@host:~$ cd pintos/src/utils
user@host:~/pintos/src/utils$ sudo cp -f backtrace pintos pintos-mkdisk pintos-set-cmdline Pintos.pm pintos-gdb ../misc/gdb-macros /usr/bin/
user@host:~/pintos/src/utils$ sudo chmod 755 /usr/bin/backtrace /usr/bin/pintos /usr/bin/pintos-mkdisk /usr/bin/pintos-setcmdline /usr/bin/pintos-gdb
user@host:~/pintos/src/utils$ sudo chmod 644 /usr/bin/Pintos.pm /usr/bin/gdb-macros
```

By doing this, you moving all the files to system `/usr/bin` folder and granting them rights to be read and executed

## Fixing known issues

**1. Encoding bug:**

When you've done all the steps above, you can meet one known bug that is caused by encoding:

```console
Unrecognized character \x16; marked by <-- HERE after   if ($<-- HERE near column 7 at /usr/bin/pintos line 911.
```

In order to fix it, you need to re-type `$^V` sequence in `/usr/bin/pintos` file exactly in `911` string `if ($^V ge 5.8.0) { ... }`. After doing this everything should work fine.

**2. Infinity loading QEMU bug:**

This bug should look like QEMU is infinitely relaunching without changing it's output like this:

```console
...
Loading.............
Kernel command line: -q run alarm-single
Pintos booting with PiLo hda1
Loading.............
Kernel command line: -q run alarm-single
Pintos booting with PiLo hda1
Loading.............
...
```

This bug is caused by wrong version of GCC and GDB. The 100% supported version is `5.4.x` and as I said in the beginning, everything will work fine if you use `16.04 Ubuntu`, because all the needed version will be installed automatically. Because you don't use correct version, therefore this bug is appearing. In order to fix it, you should install `GCC/GDB v5.4.x`. Good luck with that!

### Congrats! On this step, everything is ready to work with PintOS

&emsp;

## Launching tests in PintOS

*You can check step-by-step guide in Russian [here](./pintos_testing.pdf).*

To launch any test in PintOS, you need to compile its kernel by using `make` command. Enter `pintos/src/threads` and compile kernel here by using:

```console
user@host:~$ cd pintos/src/threads
user@host:~$ /pintos/src/threads$ make
```

Then, you can launch any tests and get results with this syntax, where you change **\<LABDIR>** and **\<TESTDIR>** to what you need:

```console
user@host:~$ cd pintos/src/threads/build
user@host:~/pintos/src/threads/build$ rm tests/<LABDIR>/*.result && (make tests/<LABDIR>/<TESTDIR>.result && make ... ) | grep -E "pass | fail"
```

After executing this command, you will see all the test results without any extra output:

```console
...
pass /tests/<LABDIR>/<TESTDIR>.o
fail /tests/<LABDIR>/<TESTDIR>.o
...
```

## Prepare all the code to be tested on IBKS

*You can check step-by-step guide in Russian [here](./pintos_guide.pdf) in III block.*

After you have done coding and passing all the needed tests, you have to pack source code correctly to load it on IBKS. To do that, execute that command:

```console
user@host:~$ cd pintos/src && make clean && cd ../../ && tar –zcf pintos_lab<LABNUMBER>.tar.gz pintos
```

Change the **\<LABNUMBER>** to the needed one and you are ready to go, load the output archive to the site and check how much test you've passed.

### Have fun exploring how operating systems works using this small OS

&emsp;

## LABs

### *Lab 1: (System timer)*

#### In this Laboratory work the task was to modify system timer to optimize active waiting

Modifications were added to `timer.c` and `timer.h` files in `src/devices/` folder. You can check all the work that have been done in `lab1/pintos_lab1.tar.gz` archive in this repository. Report is also in `lab1` folder.

The main idea was to create a dynamic-queue that sorts elements on adding and to turn on active waiting for the element that is more likely to be woken up earlier.

Using `Ubuntu Desktop 16.04 LTS` as a virtual machine to launch PintOS on, files were sent to VM using `scp`. After that, kernel was recompiled and first test was executed to test the correctness of code (is it even compiling?) with this cmd sequence:

```console
user@host:~$ mv ~/shared/timer.c ~/shared/timer.h ~/shared/pintos/src/devices/ && make clean && make && pintos --qemu -- -q run alarm-single
```

After that, it is needed to check is current code is working correct. For that, all the needed tests are executed with:

```console
user@host:~/pintos/src/threads/build$ rm tests/threads/*.result && (make tests/threads/alarm-zero.result && make tests/threads/alarm-negative.result && make tests/threads/alarm-single.result && make tests/threads/alarm-simultaneous.result && make tests/threads/alarm-multiple.result) | grep -E "pass|FAIL"
```

After that, all the tests were passed on the IBKS, and the lab was done! You can find [pdf](./lab1/lab1os.pdf) report and [docx](./lab1/lab1os.docx) report for that laboratory work.

>STATUS: Accepted!

### *Lab 2: (Scheduler)*

#### In this Laboratory work the task was to modify system scheduler to consider thread priorities and add priority donation system

A couple of system files were modified, here is list of modified files:

- `thread.c`
- `thread.h`
- `synch.c`
- `synch.h`

Also, double-linked-list was updated with some methods and `list.h`, `list.c` files were also modified. You can check all the work that have been done in `lab2/pintos_lab2.tar.gz` archive in this repository. Report is also in `lab2` folder.

The main idea was to force scheduler to choose process with the highest priority in "ready" queue and choose it as a next thread to run. In the second part of the lab, priority-donation-system was created. Thread can exchange their priorities in order not to block critical section.

Files were sent to VM using `scp` and checked locally, if it is even compiling, with this cmd sequence:

```console
user@host:~$ cd ~/shared && mv synch.c synch.h thread.c thread.h pintos/src/threads && mv list.c list.h pintos/src/lib/kernel && cd pintos/src/threads/build && make clean && make
```

After that, all tests were checked with:

```console
user@host:~/pintos/src/threads/build$ rm tests/threads/*.result && 
(make tests/threads/alarm-priority.result && 
make tests/threads/priority-change.result &&
make tests/threads/priority-fifo.result &&
make tests/threads/priority-preempt.result &&
make tests/threads/priority-sema.result &&
make tests/threads/priority-condvar.result &&
make tests/threads/priority-donate-one.result &&
make tests/threads/priority-donate-lower.result &&
make tests/threads/priority-donate-multiple.result &&
make tests/threads/priority-donate-multiple2.result &&
make tests/threads/priority-donate-sema.result &&
make tests/threads/priority-donate-nest.result &&
make tests/threads/priority-donate-chain.result) | grep -E "pass|FAIL"
```

All the tests were passed on the IBKS, and the lab was done! You can find [pdf](./lab2/lab2os.pdf) report and [docx](./lab2/lab2os.docx) report for that laboratory work.

>STATUS: Accepted!

### *Lab 3: (Narrow Bridge)*

#### In this Laboratory work the task was to create a solution for a "Narrow bridge" task

Only one system file was modified, because of restrictions:

- `narrow-bridge.c`

You can check all the work that have been done in `lab3/pintos_lab3.tar.gz` archive in this repository. Report is also in `lab3` folder. *(soon)*

The main idea was to create system of semaphores that will prevent cars from breaking bridge and each other.

**The rules for the bridge:**

- Not more then 2 cars at the same time at the bridge.
- Only one direction of movement at the same time.
- Emergency cars must go before others.

File was sent to VM using `scp` and checked locally, if it is even compiling and multiple tests were checked with this-like command:

```console
user@host:~/pintos/src/threads/build$ pintos --qemu -- -q run "narrow-bridge 1 2 3 4"
```

Numbers after "narrow-bridge" means, how much cars will be waiting. In this exact test, there will be 1 normal && 2 emergency on the left, and 3 normal && 4 emergency on the right

All the tests must be checked manually, and the lab was done! You can find [pdf](./lab3/lab3os.pdf) report and [docx](./lab3/lab3os.docx) report for that laboratory work.

>STATUS: Creating report

### *Lab 4: coming soon*

### *Lab 5: coming soon*
