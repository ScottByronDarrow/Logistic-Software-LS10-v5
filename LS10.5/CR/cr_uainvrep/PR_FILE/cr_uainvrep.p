HEAD1A    .ECREDITORS UNAPPROVED INVOICES REPORT
HEAD1C    .EFOR DOCUMENTS WITH INVOICE DATE ON OR BEFORE ^AAAAAAAAAA^
BLANK     .B1
DMC_LN1   =========================================================================================================================================================
DMC_RUL   .R=========================================================================================================================================================
DC_LN1    =========================================================================================================================
DC_RUL    .R=========================================================================================================================
DMC_HD2   |CREDITOR|       CREDITOR  NAME         |CURR |     APPROVED BY    |HOLD|  DOCUMENT     | DOCUMENT | PAYMENT  |O/SEAS CURRENCY|  EXCHANGE |  EQUIVALENT |
DMC_HD3   | NUMBER |                              |CODE |                    |REAS|   NUMBER      |   DATE   | DUE DATE |     VALUE     |    RATE   |  LOC VALUE  |
DC_HD2    |CREDITOR|       CREDITOR NAME          |     APPROVED BY    |HOLD|  DOCUMENT #   | DOCUMENT |   DATE   |      VALUE    |
DC_HD3    | NUMBER |                              |                    |REAS|   NUMBER      |   DATE   | DUE DATE |               |
DMC_DET   | ^AAAAAA^ |^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^| ^AAA^ |^AAAAAAAAAAAAAAAAAAAA^|^AAA^ |^AAAAAAAAAAAAAAA^|^AAAAAAAAAA^|^AAAAAAAAAA^|^MMMMMMMMMMM.MM^ | ^FFFF.FFFF^ |^MMMMMMMMM.MM^ |
DC_DET    | ^AAAAAA^ |^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^AAAAAAAAAAAAAAAAAAAA^|^AAA^ |^AAAAAAAAAAAAAAA^|^AAAAAAAAAA^|^AAAAAAAAAA^|^MMMMMMMMMMM.MM^ |
DMC_CDT   |                                       TOTAL FOR CREDITOR                                                    |^MMMMMMMMMMM.MM^ |           |^MMMMMMMMM.MM^ |
DC_CDT    |                                       TOTAL FOR CREDITOR                                              |^MMMMMMMMMMM.MM^ |
DMC_CRT   |                                       TOTAL FOR COMPANY                                                     |               |           |^MMMMMMMMM.MM^ |
DC_CRT    |                                       TOTAL FOR COMPANY                                                |^MMMMMMMMMMM.MM^ |
DMC_LN2   |-------------------------------------------------------------------------------------------------------------------------------------------------------|
DC_LN2    |-----------------------------------------------------------------------------------------------------------------------|
END_FILE  .EOF
