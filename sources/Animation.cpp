#include "../includes/Animation.hpp"

// Animation implementation

// public
Animation::Animation(const std::function<void(float)>& forward, float duration)
    : mForward(forward), mBackward(nullptr), mDuration(duration), mElapsed(0.0f), mDirection(Direction::Forward) {}

Animation::Animation(const std::function<void(float)>& forward, const std::function<void(float)>& backward, float duration)
    : mForward(forward), mBackward(backward), mDuration(duration), mElapsed(0.0f), mDirection(Direction::Forward) {}

void Animation::update(float dt)
{
    mElapsed += dt;
    float progress = std::min(1.0f, mElapsed / mDuration);

    if (mDirection == Direction::Forward && mForward) {
        mForward(progress);
    }
    else if (mDirection == Direction::Backward && mBackward) {
        mBackward(progress);
    }
}

bool Animation::isFinished() const
{
    return mElapsed >= mDuration;
}

void Animation::reset()
{
    mElapsed = 0.0f;
}

void Animation::setDirection(Direction dir)
{
    mDirection = dir;
}

float Animation::getDuration() const
{
    return mDuration;
}

// AnimationList implementation
// =========================================================

// private
void AnimationList::proceedToNext()
{
    if (mLooping) {
        mCurrentAnimation = (mCurrentAnimation + 1) % mList.size();
    }
    else if (mCurrentAnimation < mList.size() - 1) {
        mCurrentAnimation++;
    }
    else {
        mIsPlaying = false;
        return;
    }
    mList[mCurrentAnimation].reset();
}

void AnimationList::proceedToPrevious()
{
    if (mLooping) {
        mCurrentAnimation = (mCurrentAnimation - 1 + mList.size()) % mList.size();
    }
    else if (mCurrentAnimation > 0) {
        mCurrentAnimation--;
    }
    mList[mCurrentAnimation].reset();
}

// public
AnimationList::AnimationList()
    : mCurrentAnimation(0), mCooldown(0.0f), mDelayTimer(0.0f),
      mSpeed(1.0f), mIsPlaying(false), mLooping(false), mWaiting(false) {}

bool AnimationList::isFinished() const
{
    return !mIsPlaying && mCurrentAnimation >= mList.size() && !mLooping;
}

unsigned int AnimationList::getCurrentAnimation() const
{
    return mCurrentAnimation;
}

unsigned int AnimationList::getSize() const
{
    return mList.size();
}

float AnimationList::getSpeed() const
{
    return mSpeed;
}

bool AnimationList::isEmpty() const
{
    return mList.empty();
}

bool AnimationList::isPlaying() const
{
    return mIsPlaying;
}

bool AnimationList::isLooping() const
{
    return mLooping;
}

void AnimationList::push(const Animation& animation)
{
    mList.push_back(animation);
}

void AnimationList::clear()
{
    mList.clear();
    mCurrentAnimation = 0;
    mIsPlaying        = false;
}

void AnimationList::play()
{
    if (!mList.empty()) {
        mIsPlaying = true;
        if (mCurrentAnimation >= mList.size()) {
            mCurrentAnimation = 0;
            for (auto& anim : mList) {
                anim.reset();
            }
        }
    }
}

void AnimationList::pause()
{
    mIsPlaying = false;
}

void AnimationList::playNext()
{
    if (!mList.empty()) {
        proceedToNext();
        mIsPlaying = true;
    }
}

void AnimationList::playPrevious()
{
    if (!mList.empty()) {
        proceedToPrevious();
        mIsPlaying = true;
    }
}

void AnimationList::goToFront()
{
    if (!mList.empty()) {
        mCurrentAnimation = 0;
        mList[0].reset();
    }
}

void AnimationList::goToBack()
{
    if (!mList.empty()) {
        mCurrentAnimation = mList.size() - 1;
        mList.back().reset();
    }
}

void AnimationList::setSpeed(float speed)
{
    mSpeed = std::max(0.1f, speed); // Prevent negative or zero speed
}

void AnimationList::setLooping(bool looping)
{
    mLooping = looping;
}

void AnimationList::setCooldown(float cooldown)
{
    mCooldown = cooldown;
}

void AnimationList::update(float dt)
{
    if (mIsPlaying && !mList.empty()) {
        if (!mWaiting) {
            float adjustedDt = dt * mSpeed;
            mList[mCurrentAnimation].update(adjustedDt);

            if (mList[mCurrentAnimation].isFinished()) {
                if (mCooldown > 0.0f) {
                    mWaiting    = true;
                    mDelayTimer = 0.0f;
                }
                else {
                    proceedToNext();
                }
            }
        }
        else {
            mDelayTimer += dt;
            if (mDelayTimer >= mCooldown) {
                mWaiting = false;
                proceedToNext();
            }
        }
    }
}