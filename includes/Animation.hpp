#pragma once
#include "../INIT.hpp"
#include "../includes/UI.hpp"

enum class Direction {
    Forward,
    Backward
};

class Animation {
private:
    std::function<void(float)> mForward;
    std::function<void(float)> mBackward; // optional, can be nullptr
    float mDuration;                      // seconds
    float mElapsed;                       // seconds
    Direction mDirection;

public:
    // Only forward function
    Animation(const std::function<void(float)>& forward, float duration);
    // Both forward and backward functions
    Animation(const std::function<void(float)>& forward, const std::function<void(float)>& backward, float duration);

    void update(float dt);
    bool isFinished() const;
    void reset();
    void setDirection(Direction dir);
    float getDuration() const;
};

class AnimationList {
private:
    void proceedToNext();
    void proceedToPrevious();

    unsigned int mCurrentAnimation;
    float mCooldown;   // Delay between animations in seconds
    float mDelayTimer; // Timer for delay in seconds
    float mSpeed;      // Speed multiplier
    bool mIsPlaying;   // Playback state
    bool mLooping;     // Loop the sequence
    bool mWaiting;     // Waiting between animations
    std::vector<Animation> mList;

public:
    AnimationList();

    bool isFinished() const;
    unsigned int getCurrentAnimation() const;
    unsigned int getSize() const;
    float getSpeed() const;
    bool isEmpty() const;
    bool isPlaying() const;
    bool isLooping() const;

    void push(const Animation& animation);
    void clear();
    void play();
    void pause();
    void playNext();
    void playPrevious();
    void goToFront();
    void goToBack();
    void setSpeed(float speed);
    void setLooping(bool looping);
    void setCooldown(float cooldown);
    void update(float dt);
};