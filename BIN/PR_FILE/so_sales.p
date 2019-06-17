RULEOFF   .R====================================================================================================================================================
LINE0     ====================================================================================================================================================
HEADER1   !   DATE   ! CUSTOMER  !CUSTOMER!            CUSTOMER NAME                 !    ITEM NUMBER   !    QTY    !    LOCAL    !   EXTENDED   ! MARGIN%%  ! 
HEADER2   ! OF ORDER !  ACRONYM  ! NUMBER !                                          !                  !           !    VALUE    !    VALUE     !           !
LINE1     !----------!-----------!--------!------------------------------------------!------------------!-----------!-------------!--------------!-----------! 
BEGIN_CUST! WEEK BEGINNING (^DD/DD/DDDD^)   !                                          !                  !           !             !              !           ! 
B_LINE    !          !           !        !                                          !                  !           !             !              !           ! 
ORD_LINE  !^DD/DD/DDDD^! ^AAAAAAAAA^ ! ^AAAAAA^ ! ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ ! ^AAAAAAAAAAAAAAAA^ ! ^FFFFFFF.FF^! ^FFFFFFFFF.FF^!  ^FFFFFFFFF.FF^! ^FFFFF.FF^  ! 
TOT_LINE  ! ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^                       !                  ! ^FFFFFFF.FF^!             ! ^FFFFFFFFFF.FF^! ^FFFFF.FF^  ! 

RULEOFF_S .R=======================================================================================================
LINE_0S   =======================================================================================================
HEADER1_S !   DATE   !CUSTOMER !CUSTOMER!             CUSTOMER NAME                !   EXTENDED   ! MARGIN %%   ! 
HEADER2_S ! OF ORDER ! ACRONYM ! NUMBER !                                          !    VALUE     !             ! 
LINE_1S   !----------!---------!--------!------------------------------------------!--------------!-------------! 
BEGIN_CS  ! WEEK BEGINNING (^DD/DD/DDDD^) !                                          !              !             ! 
B_LINE_S  !          !         !        !                                          !              !             !
ORD_LINE_S!^DD/DD/DDDD^!^AAAAAAAAA^! ^AAAAAA^ ! ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ ! ^FFFFFFFFF.FF^ ! ^FFFFF.FF^    ! 
TOT_LINE_S! ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^                     ! ^FFFFFFFFF.FF^ ! ^FFFFF.FF^    ! 
WEEK_TOT_S! TOTAL FOR WEEK ENDING (^DD/DD/DDDD^)                                     !              ! ^FFFFF.FF^    ! 

HD_ITEM1  !   DATE   !    ^AAAAAAAA^      !    ^AAAAAAAA^  DESCRIPTION                 !      CUSTOMER      !    QTY    !    LOCAL    !   EXTENDED   !  MARGIN%% ! 
HD_ITEM2  ! OF ORDER !     NUMBER       !                                          !      ACRONYM       !           !    VALUE    !    VALUE     !           !  
LINE_ITEM !----------!------------------!------------------------------------------!--------------------!-----------!-------------!--------------!-----------! 
BEGIN_ITEM! WEEK BEGINNING (^DD/DD/DDDD^) !                                          !                    !           !             !              !           ! 
B_ITEM    !          !                  !                                          !                    !           !             !              !           ! 
ITEM_LINE !^DD/DD/DDDD^! ^AAAAAAAAAAAAAAAA^ ! ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ !     ^AAAAAAAAA^      ! ^FFFFFFF.FF^! ^FFFFFFFFF.FF^!  ^FFFFFFFFF.FF^! ^FFFFF.FF^  ! 
TOT_ITEM  ! ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^                     !                    ! ^FFFFFFF.FF^!             ! ^FFFFFFFFFF.FF^! ^FFFFF.FF^  ! 

HD_ITEM_S1!   DATE   !    ^AAAAAAAA^      !    ^AAAAAAAA^  DESCRIPTION                 !   EXTENDED   !  MARGIN %%   ! 
HD_ITEM_S2! OF ORDER !     NUMBER       !                                          !    VALUE     !             !  
LINE_ITEMS!----------!------------------!------------------------------------------!--------------!-------------! 
BEGIN_IS  ! WEEK BEGINNING (^DD/DD/DDDD^) !                                          !              !             ! 
B_ITEM_S  !          !                  !                                          !              !             ! 
ITEM_LINES!^DD/DD/DDDD^! ^AAAAAAAAAAAAAAAA^ ! ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^ ! ^FFFFFFFFF.FF^ !  ^FFFFF.FF^   ! 
TOT_ITEM_S! ^AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA^                     ! ^FFFFFFFFF.FF^ !  ^FFFFF.FF^   ! 

