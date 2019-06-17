RULEOFF   .R========================================================================================================================================================
RULER     ========================================================================================================================================================
HEAD1     |CUSTOMER| P/F INV.|             CUSTOMER  NAME             | INVOICE  |   GROSS    |  DISCOUNT  |    NETT    |FREIGHT+INSU|   ^AAAAAA^   |  P/F INVOICE |
HEAD2     | NUMBER | NUMBER. |                                        |   DATE   |   AMOUNT   |   AMOUNT   |   AMOUNT   |+OTHER AMTS.|   AMOUNT   |    AMOUNT    |
HEAD3     |--------|---------|----------------------------------------|----------|------------|------------|------------|------------|------------|--------------|
LINE1     | ^AAAAAA^ |^AAAAAAAA^ |^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^AAAAAAAAAA^| ^FFFFFFF.FF^ | ^FFFFFFF.FF^ | ^FFFFFFF.FF^ | ^FFFFFFF.FF^ | ^FFFFFFF.FF^ | ^FFFFFFFFF.FF^ |
TAIL1     |========|=========|========================================|==========|============|============|============|============|============|==============|
CUST_TOT  | ^AAAAAAAAAAAAAAAAAAAAAAAAA^                                 |          | ^FFFFFFF.FF^ | ^FFFFFFF.FF^ | ^FFFFFFF.FF^ | ^FFFFFFF.FF^ | ^FFFFFFF.FF^ | ^FFFFFFFFF.FF^ |


RULEOFF1  .R=========================================================================================================================================================
RULER1    =========================================================================================================================================================
HEAD11    |  ITEM NUMBER.  |        DESCRIPTION.         |QUANTITY| P/F INV| CUSTOMER | INVOICE  | EXTENDED | EXTENDED | DISCOUNT |   NET    |  ^AAAAAA^ | EXTENDED |
HEAD12    |                |                             |SUPPLIED| NUMBER | ACRONYM  |  DATE    |  TOTAL.  |   COST.  | AMOUNT.  | EXTENDED |  VALUE. |  AMOUNT  |
HEAD13    |----------------|-----------------------------|--------|--------|----------|----------|----------|----------|----------|----------|---------|----------|
GP_LINE   | Group : ^A^ ^AAAAAAAAAAA^                        |        |        |          |          |          |          |          |         |          |          |
COLN_LINE |^AAAAAAAAAAAAAAAA^|^AAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^FFFFF.FF^| ^AAAAAA^ |^AAAAAAAAA^ |^AAAAAAAAAA^|^FFFFFFF.FF^|^FFFFFFF.FF^|^FFFFFFF.FF^|^FFFFFFF.FF^|^FFFFFF.FF^|^FFFFFFF.FF^|
TOT_LINE  | ^AAAAAAAAAAAAAAAAAAAAAAAAA^                    |^FFFFF.FF^|        |          |          |^FFFFFFF.FF^|^FFFFFFF.FF^|^FFFFFFF.FF^|^FFFFFFF.FF^|^FFFFFF.FF^|^FFFFFFF.FF^|
