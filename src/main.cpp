#include <atomic>
#include <cmath>
#include <iostream>
#include <thread>

#include "ftxui/component/component.hpp"
#include "ftxui/component/screen_interactive.hpp"
#include "ftxui/dom/elements.hpp"
#include "ftxui/screen/screen.hpp"
#include "ftxui/screen/string.hpp"

int main() {
  using namespace std;
  using namespace ftxui;

  auto screen = ScreenInteractive::Fullscreen();

  int shift = 0;

  auto my_graph = [&shift](int width, int height) {
    std::vector<int> output(width);
    for (int i = 0; i < width; ++i) {
      float v = 0.5f;
      v += 0.1f * sin((i + shift) * 0.1f);
      v += 0.2f * sin((i + shift + 10) * 0.15f);
      v += 0.1f * sin((i + shift) * 0.03f);
      v *= height;
      output[i] = (int)v;
    }
    return output;
  };

  auto htop = Renderer([&]() -> Element {
    return vbox({
        text("Frequency [Mhz]") | hcenter,
        hbox({
            vbox({
                text("2400 "),
                filler(),
                text("1200 "),
                filler(),
                text("0 "),
            }),
            graph(std::ref(my_graph)) | flex,
        }) | flex,
    });
  });

  auto singleSlot1 =
      hbox({vbox({text("Slot 1"), text("EntityID: 42")}), separatorLight(),
            hbox({color(Color::Red, text("G") | bold),
                  color(Color::White, text("V") | bold),
                  color(Color::Green, text("F") | bold)}),
            separatorLight(),
            htop->Render() | flex | size(WIDTH, GREATER_THAN, 80) | size(HEIGHT, GREATER_THAN, 15)});
  auto singleSlot2 = hbox({text("Slot 2"), separatorLight(),
                           hbox({bgcolor(Color::Red, text("G") | bold),
                                 bgcolor(Color::Black, text("V") | bold),
                                 bgcolor(Color::Green, text("F") | bold)})});
  auto singleSlot3 = hbox({text("Slot 3"), separatorLight(),
                           hbox({bgcolor(Color::Red, text("G") | bold),
                                 bgcolor(Color::Black, text("V") | bold),
                                 bgcolor(Color::Green, text("F") | bold)})});
  auto singleSlot4 = hbox({text("Slot 4"), separatorLight(),
                           hbox({bgcolor(Color::Red, text("G") | bold),
                                 bgcolor(Color::Black, text("V") | bold),
                                 bgcolor(Color::Green, text("F") | bold)})});

  auto slotDisplay = vbox({singleSlot1, separator(), singleSlot2, separator(),
                           singleSlot3, separator(), singleSlot4});

  auto main_renderer = Renderer([&] {
    return vbox({text("FTXUI Demo") | bold | hcenter,
                 hbox({slotDisplay | borderLight})});
  });

  std::atomic<bool> refresh_ui_continue{true};
  std::thread refresh_ui([&] {
    while (refresh_ui_continue) {
      using namespace std::chrono_literals;
      std::this_thread::sleep_for(0.05s);
      // The |shift| variable belong to the main thread. `screen.Post(task)`
      // will execute the update on the thread where |screen| lives (e.g. the
      // main thread). Using `screen.Post(task)` is threadsafe.
      screen.Post([&] { shift++; });
      // After updating the state, request a new frame to be drawn. This is
      // done by simulating a new "custom" event to be handled.
      screen.Post(Event::Custom);
    }
  });

  screen.Loop(main_renderer);
  refresh_ui_continue = false;
  refresh_ui.join();

  return EXIT_SUCCESS;
}
