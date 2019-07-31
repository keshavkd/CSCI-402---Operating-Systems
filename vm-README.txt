Documentation for Kernel Assignment 3
=====================================

+------------------------+
| BUILD & RUN (Required) |
+------------------------+

Replace "(Comments?)" with the command the grader should use to compile your kernel (should simply be "make").
    To compile the kernel, the grader should type: make
If you have additional instruction for the grader, replace "(Comments?)" with your instruction (or with the word "none" if you don't have additional instructions):
    Additional instructions for building/running this assignment: none

+-----------------------+
| USER SHELL (Required) |
+-----------------------+

Is the following statement correct about your submission (please replace
    "(Comments?)" with either "yes" or "no" and if your answer is "no", please
    provide additional information for grading without user-space shell)?
    We run /sbin/init from the kernel init process in our submission": yes

+-------------------------+
| SELF-GRADING (Required) |
+-------------------------+

Replace each "(Comments?)" with appropriate information and each stand-alone "?"
with a numeric value:

(A.1) In mm/pframe.c:
    (a) In pframe_get(): 2 out of 2 pt
    (b) In pframe_pin(): 1 out of 1 pt
    (c) In pframe_unpin(): 1 out of 1 pt

(A.2) In vm/mmap.c:
    (a) In do_mmap(): 2 out of 2 pts

(A.3) In vm/vmmap.c:
    (a) In vmmap_destroy(): 2 out of 2 pts
    (b) In vmmap_insert(): 4 out of 4 pts
    (c) In vmmap_lookup(): 2 out of 2 pt
    (d) In vmmap_map(): 3 out of 3 pts
    (e) In vmmap_is_range_empty(): 2 out of 2 pt

(A.4) In vm/anon.c:
    (a) In anon_init(): 1 out of 1 pt
    (b) In anon_ref(): 1 out of 1 pt
    (c) In anon_put(): 1 out of 1 pt
    (d) In anon_fillpage(): 1 out of 1 pt

(A.5) In vm/pagefault.c:
    (a) In handle_pagefault(): 2 out of 2 pts

(A.6) In vm/shadow.c:
    (a) In shadow_init(): 1 out of 1 pt
    (b) In shadow_ref(): 1 out of 1 pt
    (c) In shadow_put(): 1 out of 1 pts
    (d) In shadow_lookuppage(): 2 out of 2 pts
    (e) In shadow_fillpage(): 2 out of 2 pts

(A.7) In proc/fork.c:
    (a) In do_fork(): 6 out of 6 pts

(A.8) In proc/kthread.c:
    (a) In kthread_clone(): 2 out of 2 pts

(B.1) /usr/bin/hello (3 out of 3 pts)
(B.2) /usr/bin/args ab cde fghi j (2 out of 2 pts)
(B.3) /bin/uname -a (2 out of 2 pts)
(B.4) /bin/stat /README (1 out of 1 pt)
(B.5) /bin/stat /usr (1 out of 1 pt)
(B.6) /bin/ls /usr/bin (1 out of 1 pt)
(B.7) /usr/bin/fork-and-wait (5 out of 5 pts)

(C.1) help (1 out of 1 pt)
(C.2) echo hello (1 out of 1 pt)
(C.3) cat /README (1 out of 1 pt)
(C.4) /bin/ls (1 out of 1 pt)
(C.5) segfault (1 out of 1 pt)

(D.1) /usr/bin/vfstest (6 out of 6 pts)
(D.2) /usr/bin/memtest (6 out of 6 pts)
(D.3) /usr/bin/eatmem (6 out of 6 pts)
(D.4) /usr/bin/forkbomb (6 out of 6 pts)
(D.5) /usr/bin/stress (6 out of 6 pts)

(E.1) /usr/bin/vfstest (0 out of 1 pt)
(E.2) /usr/bin/memtest (0 out of 1 pt)
(E.3) /usr/bin/eatmem (0 out of 1 pt)
(E.4) /usr/bin/forkbomb (0 out of 1 pt)
(E.5) /usr/bin/stress (0 out of 1 pt)

(F) Self-checks: (10 out of 10 pts)
    Please provide details, add subsections and/or items as needed; or, say that "none is needed".
    Details: (Comments?)

Missing/incomplete required section(s) in README file (vm-README.txt): (-0 pts)
Submitted binary file : (-0 pts)
Submitted extra (unmodified) file : (-0 pts)
Wrong file location in submission : (-0 pts)
Extra printout when running with DBG=error,test in Config.mk : (-0 pts)
Incorrectly formatted or mis-labeled "conforming dbg() calls" : (-0 pts)
Cannot compile : (-0 pts)
Compiler warnings : (-0 pts) 
"make clean" : (-0 pts) 
Kernel panic : (-0 pts)
Kernel freezes : (-0 pts)
Cannot halt kernel cleanly : (-10 pts)

+--------------------------------------+
| CONTRIBUTION FROM MEMBERS (Required) |
+--------------------------------------+

1)  Names and USC e-mail addresses of team members: 
							Aarushi Arya : aarushia@usc.edu 
							Naren Teja Divvala : divvala@usc.edu
							Keshav D Karanth : kkaranth@usc.edu
							Varidhi Garg : varidhig@usc.edu
2)  Is the following statement correct about your submission (please replace
        "(Comments?)" with either "yes" or "no", and if the answer is "no",
        please list percentages for each team member)?  "Each team member
        contributed equally in this assignment": yes

+---------------------------------+
| BUGS / TESTS TO SKIP (Required) |
+---------------------------------+

Are there are any tests mentioned in the grading guidelines test suite that you
know that it's not working and you don't want the grader to run it at all so you
won't get extra deductions, please replace "(Comments?)" below with your list.
(Of course, if the grader won't run such tests in the plus points section, you
will not get plus points for them; if the garder won't run such tests in the
minus points section, you will lose all the points there.)  If there's nothing
the grader should skip, please replace "(Comments?)" with "none".

Please skip the following tests: none

+-----------------------------------------------+
| OTHER (Optional) - Not considered for grading |
+-----------------------------------------------+

Comments on design decisions: none
