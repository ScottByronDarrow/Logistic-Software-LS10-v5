HEADER    =========================================================================================================================================================
RULEOFF   .R=========================================================================================================================================================
HEADER1   |                |                                        | <-------------- MTD SALES -----------------> | <--------------- YTD SALES ----------------> |
AHEADER   | CATEGORY NO /  |           CATEGORY DESCRIPTION /       | <-------------- MTD SALES -----------------> | <--------------- YTD SALES ----------------> |
THEADER   | CUST  TYPE /   |          CUST  TYPE DESCRIPTION /      | <-------------- MTD SALES -----------------> | <--------------- YTD SALES ----------------> |
SHEADER   | SALESMAN NO /  |               SALESMAN  NAME  /        | <-------------- MTD SALES -----------------> | <--------------- YTD SALES ----------------> |
HEADER2   | ITEM NUMBER    |            ITEM     DESCRIPTION        |    QTY     |   SALES    |    COST    |%MARGIN|    QTY     |   SALES    |    COST    |%MARGIN|
HEADER3   |   CUSTOMER     |            CUSTOMER   NAME             |    QTY     |   SALES    |    COST    |%MARGIN|    QTY     |   SALES    |    COST    |%MARGIN|
HEADER4   |ITEM NO/CUSTOMER|    ITEM DESCRIPTION / CUSTOMER NAME    |    QTY     |   SALES    |    COST    |%MARGIN|    QTY     |   SALES    |    COST    |%MARGIN|
HEADER5   |CUSTOMER/ITEM NO|    CUSTOMER NAME / ITEM DESCRIPTION    |    QTY     |   SALES    |    COST    |%MARGIN|    QTY     |   SALES    |    COST    |%MARGIN|
SEPARATOR |----------------|----------------------------------------|------------|------------|------------|-------|------------|------------|------------|-------|
BLANKLINE |                                                                                                                                                       |
PROD_DET  |^AAAAAAAAAAAAAAAA^|^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^AAAAAAA^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^AAAAAAA^|
SUB_TOT   | ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ |^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^AAAAAAA^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^AAAAAAA^|

XHEADER   ===============================================================================================================
XRULEOFF  .R===============================================================================================================
XHEADER1  |                |                                        | <----- MTD SALES -----> | <----- YTD SALES -----> |
XAHEADER  | CATEGORY NO /  |           CATEGORY DESCRIPTION /       | <----- MTD SALES -----> | <----- YTD SALES -----> |
XTHEADER  | CUST  TYPE /   |          CUST  TYPE DESCRIPTION /      | <----- MTD SALES -----> | <----- YTD SALES -----> |
XSHEADER  | SALESMAN NO /  |               SALESMAN  NAME  /        | <----- MTD SALES -----> | <----- YTD SALES -----> |
XHEADER2  | ITEM NUMBER    |            ITEM     DESCRIPTION        |    QTY     |   SALES    |    QTY     |   SALES    |
XHEADER3  |   CUSTOMER     |            CUSTOMER   NAME             |    QTY     |   SALES    |    QTY     |   SALES    |
XHEADER4  |ITEM NO/CUSTOMER|    ITEM DESCRIPTION / CUSTOMER NAME    |    QTY     |   SALES    |    QTY     |   SALES    |
XHEADER5  |CUSTOMER/ITEM NO|    CUSTOMER NAME / ITEM DESCRIPTION    |    QTY     |   SALES    |    QTY     |   SALES    |
XSEPARATOR|----------------|----------------------------------------|------------|------------|------------|------------|
XBLANKLINE|                                                                                                             |
XPROD_DET |^AAAAAAAAAAAAAAAA^|^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|
XSUB_TOT  | ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ |^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|


YHEADER   ===============================================================================================================
YRULEOFF  .R===============================================================================================================
YHEADER1  |                |                                        | <-------------- MTD SALES ----------------------> |
YHEADER2  | ITEM NUMBER    |            ITEM     DESCRIPTION        |    QTY     |   SALES    |    COST    |  %MARGIN   |
YHEADER4  |ITEM NO/CUSTOMER|    ITEM DESCRIPTION / CUSTOMER NAME    |    QTY     |   SALES    |    COST    |  %MARGIN   |
YSEPARATOR|----------------|----------------------------------------|------------|------------|------------|------------|
YPROD_DET |^AAAAAAAAAAAAAAAA^|^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|
YSUB_TOT  | ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ |^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|


ZHEADER  ====================================================================================
ZRULEOFF .R====================================================================================
ZHEADER1 |                |                                        | <----- MTD SALES -----> |
ZHEADER2 | ITEM NUMBER    |            ITEM     DESCRIPTION        |    QTY     |   SALES    |
XHEADER4 |ITEM NO/CUSTOMER|    ITEM DESCRIPTION / CUSTOMER NAME    |    QTY     |   SALES    |
ZSEPARATOR|----------------|----------------------------------------|------------|------------|
ZPROD_DET |^AAAAAAAAAAAAAAAA^|^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|
ZSUB_TOT  | ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ |^FFFFFFFFF.FF^|^FFFFFFFFF.FF^|
