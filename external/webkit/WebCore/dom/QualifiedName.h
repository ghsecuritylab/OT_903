

#ifndef QualifiedName_h
#define QualifiedName_h

#include "AtomicString.h"
#include <wtf/HashTraits.h>

namespace WebCore {

struct QualifiedNameComponents {
    StringImpl* m_prefix;
    StringImpl* m_localName;
    StringImpl* m_namespace;
};

class QualifiedName : public FastAllocBase {
public:
    class QualifiedNameImpl : public RefCounted<QualifiedNameImpl> {
    public:
        static PassRefPtr<QualifiedNameImpl> create(const AtomicString& prefix, const AtomicString& localName, const AtomicString& namespaceURI)
        {
            return adoptRef(new QualifiedNameImpl(prefix, localName, namespaceURI));
        }

        const AtomicString m_prefix;
        const AtomicString m_localName;
        const AtomicString m_namespace;
        mutable AtomicString m_localNameUpper;

    private:
        QualifiedNameImpl(const AtomicString& prefix, const AtomicString& localName, const AtomicString& namespaceURI)
            : m_prefix(prefix)
            , m_localName(localName)
            , m_namespace(namespaceURI)
        {
            ASSERT(!namespaceURI.isEmpty() || namespaceURI.isNull());
        }        
    };

    QualifiedName(const AtomicString& prefix, const AtomicString& localName, const AtomicString& namespaceURI);
    QualifiedName(const AtomicString& prefix, const char* localName, const AtomicString& namespaceURI);
    ~QualifiedName() { deref(); }
#ifdef QNAME_DEFAULT_CONSTRUCTOR
    QualifiedName() : m_impl(0) { }
#endif

    QualifiedName(const QualifiedName& other) : m_impl(other.m_impl) { ref(); }
    const QualifiedName& operator=(const QualifiedName& other) { other.ref(); deref(); m_impl = other.m_impl; return *this; }

    bool operator==(const QualifiedName& other) const { return m_impl == other.m_impl; }
    bool operator!=(const QualifiedName& other) const { return !(*this == other); }

    bool matches(const QualifiedName& other) const { return m_impl == other.m_impl || (localName() == other.localName() && namespaceURI() == other.namespaceURI()); }

    bool hasPrefix() const { return m_impl->m_prefix != nullAtom; }
    void setPrefix(const AtomicString& prefix) { *this = QualifiedName(prefix, localName(), namespaceURI()); }

    const AtomicString& prefix() const { return m_impl->m_prefix; }
    const AtomicString& localName() const { return m_impl->m_localName; }
    const AtomicString& namespaceURI() const { return m_impl->m_namespace; }

    // Uppercased localName, cached for efficiency
    const AtomicString& localNameUpper() const;

    String toString() const;

    QualifiedNameImpl* impl() const { return m_impl; }
    
    // Init routine for globals
    static void init();
    
private:
    void init(const AtomicString& prefix, const AtomicString& localName, const AtomicString& namespaceURI);
    void ref() const { m_impl->ref(); }
    void deref();
    
    QualifiedNameImpl* m_impl;
};

#ifndef WEBCORE_QUALIFIEDNAME_HIDE_GLOBALS
extern const QualifiedName anyName;
inline const QualifiedName& anyQName() { return anyName; }
#endif

inline bool operator==(const AtomicString& a, const QualifiedName& q) { return a == q.localName(); }
inline bool operator!=(const AtomicString& a, const QualifiedName& q) { return a != q.localName(); }
inline bool operator==(const QualifiedName& q, const AtomicString& a) { return a == q.localName(); }
inline bool operator!=(const QualifiedName& q, const AtomicString& a) { return a != q.localName(); }

inline unsigned hashComponents(const QualifiedNameComponents& buf)
{
    ASSERT(sizeof(QualifiedNameComponents) % (sizeof(uint16_t) * 2) == 0);
    
    unsigned l = sizeof(QualifiedNameComponents) / (sizeof(uint16_t) * 2);
    const uint16_t* s = reinterpret_cast<const uint16_t*>(&buf);
    uint32_t hash = WTF::stringHashingStartValue;
    
    // Main loop
    for (; l > 0; l--) {
        hash += s[0];
        uint32_t tmp = (s[1] << 11) ^ hash;
        hash = (hash << 16) ^ tmp;
        s += 2;
        hash += hash >> 11;
    }
    
    // Force "avalanching" of final 127 bits
    hash ^= hash << 3;
    hash += hash >> 5;
    hash ^= hash << 2;
    hash += hash >> 15;
    hash ^= hash << 10;
    
    // this avoids ever returning a hash code of 0, since that is used to
    // signal "hash not computed yet", using a value that is likely to be
    // effectively the same as 0 when the low bits are masked
    if (hash == 0)
        hash = 0x80000000;
    
    return hash;
}
    
struct QualifiedNameHash {
    static unsigned hash(const QualifiedName& name) { return hash(name.impl()); }
    
    static unsigned hash(const QualifiedName::QualifiedNameImpl* name) 
    {    
        QualifiedNameComponents c = { name->m_prefix.impl(), name->m_localName.impl(), name->m_namespace.impl() };
        return hashComponents(c);
    }
    
    static bool equal(const QualifiedName& a, const QualifiedName& b) { return a == b; }
    static bool equal(const QualifiedName::QualifiedNameImpl* a, const QualifiedName::QualifiedNameImpl* b) { return a == b; }
    
    static const bool safeToCompareToEmptyOrDeleted = false;
};
    
}

namespace WTF {
    
    template<typename T> struct DefaultHash;

    template<> struct DefaultHash<WebCore::QualifiedName> {
        typedef WebCore::QualifiedNameHash Hash;
    };
    
    template<> struct HashTraits<WebCore::QualifiedName> : GenericHashTraits<WebCore::QualifiedName> {
        static const bool emptyValueIsZero = false;
        static WebCore::QualifiedName emptyValue() { return WebCore::QualifiedName(WebCore::nullAtom, WebCore::nullAtom, WebCore::nullAtom); }
        static void constructDeletedValue(WebCore::QualifiedName& slot) { new (&slot) WebCore::QualifiedName(WebCore::nullAtom, WebCore::AtomicString(HashTableDeletedValue), WebCore::nullAtom); }
        static bool isDeletedValue(const WebCore::QualifiedName& slot) { return slot.localName().isHashTableDeletedValue(); }
    };
}

#endif
