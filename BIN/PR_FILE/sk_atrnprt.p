.PL44
.14
.NF
.PI12
.L134
.B5
                                                                         ISSUING   BRANCH  NO .ISS_BR_NUMB                 .TR_MESSAGES
                                                                         .ISS_BR_NAME .TR_MESSAGE2
                                                                                                                 .TRAN_N_DATE .TME_ENTERED
            RECEIVING BRANCH  NO .REC_BR_NUMB
            .REC_BR_NAME                                                               .DOCKET_NUMB
            .REC_BR_ADR1
            .REC_BR_ADR2                                                               .PAGE_NUMBER
            .REC_BR_ADR3
.B3
     ITEM NUMBER                 DESCRIPTION                        QTY ORDERED   QTY SUPPLIED         QTY B/O     LOCATION 
.FORMAT_LN_1       .LN_ITM_NUMB .LN_ITM_NAME .LN_QT_ORDER .LN_QT_SUPPY  .LN_QT_BORDR      .LN_LOCATION
.DF3
.B6
                     Order Ref: .TR_TRAN_REF
.B6
.END_USER

===============================================================================
| .COMMAND      |              DESCRIPTION OF DOT COMMAND                     |
===============================================================================
| .ISS_CO_NUMB	| Issuing company number.                                     |
| .ISS_CO_NAME	| Issuing company name.                                       |
| .ISS_CO_SHRT	| Issuing company short name.                                 |
| .ISS_CO_ADR1	| Issuing company address part one.                           |
| .ISS_CO_ADR2	| Issuing company address part two.                           |
| .ISS_CO_ADR3	| Issuing company address part three.                         |
| .REC_CO_NUMB	| Receiving company number.                                   |
| .REC_CO_NAME	| Receiving company name.                                     |
| .REC_CO_SHRT	| Receiving company short name.                               |
| .REC_CO_ADR1	| Receiving company address part one.                         |
| .REC_CO_ADR2	| Receiving company address part two.                         |
| .REC_CO_ADR3	| Receiving company address part three.                       |
| .ISS_BR_NUMB	| Issuing branch number.                                      |
| .ISS_BR_NAME	| Issuing branch name.                                        |
| .ISS_BR_SHRT	| Issuing branch short name.                                  |
| .ISS_BR_ADR1	| Issuing branch address part one.                            |
| .ISS_BR_ADR2	| Issuing branch address part two.                            |
| .ISS_BR_ADR3	| Issuing branch address part three.                          |
| .REC_BR_NUMB	| Receiving branch number.                                    |
| .REC_BR_NAME	| Receiving branch name.                                      |
| .REC_BR_SHRT	| Receiving branch short name.                                |
| .REC_BR_ADR1	| Receiving branch address part one.                          |
| .REC_BR_ADR2	| Receiving branch address part two.                          |
| .REC_BR_ADR3	| Receiving branch address part three.                        |
| .ISS_WH_NUMB	| Issuing warehouse number                                    |
| .ISS_WH_NAME	| Issuing warehouse name.                                     |
| .ISS_WH_SHRT	| Issuing warehouse short name.                               |
| .REC_WH_NUMB	| Receiving warehouse number.                                 |
| .REC_WH_NAME	| Receiving warehouse name.                                   |
| .REC_WH_SHRT	| Receiving warehouse short name.                             |
| .LN_ITM_NUMB	| Line item number.                                           |
| .LN_ITM_NAME	| Line item description.                                      |
| .LN_QT_ORDER	| Line quantity ordered.                                      |
| .LN_QT_BORDR	| Line quantity back ordered.                                 |
| .LN_QT_SUPPY	| Line quantity supplied.                                     |
| .LN_SER_NUMB	| Line item serial number.                                    |
| .LN_LOCATION	| Line item location.                                         |
| .TRAN_N_DATE	| Transfer Date.  ( dd/mm/yy )                                |
| .TRAN_M_DATE	| Current module date ( STOCK ) ( dd/mm/yy )                  |
| .TRAN_C_DATE	| Current system date ( dd/mm/yy )                            |
| .DOCKET_NUMB	| Delivery docket number.                                     |
| .PAGE_NUMBER	| Page number.                                                |
| .FORMAT_LN_1	| Line item format line.                                      |
| .FORMAT_LN_2	| Line item format line 2, User for serial items.             |
| .TR_COMMENTS  | Transfer comments					      |
| .TR_MESSAGES  | Transfer Message line, i.e Reprint etc etc .	              |
| .TR_CAR_CODE  | Carrier Code                                  	      |
| .FRGHT_VALUE  | Freight Value.                                	      |
| .FRT_TOT_WGT  | Freight Total Weight.                         	      |
| .FRT_CONS_NO  | Freight Consignment Number.                   	      |
| .FRT_CARTONS  | Number Of Cartons.                            	      |
| .LINE_SPACE1  | Set single spacing.                           	      |
| .LINE_SPACE2  | Set double spacing.                           	      |
| .LINE_SPACE3  | Set triple spacing.                           	      |
===============================================================================


HMARGIN   .B1
TITLE1    .EPARTS TRANSFER
MARGIN1   .B1
TITLE2                                                          From : ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^      DOCKET  ^AAAAAA^
MARGIN2   .B1

DATE                                                                                                                           ^DD/DD/DD^
MARGIN3   .B1
REC_BRANCH        ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^
MARGIN4   .B1
PAGE_NO                                                                                                                        ^III^
MARGIN5   .B4
COL_HEAD          Item No.                   Item Description              Quantity
MARGIN6   .B0
TR_LINE       ^AAAAAAAAAAAAAAAA^  ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^  ^FFFFFF.FF^
SER_LINE                          ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ 
VBLE_BLANK.B^I^
MARGIN7   .B6
TAIL1                        FROM :                               TO :                                                       ^AAAAAAAAAAAAA^
TAIL2                           ^AAAAAAAAAAAAAAAAAAAAAAAAA^            ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^
NEXT_PAGE .B6
