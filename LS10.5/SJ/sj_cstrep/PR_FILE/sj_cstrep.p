HED1      .ESERVICE ORDER JOB COST DETAILS 
HED2      Service Order No. ^LLLLLL^                  Customer Order No. ^AAAAAAAAAA^
HED3      Charge Customer  ^AAAAAA^                   For Customer  ^AAAAAA^                      Order Date: ^AAAAAAAAAA^
HED4      ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^  ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^  Req'd Date: ^AAAAAAAAAA^
HED5      ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^  ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^
HED6      Contact ^AAAAAAAAAAAAAAAAAAAA^              Status : ^A^ - ^AAAAAAAAAAAAAAAAAAAAAAAA^   
          
SKIP      .B^II^

ULINE     -----------------------------------------------------------------------------------------------------------
JD1       Job Description:
JD2                          ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^     

SD1       Service Details:
          
SP1       | Material / Costs |                                     |          |       |     |  Extend   |  Extend   |
SP2       |       Code       |             Description             |   Date   |  Qty  | Uom |   Cost    |  Charge   |
SP3       |------------------|-------------------------------------|----------|-------|-----|-----------|-----------|

SP4       | ^AAAAAAAAAAAAAAAA^ | ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ |^AAAAAAAAAA^|^FFFFF.F^| ^AAA^ | ^FFFFFF.FF^ | ^FFFFFF.FF^ |
SP5       | Totals           |                                     |          |       |     | ^FFFFFF.FF^ | ^FFFFFF.FF^ |


LD1       |   Labour    |          |  1.0 t |  1.5 t |  2.0 t |  Cost   | O'head  | Profit  |  Extend   |  Extend   |
LD2       | Serv Person |   Date   |  Hrs   |  Hrs   |  Hrs   |  Rate   |  Rate   |  Rate   |   Cost    |  Charge   |
LD3       |-------------|----------|--------|--------|--------|---------|---------|---------|-----------|-----------|
LD4       | ^AAAAAAAAAAA^ |^AAAAAAAAAA^| ^FFF.FF^ | ^FFF.FF^ | ^FFF.FF^ | ^FFFF.FF^ | ^FFFF.FF^ | ^FFFF.FF^ | ^FFFFFF.FF^ | ^FFFFFF.FF^ |
LD5       | Totals      |          | ^FFF.FF^ | ^FFF.FF^ | ^FFF.FF^ |         |         |         | ^FFFFFF.FF^ | ^FFFFFF.FF^ |

KM1       |      Travel      |          |            |                            |         |   Extend  |  Extend   |
KM2       |  Service Person  |   Date   |  Mileage   |                            |  Rate   |   Cost    |  Charge   |
KM3       |------------------|----------|------------|----------------------------|---------|-----------|-----------|
KM4       | ^AAAAAAAAAAAAAAAA^ |^AAAAAAAAAA^| ^FFFFF.FF^ km|                            | ^FFFF.FF^ | ^FFFFFF.FF^ | ^FFFFFF.FF^ |
KM5       | Totals           |          | ^FFFFF.FF^ km|                            |         | ^FFFFFF.FF^ | ^FFFFFF.FF^ |


DC1       |              |         Outside Purchases         |          |          |Invoice |  Extend   |  Extend   |
DC2       |  P/order No  |            Description            |   Date   | Supplier |   No   |   Cost    |  Charge   |
DC3       |--------------|-----------------------------------|----------|----------|--------|-----------|-----------|
DC4       | ^AAAAAAAAAAAA^ | ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ |^AAAAAAAAAA^|  ^AAAAAA^  |^AAAAAAAA^| ^FFFFFF.FF^ | ^FFFFFF.FF^ |
DC5       |  Totals      |                                   |          |          |        | ^FFFFFF.FF^ | ^FFFFFF.FF^ |

TOT1      |  Service Job Totals                                                             | ^FFFFFF.FF^ | ^FFFFFF.FF^ |

EOF       .EOF
