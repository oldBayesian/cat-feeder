#include <array>
#include <cstdint>
#include <iostream>

struct FeederSim {
  static constexpr unsigned int maxFeedDay = 8;
  static constexpr uint64_t feedDelayInterval = 1000ULL * 60 * 5; // 5 min
  static constexpr std::array<uint64_t, 4> feedTimes = {
      1000ULL * 60 * 5,
      1000ULL * 60 * 15,
      1000ULL * 60 * 20,
      1000ULL * 60 * 25,
  };

  uint64_t timeStartDay = 0;
  uint64_t timeLastFeed = 0;
  uint64_t now = 0;
  unsigned int feedCounterDay = 0;
  unsigned int nextAutoFeedIndex = 0;

  void feed(const char *reason) {
    timeLastFeed = now;
    feedCounterDay++;
    std::cout << reason << " feed at minute " << (now / 60000) << " (count="
              << feedCounterDay << ")\n";
  }

  void loopStep() {
    uint64_t timeElapseDay = now - timeStartDay;
    uint64_t timeElapseLastFeed = now - timeLastFeed;
    bool isFeedDelay = (timeElapseLastFeed < feedDelayInterval);

    if (nextAutoFeedIndex < feedTimes.size() &&
        timeElapseDay >= feedTimes[nextAutoFeedIndex]) {
      std::cout << "slot idx=" << nextAutoFeedIndex
                << " reached at minute " << (now / 60000) << "\n";

      if (feedCounterDay < maxFeedDay && !isFeedDelay) {
        feed("AUTO");
        nextAutoFeedIndex++;
      } else {
        std::cout << "AUTO blocked (delay=" << (isFeedDelay ? "true" : "false")
                  << ", count=" << feedCounterDay << ")\n";
      }
    }
  }
};

int main() {
  FeederSim sim;

  // Simulate power-on at 6:00 and run for 35 minutes in 1-minute ticks.
  // Inject one manual feed at minute 14 to prove blocked slots retry.
  for (unsigned minute = 0; minute <= 35; ++minute) {
    sim.now = minute * 60000ULL;

    if (minute == 14) {
      sim.feed("MANUAL");
    }

    sim.loopStep();
  }

  return 0;
}
