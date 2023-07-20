#pragma once

// embedmemory.h
// 5/8/2014 jichi
// Shared memory reader.
#include<Windows.h> 
#include<string>
enum { LanguageCapacity = 4 };
struct Cell {
    UINT64 connect;
    UINT64 status;
    UINT64 hash;
    //qint32 signature;
    UINT64 role;
    char language[LanguageCapacity];
    int textSize;
    wchar_t text[1];
private:
    Cell() {} // disable constructor
};
class EmbedMemory_MMAP {
public:
    
    
    Cell* sharedcell;
    HANDLE filemap;
    explicit EmbedMemory_MMAP();
    ~EmbedMemory_MMAP();
    std::wstring key() const;
    void setKey(const std::wstring& v);

    int cellCount() const; // total number of cells
    int cellSize() const; // size for each cell

    ///  The initial size must be larger than 20.
    bool create(int cellSize, int cellCount = 1, bool readOnly = false);
    bool attach(bool readOnly = false);
    bool isAttached() const;

    bool detach();
    bool detach_() { return detach(); } // escape keyword for Shiboken

    bool lock();
    bool unlock();

    bool hasError() const;
    std::wstring errorString() const;

    // Contents

    const char* constData(int i) const; // for debuggong purpose only

    UINT8 dataStatus(int i) const;
    void setDataStatus(int i, UINT8 v);

    UINT64 dataHash(int i) const;
    void setDataHash(int i, UINT64 v);

    //qint32 dataSignature() const;
    //void setDataSignature(qint32 v);

    UINT8 dataRole(int i) const;
    void setDataRole(int i, UINT8 v);

    std::wstring dataLanguage(int i) const;
    void setDataLanguage(int i, const std::wstring& v);

    std::wstring dataText(int i) const;
    void setDataText(int i, const std::wstring& v);
    int dataTextCapacity() const; // number of characters allowed in text
    bool create();

    enum DataStatus {
        EmptyStatus = 0 // There is no data
        , ReadyStatus   // The data is ready to read
        , BusyStatus    // The producer is busy writing the data
        , CancelStatus  // The data requires is cancelled
    };

    bool isDataEmpty(int i) const { return dataStatus(i) == EmptyStatus; }
    bool isDataReady(int i) const { return dataStatus(i) == ReadyStatus; }
    bool isDataBusy(int i) const { return dataStatus(i) == BusyStatus; }
    bool isDataCanceled(int i) const { return dataStatus(i) == CancelStatus; }
}; 
// EOF
