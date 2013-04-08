#ifndef BMD_MAC_H
#define BMD_MAC_H BMD_MAC_H

typedef void* MacStuff;

//Inits mac-specific stuff when compiled on os x
MacStuff* initMac();

void exitMac(MacStuff* s);

#endif //BMD_MAC_H
