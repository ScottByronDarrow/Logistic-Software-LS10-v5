HEAD1A    .ESUPPLIERS DETAILED CASH COMMITMENTS REPORT
HEAD1B    .ESUPPLIERS SUMMARY CASH COMMITMENTS REPORT
HEAD1D    .ESORTED IN SUPPLIERS ^AAAAAAA^ ORDER
BLANK     .B1
DMC_LN1   =========================================================================================================================
DMC_RUL   .R=========================================================================================================================
DC_LN1    ===============================================================================================
DC_RUL    .R===============================================================================================
DMC_HD2   |SUPPLIER|   SUPPLIER  NAME             |  DOCUMENT     | DOCUMENT | PAYMENT  |O/SEAS CURRENCY|  EXCHANGE |  EQUIVALENT |
DMC_HD3   | NUMBER |                              |   NUMBER      |   DATE   | DUE DATE |     VALUE     |    RATE   |  LOC VALUE  |
DC_HD2    |SUPPLIER|   SUPPLIER NAME              |  DOCUMENT #   | DOCUMENT |   DATE   |      VALUE    |
DC_HD3    | NUMBER |                              |   NUMBER      |   DATE   | DUE DATE |               |
DMC_CUR   | CURRENCY: ^AAA^                         |               |          |          |               |           |             |
DC_CUR    | CURRENCY: ^AAA^                         |               |          |          |               |
DMC_DET   | ^AAAAAA^ |^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^AAAAAAAAAAAAAAA^|^DD/DD/DDDD^|^DD/DD/DDDD^|^MMMMMMMMMMM.MM^ | ^FFFF.FFFF^ |^MMMMMMMMM.MM^ |
DC_DET    | ^AAAAAA^ |^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^AAAAAAAAAAAAAAA^|^DD/DD/DDDD^|^DD/DD/DDDD^|^MMMMMMMMMMM.MM^ |
DMC_CDT   |                                        TOTAL FOR SUPPLIER                   |^MMMMMMMMMMM.MM^ |           |^MMMMMMMMM.MM^ |
DC_CDT    |                                        TOTAL FOR SUPPLIER                   |^MMMMMMMMMMM.MM^ |
DMC_CRT   |                                        TOTAL FOR CURRENCY                   |^MMMMMMMMMMM.MM^ |           |^MMMMMMMMM.MM^ |
DC_CRT    |                                        TOTAL FOR COMPANY                    |^MMMMMMMMMMM.MM^ |
DMC_LN2   |-----------------------------------------------------------------------------------------------------------------------|
DC_LN2    |---------------------------------------------------------------------------------------------|
SMC_LN1   =======================================================================
SMC_RUL   .R=======================================================================
SC_LN1    =========================================================
SC_RUL    .R=========================================================
SMC_HD2   |SUPPLIER|   SUPPLIER  NAME             |O/SEAS CURRENCY|  EQUIVALENT |
SMC_HD3   | NUMBER |                              |     VALUE     |  LOC VALUE  |
SC_HD2    |SUPPLIER|   SUPPLIER  NAME             |      VALUE    |
SC_HD3    | NUMBER |                              |               |
SMC_CUR   | CURRENCY: ^AAA^                         |               |             |
SMC_CDT   | ^AAAAAA^ |^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^MMMMMMMMMMM.MM^ |^MMMMMMMMM.MM^ |
SC_CDT    | ^AAAAAA^ |^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^|^MMMMMMMMMMM.MM^ |
SMC_CRT   |     TOTAL FOR CURRENCY                |^MMMMMMMMMMM.MM^ |^MMMMMMMMM.MM^ |
SC_CRT    |     TOTAL FOR COMPANY                 |^MMMMMMMMMMM.MM^ |
SMC_LN2   |---------------------------------------------------------------------|
SC_LN2    |-------------------------------------------------------|
END_FILE  .EOF
