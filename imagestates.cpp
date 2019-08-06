#include "imagestates.hpp"

void ImageStates::AddState(ImageStates::ImagePtr image)
{
    if(!image) return;

    auto pImage = std::make_shared<QImage>(*image);
    if(imgIterator_ != std::cend(imgStates_)) {
        imgStates_.erase(std::next(imgIterator_), std::cend(imgStates_));
    }
    imgIterator_ = imgStates_.insert(std::cend(imgStates_), std::move(pImage));
}

ImageStates::ImagePtr ImageStates::Revert()
{
    imgIterator_ = std::cbegin(imgStates_);
    return *imgIterator_;
}

ImageStates::ImagePtr ImageStates::CurrentState() const
{
    if(imgIterator_ != std::cend(imgStates_)) {
        return *imgIterator_;
    }
    return {};
}

ImageStates::ImagePtr ImageStates::NextState()
{
    if(imgIterator_ != std::cend(imgStates_) &&
            std::next(imgIterator_) != std::cend(imgStates_)) {
        ++imgIterator_;
        return *imgIterator_;
    }
    return {};
}

ImageStates::ImagePtr ImageStates::PrevState()
{
    if(imgIterator_ != std::cbegin(imgStates_)){
        --imgIterator_;
        return *imgIterator_;
    }
    return {};
}
