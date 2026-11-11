// Modified on 2026-11-11 for Qt Creator 20.0.2 and Qt 6.
#pragma once

#include "freestorepackager.hh"

namespace Freebox::Internal {

class Tar
{
public:
    QByteArray package(const QMultiMap<qsizetype, FreeStorePackageFile> &entries, QString *invalidPath);

private:
    static QByteArray entryHeader(const FreeStorePackageFile &entry);
};

} // namespace Freebox::Internal
