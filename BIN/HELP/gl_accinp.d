  ^1MAINTAIN GENERAL LEDGER ACCOUNTS^6
    
    This  option provides for the establishment of General Ledger Accounts
    from  the  codes established at the various levels within the  General
    Ledger  structure.   For  example  the   account  code  structure   of
    XX-XXX-XXXX will have individual codes established at level one, level
    two  and  level three.  The account codes for the General Ledger  will
    have an account established at level one being two digits and accounts
    established at level two being five digits and accounts established at
    level three, being the full nine digits.
    
    It  is necessary to first establish the account code at level one then
    level two, then level three.
    


                      ^^GGGGGGGG^^ End of Page 1 ^^GGGGGGG^^
~
    Each account code is assigned a Class, this Class is used to determine
    which accounts are automatically closed at Year End and which accounts
    flow through to the New Year i.e.  Assets and Liability Accounts).
    
    As  part of the Year End procedure all Income and Expense Summary  and
    Posting Accounts are zeroed.  The Nett difference is posted through to
    the Appropriation Account, so that at the beginning of period 1 in the
    New  Year the only accounts with balances are the Liability and  Asset
    Summary and Posting Accounts.
    
    It  is therefore important to assign the correct Class to an  Account.
    The  basic  rules  are that a Posting Level must be the  lowest  level
    account  in  the structure.  A Summary Account cannot be  assigned  or
    cannot be below a Posting Level Account.
    

                      ^^GGGGGGGG^^ End of Page 2 ^^GGGGGGG^^
~
    Non Financial (Statistical) accounts may be created for the purpose of
    linking  Financial  accounts  (at  any Level)  to  the  Non  Financial
    accounts.   The  linking  is done, using the [L]INK option  after  the
    accounts have been established
    
    The  opportunity  is given to amend the account name or to default  to
    the concatenation of the account level descriptions.
    
    The account number prompt will display fields equivalent to the number
    of  account codes plus the number of level breaks; for example In  the
    above example the account number will display eleven fields.  Nine for
    the  account  code and two representing two breaks within the  account
    code structure.
    


                      ^^GGGGGGGG^^ End of Page 3 ^^GGGGGGG^^
~
  The following rules control which Class can be assigned to an account number.
    
    ^1Account Class   Valid Higher Level A/cs     Valid Statistical A/cs^6
         FLS               FLS, CC                          NFS
         FAS               FAS, FLS, CC                     NFS
         FIS               FIS, FLS, CC                     NFS
         FES               FES, FAS, FLS, CC                NFS
         FLP               FLS, CC                          NFS
         FAP               FAS, FLS, CC                     NFS
         FIP               FIS, FLS, CC                     NFS
         FEP               FES, FIS, FAS, FLS, CC           NFS
    
         NFC               N/A
         NFS               NFS, NFC
         NFP               NFS
    
                      ^^GGGGGGGG^^ End of Page 4 ^^GGGGGGG^^
~
Continued...

    ^1Account Class   Valid Higher Level A/cs     Valid Statistical A/cs^6
         FLS               Financial Liability Summary
         FAS               Financial Asset Summary
         FIS               Financial Income Summary
         FES               Financial Expense Summary
         FLP               Financial Liability Posting
         FAP               Financial Asset Posting
         FIP               Financial Income Posting
         FEP               Financial Expense Posting
    
         NFC               Non-Financial Control
         NFS               Non-Financial Summary
         NFP               Non-Financial Posting

^^GGGGGGGG^^ ^1End of Help.^6  Please reference your manuals for more details. ^^GGGGGGG^^
~
