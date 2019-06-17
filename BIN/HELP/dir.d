              ^1Pinnacle Directory layout and key files.^6                Page 1

^1/usr/ver(x)^6                     -       Directory.

      This directory contains all of Pinnacle with the following exceptions. 
            
          -  Pinnacle startup scripts in /etc.
          -  /etc/PSLttys. Used for Pinnacle Mail message terminal info.
          -  /etc/setterm. Used to interrogate the above.
          -  /usr/spool/cron/crontabs/root. Pinnacle specific line added.

        Non machine specific versions of files/scripts related to above are 
        held in /usr/CSRC/STD/SCRIPT/SQL/MISC .

^1/usr/ver(x)/DATA^6                -      Directory.

      This directory contains all User data.

              ^1Pinnacle Directory layout and key files.^6                Page 2

^1/usr/ver(x)/BIN^6                 -      Directory.

      This directory contains all Pinnacle program and operating environment
      files.

^1/usr/ver(x)/LETTERS^6             -      Directory.

      This directory contains source files for overdue letters used within
        Pinnacle.

^1/usr/ver(x)/SCHEMA^6              -      Directory.

      This directory contains source files for database table information.



              ^1Pinnacle Directory layout and key files.^6                Page 3

^1/usr/ver(x)/SPREAD^6              -      Directory.

      This directory contains work files for G/L to spread sheet interface.

^1/usr/ver(x)/TAB^6                 -      Directory.

      This directory contains work files used by Pinnacle screens.

^1/usr/ver(x)/WORK^6                -      Directory.

      This directory contains work files used by Pinnacle.

^1/usr/ver(x)/gl_reports^6          -      Directory.
^1/usr/ver(x)/su_reports^6          -      Directory.

      These directories contains user specific G/L report generator files.
              ^1Pinnacle Directory layout and key files.^6                Page 4

^1/usr/ver(x)/Read_Me^6             -      Text File.

^1/usr/ver(x)/PRINTERx^6            -      Directories.
^1/usr/ver(x)/QUEUE_NAMES^6           

      These directories contain Queue specific print files. 
      NOTE that QUEUE_NAMES are specific to queue's set up by User.

^1/usr/ver(x)/BIN/ACCOUNT^6         -      Directory.

       This directory contains Pinnacle system accounting files.

^1/usr/ver(x)/BIN/AUDIT^6           -      Directory.

       This directory contains Pinnacle master file change audits.

              ^1Pinnacle Directory layout and key files.^6                Page 5

^1/usr/ver(x)/BIN/BM^6              -      Directory.

       This directory contains bill of material / manufacturing programs.

^1/usr/ver(x)/BIN/CR^6              -      Directory.

       This directory contains creditors programs.

^1/usr/ver(x)/BIN/DB^6              -      Directory.

       This directory contains debtors programs.

^1/usr/ver(x)/BIN/FA^6              -      Directory.

       This directory contains fixed asset programs.

              ^1Pinnacle Directory layout and key files.^6                Page 6

^1/usr/ver(x)/BIN/FF^6              -      Directory.

       This directory contains focus forcasting / DRP programs.

^1/usr/ver(x)/BIN/GL^6              -      Directory.

       This directory contains General ledger programs.

^1/usr/ver(x)/BIN/HELP^6            -      Directory.

       This directory contains Help files.

^1/usr/ver(x)/BIN/LICENSE^6         -      Text file.

       This file contains Pinnacle licence information.

              ^1Pinnacle Directory layout and key files.^6                Page 7

^1/usr/ver(x)/BIN/LOG^6             -      Directory.

       This directory contains log files from rebuilds and night processing.

^1/usr/ver(x)/BIN/MENU^6            -      Directory.

       This directory contains menu related programs.

^1/usr/ver(x)/BIN/MENUSYS^6         -      Directory.

       This directory contains menu, terminal and printer related text files.

^1/usr/ver(x)/BIN/MH^6              -      Directory.

       This directory contains machine history programs.

              ^1Pinnacle Directory layout and key files.^6                Page 8

^1/usr/ver(x)/BIN/OL^6              -      Directory.

       This directory contains on-line invoicing programs.


^1/usr/ver(x)/BIN/PINNACLE^6        -      Text file.

       This file contains Pinnacle environment information.

^1/usr/ver(x)/BIN/PO^6              -      Directory.

       This directory contains purchase order programs.

