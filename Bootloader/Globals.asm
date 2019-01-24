%define GDTEntry_size   32
%define GDT_READ        2
%define GDT_WRITE       2
%define GDT_EXECUTE     8
%define CODE_SEGMENT    1
%define DATA_SEGMENT    2
    
WelcomeMessage: db "Welcome to KulkarNix",0

%macro NULLGDT_ENTRY 0
    ;; Yea yea could have been done better
    dw  0
    dw  0
    db  0
    db  0
    db  0
    db  0
%endmacro    

%macro  GDT_ENTRY 3
    ;; %1 Base
    ;; %2 Limit
    ;; %3 Access byte
    dw  (%2 & 0ffffh)              ; Limit 0-15 bits 
    dw  (%1 & 0ffffh)              ; Base 0-15
    db  ((%1>>16) & 0ffh)          ; Base 16-23 
    db  ((090h | %3) & 0ffh)       ; Define access byte
    db  (0c0h | ((%2>> 16) & 0fh)) ; Granularity 1 Size 1, 16-19 Limit
    db  ((%1>>24) & 0fh)             ; 24-31 Base
%endmacro
