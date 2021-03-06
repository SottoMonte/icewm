#include "config.h"
#include "udir.h"
#include "base.h"
#include <dirent.h>
#include <stdlib.h>

class DirPtr {
private:
    bool own;
    DIR *ptr;
    struct dirent *de;

    void open(const char *path) { ptr = opendir(path); own = true; }
    bool read() { return (de = readdir(ptr)) != nullptr; }
    bool dots() const { return '.' == *name(); }

public:
    DirPtr(const char * path) : own(false), ptr(nullptr), de(nullptr) { open(path); }
    DirPtr(upath path) : own(false), ptr(nullptr), de(nullptr) { open(path.string()); }
    DirPtr(void *vp) : own(false), ptr(static_cast<DIR*>(vp)), de(nullptr) { }

    ~DirPtr() { if (own) close(); }
    void close() { if (ptr) { closedir(ptr); ptr = nullptr; own = false; de = nullptr; } }

    operator DIR *() const { return ptr; }

    char* name() const { return de->d_name; }
    int length() const { return int(strlen(name())); }
    int size() const { return 1 + length(); }
    struct dirent *next() {
        while (read() && dots());
        return de;
    }
    void rewind() const { rewinddir(ptr); }
};

cdir::cdir(const char* path)
    : fPath(path), impl(nullptr)
{
    if (path) {
        open();
    }
}

void cdir::close() {
    if (impl) {
        DirPtr(impl).close();
        impl = nullptr;
    }
}

bool cdir::open(const char* path) {
    fPath = path;
    return open();
}

bool cdir::open() {
    close();
    if (fPath) {
        impl = static_cast<void *>(opendir(fPath));
    }
    return isOpen();
}

bool cdir::next() {
    if (impl) {
        DirPtr dirp(impl);
        if (dirp.next()) {
            strlcpy(fEntry, dirp.name(), sizeof fEntry);
            return true;
        }
    }
    return false;
}

bool cdir::nextExt(const char *extension) {
    int xlen = int(strlen(extension));
    while (next()) {
        int start = int(strlen(fEntry)) - xlen;
        if (start >= 0 && 0 == strcmp(fEntry + start, extension)) {
            return true;
        }
    }
    return false;
}

void cdir::rewind() {
    if (isOpen()) {
        DirPtr(impl).rewind();
    }
}

adir::adir(const char* path)
    : fPath(path), fLast(-1)
{
    if (path) {
        open();
    }
}

void adir::close() {
    fName.clear();
    fLast = -1;
}

bool adir::open(const char* path) {
    fPath = path;
    return open();
}

bool adir::open() {
    close();
    if (fPath) {
        DirPtr dirp(fPath);
        if (dirp) {
            while (dirp.next()) {
                fName.append(dirp.name());
            }
            fName.sort();
        }
        fLast = -1;
    }
    return isOpen();
}

bool adir::next() {
    return 1 + fLast < count() && ++fLast >= 0;
}

const char* adir::entry() const {
    if (fLast >= 0 && fLast < count()) {
        return fName[fLast];
    }
    return nullptr;
}

bool adir::nextExt(const char *extension) {
    int xlen = int(strlen(extension));
    while (next()) {
        int start = int(strlen(entry())) - xlen;
        if (start >= 0 && 0 == strcmp(entry() + start, extension)) {
            return true;
        }
    }
    return false;
}

udir::udir(const upath& path)
    : fPath(path), impl(nullptr)
{
    if (fPath.nonempty()) {
        open();
    }
}

void udir::close() {
    if (impl) {
        DirPtr(impl).close();
        impl = nullptr;
    }
}

bool udir::open(const upath& path) {
    fPath = path;
    return open();
}

bool udir::open() {
    close();
    if (fPath.nonempty()) {
        impl = static_cast<void *>(opendir(fPath.string()));
    }
    return isOpen();
}

bool udir::next() {
    if (impl) {
        DirPtr dirp(impl);
        if (dirp.next()) {
            fEntry = dirp.name();
            return true;
        }
    }
    return false;
}

bool udir::nextExt(const mstring& extension) {
    while (next()) {
        if (fEntry.endsWith(extension)) {
            return true;
        }
    }
    return false;
}

sdir::sdir(const upath& path)
    : fPath(path), fLast(-1)
{
    if (path.nonempty()) {
        open();
    }
}

void sdir::close() {
    fName.clear();
    fLast = -1;
}

bool sdir::open(const upath& path) {
    fPath = path;
    return open();
}

bool sdir::open() {
    close();
    if (fPath.nonempty()) {
        DirPtr dirp(fPath);
        if (dirp) {
            while (dirp.next()) {
                mstring copy(dirp.name());
                fName.append(copy);
            }
            fName.sort();
        }
        fLast = -1;
    }
    return isOpen();
}

bool sdir::next() {
    return 1 + fLast < count() && ++fLast >= 0;
}

const mstring& sdir::entry() const {
    return fName[fLast];
}

bool sdir::nextExt(const mstring& extension) {
    while (next()) {
        if (entry().endsWith(extension)) {
            return true;
        }
    }
    return false;
}


// vim: set sw=4 ts=4 et:
