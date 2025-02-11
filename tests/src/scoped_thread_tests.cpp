#include <gtest/gtest.h>

#include "databento/detail/scoped_thread.hpp"

namespace databento::detail ::test {
TEST(ScopedThreadTests, CtorSimple) {
  bool flag = false;
  {
    const ScopedThread target{[&flag] { flag = true; }};
  }  // joins
  ASSERT_TRUE(flag);
}

TEST(ScopedThreadTests, CtorWithArgs) {
  bool flag = false;
  {
    const ScopedThread target{[](bool* f) { *f = true; }, &flag};
  }  // joins
  ASSERT_TRUE(flag);
}

TEST(ScopedThreadTests, DefaultCtor) { const ScopedThread target{}; }

TEST(ScopedThreadTests, MoveCtor) {
  auto res = 0;
  const auto initThread = [&res] { return ScopedThread{[&res] { res = 9; }}; };
  {
    const ScopedThread target{initThread()};
  }  // joins
  ASSERT_EQ(res, 9);
}

TEST(ScopedThreadTests, MoveAssign) {
  bool flag1 = false;
  bool flag2 = false;
  {
    ScopedThread target1{};
    ScopedThread target2{};
    target1 = ScopedThread{[&flag1] { flag1 = true; }};
    target2 = ScopedThread{[&flag2] { flag2 = true; }};
    target2 = ScopedThread{};  // joins target2
    ASSERT_TRUE(flag2);
  }  // target1
  ASSERT_TRUE(flag1);
}

TEST(ScopedThreadTests, Join) {
  bool flag1 = false;
  ScopedThread target{[&flag1] { flag1 = true; }};
  ASSERT_TRUE(target.Joinable());
  target.Join();
  ASSERT_TRUE(flag1);
  ASSERT_FALSE(target.Joinable());
}
}  // namespace databento::detail::test