^1/usr/ver(x)/BIN/PRT^6             -      Directory.

       This directory contains print files from Quotations etc.
              ^1Pinnacle Directory layout and key files.^6                Page 9

^1/usr/ver(x)/BIN/PR_FILE^6         -      Directory.

       This directory contains format files for Packing slip lineup etc.

^1/usr/ver(x)/BIN/Post_status^6     -      Text file.

       This file contains invoice status descriptions.

^1/usr/ver(x)/BIN/QT^6              -      Directory.

       This directory contains quotation programs.

^1/usr/ver(x)/BIN/RUN_EODAY^6       -      Shell script.

       This shell script is run when terminal one ( console ) logs out.

              ^1Pinnacle Directory layout and key files.^6                Page 10

^1/usr/ver(x)/BIN/SA^6              -      Directory.

       This directory contains sales analysis programs.

^1/usr/ver(x)/BIN/SECURE^6          -      Text file.

       This file contains User login security information.

^1/usr/ver(x)/BIN/SJ^6              -      Directory.

       This directory contains service job costing information.

^1/usr/ver(x)/BIN/SK^6              -      Directory.

       This directory contains stock control information.

              ^1Pinnacle Directory layout and key files.^6                Page 11

^1/usr/ver(x)/BIN/SK_TRAN_DEL^6     -      Shell script.

       This shell script is run by delete stock transactions and is 
        used for performance reasons. If file does not exists stock 
        transactions delete will still work but may take longer is large 
        volumns are in existance.

^1/usr/ver(x)/BIN/SO^6              -      Directory.

       This directory contains sales order processing programs.

^1/usr/ver(x)/BIN/SUB_MENU^6        -      Directory.

       This directory contains sub-menu text and control files.


              ^1Pinnacle Directory layout and key files.^6                Page 12

^1/usr/ver(x)/BIN/TM^6              -      Directory.

       This directory contains tele-marketing programs.

^1/usr/ver(x)/BIN/TS^6              -      Directory.

       This directory contains tele-sales programs.

^1/usr/ver(x)/BIN/UTILS^6           -      Directory.

       This directory contains Pinnacle system utilities.

^1/usr/ver(x)/BIN/day.sh^6          -      Shell Script.

       This shell script is used for night processing performed during the day.

              ^1Pinnacle Directory layout and key files.^6                Page 13

^1/usr/ver(x)/BIN/inf.glwk.sql^6    -      SQL script.

       This SQL script is used to consolidate G/L stock transaction created 
        from multiple terminals onto one terminal.

^1/usr/ver(x)/BIN/menu^6            -      Program Binary.

       Actual Menu binary linked to MENU/menu.

^1/usr/ver(x)/BIN/monthly.sh^6      -      Shell Script.

       This shell script is used for month end processing.

^1/usr/ver(x)/BIN/night.sh^6        -      Shell Script.

       This shell script is used for night processing.
              ^1Pinnacle Directory layout and key files.^6                Page 14

^1/usr/ver(x)/BIN/night_reports^6   -      Shell Script.

       This shell script is used for running user defined reports overnight.

^1/usr/ver(x)/BIN/perm_night^6      -      Shell Script.

       This shell script is used for running user defined reports every night.

^1/usr/ver(x)/BIN/pformat^6         -      Program Binary.

       Actual Print-formater binary linked to UTILS/pformat.

^1/usr/ver(x)/BIN/ps_menu^6         -      Program binary.

       Actual Multi-view menu binary linked to MENU/menu.

              ^1Pinnacle Directory layout and key files.^6                Page 15

^1/usr/ver(x)/BIN/smenu^6           -      Program binary.

       Actual Sub-Menu binary linked to MENU/menu.

^1/usr/ver(x)/BIN/weekly.sh^6       -      Shell Script.

       This shell script is used for week end processing.


^1/usr/ver(x)/BIN/.cshrc^6          -      Psl standard c-shell
^1/usr/ver(x)/BIN/.logout^6         -      Psl standard bourne shell login
^1/usr/ver(x)/BIN/.profile^6        -      Psl standard bourne shell profile

^1/usr/ver(x)/BIN/DATA/data.dbs^6   -      Directory.

      Database directory holding user data.
              ^1Pinnacle Directory layout and key files.^6                Page 16

^1/usr/ver(x)/BIN/DATA/SPEC_FORM^6  -      Directory.

        Holds special & non standard forms used with informix.

^1/usr/ver(x)/BIN/DATA/FORM^6       -      Directory.

        Holds standard forms used with informix.
