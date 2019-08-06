#ifndef IMAGESTATESITERATOR_HPP
#define IMAGESTATESITERATOR_HPP

#include <memory>
#include <vector>

#include <QImage>

class ImageStates
{
    using ImagePtr = std::shared_ptr<const QImage>;
    using Images = std::vector<ImagePtr>;
public:
    ImageStates() : imgIterator_{std::cend(imgStates_)} {}

    inline void Clear() { imgStates_.clear(); imgIterator_ = std::cend(imgStates_); }
    inline bool HasStates() const { return !imgStates_.empty(); }

    void AddState(ImagePtr image);
    ImagePtr Revert();
    ImagePtr CurrentState() const;
    ImagePtr NextState();
    ImagePtr PrevState();

private:
    Images imgStates_;
    Images::const_iterator imgIterator_;
};

#endif // IMAGESTATESITERATOR_HPP
