ReadSector:
    ;; using int 13h, 02h currently since the image will be conatined in a floppy disk
    ;; TODO: Add support for extended calls
    ;; Arguments
    ;; dl -> drive letter
    ;; cl -> sector number
    ;; bx -> adress to load the data
    mov     ax, 0201h           ; service rountine, number of sectors to read
    int     13h
    ;; This seems pointless but thats beacuse its only for floppy disks, to support harddisk/CDROMS we need a lot of stuff here
.end:
    ret
