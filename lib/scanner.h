#ifndef __SCANNER_H
#define __SCANNER_H

typedef void (scan_handler)(unsigned long);
void scanner_init(scan_handler *handler);

#endif /* __SCANNER_H */