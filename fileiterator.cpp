#include "fileiterator.hpp"


void FileIterator::LoadDirectory(const QDir& dir)
{
   files_ = dir.entryList(QDir::Files);
   std::transform(files_.cbegin(), files_.cend(), files_.begin(),
   [&dir](const auto& item){
       return dir.path() + '/' + item;
   });
   filesIt_ = std::begin(files_);
}

void FileIterator::LoadFile(const QString& pathToFile)
{
    QDir currDir(pathToFile);
    currDir.cdUp();
    LoadDirectory(currDir);
    filesIt_ = std::find(std::cbegin(files_), std::cend(files_), pathToFile);
    if(filesIt_ == std::cend(files_)) {
        filesIt_ = std::cbegin(files_);
    }
}

//QString FileIterator::Next()
//{
//    if(filesIt_ == std::cend(files_)) {
//        filesIt_ = std::cbegin(files_);
//    }
//    else {
//        ++filesIt_;
//        if(filesIt_ == std::cend(files_)) {
//            filesIt_ = std::cbegin(files_);
//        }
//    }
//    return *filesIt_;
//}

//QString FileIterator::Prev()
//{
//    if(filesIt_ == std::cbegin(files_)) {
//        filesIt_ = std::cend(files_);
//    }
//    --filesIt_;
//    return *filesIt_;
//}
