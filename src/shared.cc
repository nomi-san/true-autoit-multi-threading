#include <atomic>
#include <mutex>
#include <unordered_map>
#include <windows.h>

#if _DEBUG
#include <iostream>
#include <fstream>
#endif

// Simple shared object with hash map.
// Refs:
//   https://github.com/AutoItMicro/AutoItObject
//   https://docs.microsoft.com/en-us/windows/win32/api/oaidl/nn-oaidl-idispatch

class Shared : public IDispatch
{
#if _DEBUG
    UINT32 m_id;
#endif

    std::atomic<ULONG> m_ref;
    std::unordered_map<UINT, VARIANT> m_values;
    std::mutex m_mutex;

    // Empty string value.
    static constexpr VARIANT VAL_EMPTY = { VT_EMPTY };

    // FNV-1a 32-bit.
    static UINT32 getHash(LPCWSTR key, UINT length)
    {
        UINT32 hash = 2166136261U;

        for (UINT i = 0; i < length; i++)
        {
            hash ^= key[i];
            hash *= 16777619;
        }

        return hash;
    }

    bool hasKey(UINT32 key)
    {
        return m_values.size() != 0
            && m_values.find(key) != m_values.end();
    }

    void getValue(UINT32 key, VARIANT *value)
    {
        if (hasKey(key))
        {
            VariantCopy(value, &m_values[key]);
        }
        else
        {
            VariantCopy(value, &VAL_EMPTY);
        }
    }

    void setValue(UINT32 key, const VARIANT *value)
    {
        std::lock_guard<std::mutex> lock{ m_mutex };

        if (hasKey(key))
        {
            VariantCopy(&m_values[key], value);
        }
        else
        {
            VARIANT tmp = VAL_EMPTY;
            VariantCopy(&tmp, value);
            m_values[key] = tmp;
        }
    }

public:
    Shared()
    {
#if _DEBUG
        static std::atomic<UINT32> idCount = 0;
        m_id = ++idCount;

        std::ofstream myfile;
        myfile.open("debug.log", m_id == 1 ? std::iostream::trunc : std::iostream::app);
        myfile << "object " << m_id << " created.\n";
        myfile.close();
#endif

        m_ref = 0;
        m_values.clear();
    }

    ~Shared()
    {
        for (auto &kv : m_values)
        {
            VariantClear(&kv.second);
        }

        m_values.clear();

#if _DEBUG
        std::ofstream myfile;
        myfile.open("debug.log", std::iostream::app);
        myfile << "object " << m_id << " destroyed.\n";
        myfile.close();
#endif
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

        return ULONG(m_ref);
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
        *rgDispId = getHash(*rgszNames, lstrlenW(*rgszNames));
        return S_OK;
    }

    HRESULT STDMETHODCALLTYPE Invoke(DISPID dispIdMember, const IID &riid, LCID lcid, WORD wFlags, 
        DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override
    {
        if ((wFlags & DISPATCH_METHOD) || (wFlags & DISPATCH_PROPERTYGET))
        {
            getValue((UINT32)dispIdMember, pVarResult);
            return S_OK;
        }
        else if ((wFlags & DISPATCH_PROPERTYPUT) || (wFlags & DISPATCH_PROPERTYPUTREF))
        {
            setValue((UINT32)dispIdMember, &pDispParams->rgvarg[0]);
            return S_OK;
        }

        return DISP_E_MEMBERNOTFOUND;
    }
};

LPDISPATCH N_CreateShared()
{
    return new Shared();
}