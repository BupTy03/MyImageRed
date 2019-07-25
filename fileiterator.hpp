#ifndef FILEITERATOR_HPP
#define FILEITERATOR_HPP

#include <QStringList>
#include <QDir>

#include <algorithm>

class FileIterator
{
public:
    explicit FileIterator()
        : files_{}
        , filesIt_{std::cend(files_)}
    {}

    void LoadDirectory(const QDir& dir);
    void LoadFile(const QString& pathToFile);

    inline bool HasFiles() const { return !files_.isEmpty(); }

    void operator++();
    void operator--();
    inline QString operator*() const { return files_.isEmpty() ? QString() : *filesIt_; }

    template<class F>
    void FilterFiles(F predicate);

private:
    QStringList files_;
    QStringList::const_iterator filesIt_;
};

template<class F>
void FileIterator::FilterFiles(F predicate)
{
    QStringList result;
    std::copy_if(std::cbegin(files_), std::cend(files_), std::back_inserter(result), predicate);

    const QString currentFile = (filesIt_ != std::cend(files_)) ? *filesIt_ : "";

    files_ = std::move(result);
    if(!currentFile.isEmpty()) {
        filesIt_ = std::find(std::cbegin(files_), std::cend(files_), currentFile);
    }
    else {
        filesIt_ = std::cbegin(files_);
    }
}

#endif // FILEITERATOR_HPP
