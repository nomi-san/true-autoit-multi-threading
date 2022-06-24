#include <windows.h>

// Simple shared object,
// wrapped around Crafting Interpreter hash table (http://www.craftinginterpreters.com/hash-tables.html)
// and IDispatch interface (https://docs.microsoft.com/en-us/windows/win32/api/oaidl/nn-oaidl-idispatch).
// Refs
//   https://github.com/AutoItMicro/AutoItObject
//   https://github.com/nomi-san/mp.au3/blob/8e74a590943887f1ff035dadf246674561e617f3/mp.cc#L285

class Entry
{
public:
    const UINT32 Key = 0;
    const VARIANT Value = { VT_NULL };

    ~Entry()
    {
        VariantClear(const_cast<VARIANT *>(&Value));
    }

    void Set(UINT32 key, const VARIANT *value)
    {
        const_cast<UINT &>(Key) = key;
        VariantCopy(const_cast<VARIANT *>(&Value), value);
    }

    bool IsNull() const
    {
        return Value.vt == VT_NULL;
    }
};

class Shared : public IDispatch
{
    ULONG m_ref = 0;
    Entry *m_entries;
    int m_count, m_capacity;

    static constexpr VARIANT Null = { VT_NULL };

    // FNV-1a 32
    static UINT32 FnvHash(LPCWSTR key, UINT length)
    {
        UINT32 hash = 2166136261U;

        for (UINT i = 0; i < length; i++)
        {
            hash ^= key[i];
            hash *= 16777619;
        }

        return hash;
    }

public:
    Shared()
    {
        m_count = 0;
        m_capacity = 0;
        m_entries = nullptr;
    }

    ~Shared()
    {
        if (m_entries != nullptr)
        {
            delete[] m_entries;
            m_entries = nullptr;
        }

        m_count = 0;
        m_capacity = 0;
    }

    ULONG STDMETHODCALLTYPE AddRef() override
    {
        // Increase ref count.
        return ++m_ref;
    }

    ULONG STDMETHODCALLTYPE Release() override
    {
        // Decrease ref count.
        if (--m_ref == 0)
        {
            // Self release.
            delete this;
            return 0;
        }
        return m_ref;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(const IID &riid, void **ppvObject) override
    {
        if (riid == IID_IUnknown || riid == IID_IDispatch)
        {
            this->AddRef();
            *ppvObject = this;
            return S_OK;
        }
        return E_NOINTERFACE;
    }

    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(UINT *pctinfo) override
    {
        if (pctinfo) *pctinfo = 0;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) override
    {
        if (ppTInfo) *ppTInfo = NULL;
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE GetIDsOfNames(const IID &riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) override
    {
        *rgDispId = FnvHash(*rgszNames, cNames);
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, const IID &riid, LCID lcid, WORD wFlags, 
        DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override
    {
        if ((wFlags & DISPATCH_METHOD) || (wFlags & DISPATCH_PROPERTYGET))
        {
            Get((UINT32)dispIdMember, pVarResult);
            return S_OK;
        }
        else if ((wFlags & DISPATCH_PROPERTYPUT) || (wFlags & DISPATCH_PROPERTYPUTREF))
        {
            Set((UINT32)dispIdMember, &pDispParams->rgvarg[0]);
            return S_OK;
        }

        return DISP_E_MEMBERNOTFOUND;
    }

private:
    void Get(UINT32 key, VARIANT *value)
    {
        if (m_count != 0)
        {
            Entry *entry = FindEntry(m_entries, m_capacity, key);
            if (entry->Key != 0)
            {
                VariantCopy(value, &entry->Value);
                return;
            }
        }

        VariantCopy(value, &Null);
    }

    void Set(UINT32 key, const VARIANT *value)
    {
        if (m_count >= m_capacity * 0.75)
        {
            Expand((m_capacity < 8) ? 8 : (m_capacity * 2));
        }

        Entry *entry = FindEntry(m_entries, m_capacity, key);

        bool isNewKey = entry->Key == 0;
        if (isNewKey && entry->IsNull())
        {
            m_count++;
        }

        entry->Set(key, value);
    }

    static Entry *FindEntry(Entry *entries, int capacity, UINT32 key)
    {
        int capMask = capacity - 1;
        UINT32 index = key & capMask;
        Entry *tombstone = nullptr;

        for (;;)
        {
            Entry *entry = &entries[index];

            if (entry->Key == 0)
            {
                if (entry->IsNull())
                {
                    return tombstone != nullptr ? tombstone : entry;
                }
                else
                {
                    if (tombstone == nullptr) tombstone = entry;
                }
            }
            else if (entry->Key == key)
            {
                return entry;
            }

            index = (index + 1) & capMask;
        }
    }

    void Expand(int capacity)
    {
        Entry *entries = new Entry[capacity];

        m_count = 0;
        for (int i = 0; i < m_capacity; i++)
        {
            Entry *entry = &m_entries[i];
            if (entry->Key == 0) continue;

            Entry *dest = FindEntry(entries, capacity, entry->Key);
            dest->Set(entry->Key, &entry->Value);
            m_count++;
        }

        this->~Shared();
        m_capacity = capacity;
        m_entries = entries;
    }
};

LPDISPATCH N_CreateShared()
{
    return new Shared();
}