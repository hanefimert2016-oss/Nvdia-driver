/*
 * stubs/IOKit/IOService.h
 * Minimal IOKit stubs for syntax-checking on Linux CI.
 */
#ifndef _IOKIT_IOSERVICE_H_STUB
#define _IOKIT_IOSERVICE_H_STUB

#include <stdint.h>
#include <stddef.h>

typedef int32_t  SInt32;
typedef uint32_t IOReturn;
typedef uint32_t IOOptionBits;

#define kIOReturnSuccess     0
#define kIOReturnBadArgument 0xE00002C2
#define kIOReturnNoMemory    0xE00002BD
#define kIOReturnNotReady    0xE00002C9

/* OSObject base */
class OSObject {
public:
    virtual void retain()  {}
    virtual void release() {}
    virtual void free()    {}
    virtual ~OSObject() {}
};

/* OSDictionary stub */
class OSDictionary : public OSObject {};

/* OSString stub */
class OSString : public OSObject {};

/* OSDynamicCast – macro form: OSDynamicCast(Type, ptr)
 * Real IOKit uses metaclass RTTI; here we use static_cast for syntax checking. */
#define OSDynamicCast(T, obj) (static_cast<T *>(obj))

#define OSSafeReleaseNULL(p) do { if (p) { (p)->release(); (p) = nullptr; } } while(0)

/* OSDefineMetaClassAndStructors – generates boilerplate on real IOKit */
#define OSDeclareDefaultStructors(cls)  \
public:                                  \
    cls() = default;                     \
    virtual ~cls() = default;

#define OSDefineMetaClassAndStructors(cls, super) /* stub – no-op */

class IOService : public OSObject {
public:
    virtual bool       init(OSDictionary * = nullptr) { return true; }
    virtual IOService *probe(IOService *, SInt32 *)    { return this; }
    virtual bool       start(IOService *)              { return true; }
    virtual void       stop(IOService *)               {}
    virtual bool       attach(IOService *)             { return true; }
    virtual void       detach(IOService *)             {}
    virtual void       registerService()               {}
    IOService         *getProvider() const             { return nullptr; }
};

#endif /* _IOKIT_IOSERVICE_H_STUB */
