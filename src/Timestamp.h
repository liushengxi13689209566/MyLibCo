

#ifndef MUDUO_BASE_TIMESTAMP_H
#define MUDUO_BASE_TIMESTAMP_H

#include <stdint.h>
#include <string>

namespace Tattoo
{
class Timestamp
{
  public:
    Timestamp();

    explicit Timestamp(int64_t microSecondsSinceEpoch);
    void swap(Timestamp &that)
    {
        std::swap(microSecondsSinceEpoch_, that.microSecondsSinceEpoch_);
    }
    std::string toString() const;
    std::string toFormattedString() const;

    //微妙大于0就是　valid　的
    bool valid() const { return microSecondsSinceEpoch_ > 0; }
    int64_t microSecondsSinceEpoch() const { return microSecondsSinceEpoch_; }
    //微秒转化为秒
    time_t secondsSinceEpoch() const
    {
        return static_cast<time_t>(microSecondsSinceEpoch_ / kMicroSecondsPerSecond);
    }
    //得到现在的时间
    static Timestamp now();
    //获取一个无效时间，即时间等于0
    static Timestamp invalid();
    //一百万，一微秒等于百万分之一秒

    static const int kMicroSecondsPerSecond = 1000 * 1000;

  private:
    int64_t microSecondsSinceEpoch_;
};

inline bool operator<(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() < rhs.microSecondsSinceEpoch();
}

inline bool operator==(Timestamp lhs, Timestamp rhs)
{
    return lhs.microSecondsSinceEpoch() == rhs.microSecondsSinceEpoch();
}
////将返回两个事件时间差的秒数，注意单位!
inline double timeDifference(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

//把秒转化为微秒，构造一个对象，再把它们的时间加起来，构造一个无名临时对象返回
inline Timestamp addTime(Timestamp timestamp, double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * Timestamp::kMicroSecondsPerSecond);
    return Timestamp(timestamp.microSecondsSinceEpoch() + delta);
}
} // namespace Tattoo
#endif // MUDUO_BASE_TIMESTAMP_H
