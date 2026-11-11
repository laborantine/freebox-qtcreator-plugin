// Modified on 2026-11-11 for Qt Creator 20.0.2 and Qt 6.
#include "tar.hh"

#include <QDateTime>

#include <cstring>

namespace Freebox::Internal {
namespace {

constexpr qsizetype TarBlockSize = 512;

// Header layout derived from the original Freebox plugin.
struct TarFileHeader {
    char fileName[100];
    char fileMode[8];
    char uid[8];
    char gid[8];
    char length[12];
    char mtime[12];
    char chksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char fileNamePrefix[155];
    char padding[12];
};
static_assert(sizeof(TarFileHeader) == TarBlockSize);

template <size_t Size>
bool writeOctal(char (&field)[Size], quint64 value)
{
    const QByteArray text = QByteArray::number(value, 8);
    if (text.size() >= qsizetype(Size))
        return false;
    const size_t padding = Size - size_t(text.size()) - 1;
    std::memset(field, '0', padding);
    std::memcpy(field + padding, text.constData(), text.size());
    field[Size - 1] = '\0';
    return true;
}

} // namespace

QByteArray Tar::package(const QMultiMap<qsizetype, FreeStorePackageFile> &entries, QString *invalidPath)
{
    QByteArray archive;
    for (const FreeStorePackageFile &entry : entries) {
        const QByteArray header = entryHeader(entry);
        if (header.isEmpty()) {
            *invalidPath = entry.fileNameRelative;
            return {};
        }

        archive += header;
        if (!entry.fileInfo.isDir()) {
            archive += entry.content;
            const qsizetype remaining = entry.content.size() % TarBlockSize;
            if (remaining)
                archive += QByteArray(TarBlockSize - remaining, '\0');
        }
    }
    return archive + QByteArray(2 * TarBlockSize, '\0');
}

QByteArray Tar::entryHeader(const FreeStorePackageFile &entry)
{
    TarFileHeader header{};
    const bool directory = entry.fileInfo.isDir();
    const QByteArray path = entry.fileNameRelative.toUtf8();
    const qsizetype nameLimit = sizeof header.fileName;
    if (path.size() <= nameLimit) {
        std::memcpy(header.fileName, path.constData(), path.size());
    } else {
        const qsizetype searchFrom = qMin<qsizetype>(path.size() - (directory ? 2 : 1),
                                                    sizeof header.fileNamePrefix);
        const qsizetype separator = path.lastIndexOf('/', searchFrom);
        if (separator <= 0 || path.size() - separator - 1 > nameLimit)
            return {};
        std::memcpy(header.fileNamePrefix, path.constData(), separator);
        std::memcpy(header.fileName, path.constData() + separator + 1, path.size() - separator - 1);
    }

    const quint64 permissions = directory ? 0755 : 0644;
    if (!writeOctal(header.fileMode, permissions) || !writeOctal(header.uid, 0)
        || !writeOctal(header.gid, 0) || !writeOctal(header.length, directory ? 0 : entry.content.size())
        || !writeOctal(header.mtime, qMax<qint64>(0, entry.fileInfo.lastModified().toSecsSinceEpoch()))) {
        return {};
    }

    header.typeflag = directory ? '5' : '0';
    std::memcpy(header.magic, "ustar", 6);
    std::memcpy(header.version, "00", 2);
    std::memset(header.chksum, ' ', sizeof header.chksum);
    quint64 checksum = 0;
    for (const char byte : QByteArrayView(reinterpret_cast<const char *>(&header), sizeof header))
        checksum += static_cast<unsigned char>(byte);

    const QByteArray sum = QByteArray::number(checksum, 8).rightJustified(6, '0');
    std::memcpy(header.chksum, sum.constData(), 6);
    header.chksum[6] = '\0';
    return QByteArray(reinterpret_cast<const char *>(&header), sizeof header);
}

} // namespace Freebox::Internal
