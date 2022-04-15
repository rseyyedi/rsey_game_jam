#include <array>
#include <functional>
#include <iostream>
#include <random>

#include <docopt/docopt.h>
#include <ftxui/component/captured_mouse.hpp>// for ftxui
#include <ftxui/component/component.hpp>// for Slider
#include <ftxui/component/screen_interactive.hpp>// for ScreenInteractive
#include <spdlog/spdlog.h>

// This file will be generated automatically when you run the CMake
// configuration step. It creates a namespace called `rsey_game_jam`. You can modify
// the source template at `configured_files/config.hpp.in`.
#include <internal_use_only/config.hpp>

template<std::size_t Width, std::size_t Height> struct GameBoard
{
  static constexpr std::size_t width = Width;
  static constexpr std::size_t height = Height;
  std::string empty_tile = fmt::format("{:^4}", "");

  std::array<std::array<std::string, height>, width> strings;
  std::array<std::array<bool, height>, width> values{};

  std::size_t move_count{ 0 };

  std::string &get_string(std::size_t x, std::size_t y) { return strings.at(x).at(y); }
  [[nodiscard]] std::string get_label(std::size_t x, std::size_t y) const { return strings.at(x).at(y); }

  GameBoard()
  {
    for (std::size_t x = 0; x < width; ++x) {
      for (std::size_t y = 0; y < height; ++y) {
        auto tile_num = (x * width) + y + 1;
        strings.at(x).at(y) = fmt::format("{:^4}", std::to_string(tile_num));
      }
    }
    strings.at(width - 1).at(height - 1) = empty_tile;
    //  visit([](const auto x, const auto y, auto &gameboard) { gameboard.set(x, y, true); });
  }

  // NOLINTNEXTLINE(bugprone-exception-escape)
  void swap(std::size_t x_old, std::size_t y_old, std::size_t x_new, std::size_t y_new)
  {
    auto tmp_label = get_string(x_new, y_new);
    get_string(x_new, y_new) = get_string(x_old, y_old);
    get_string(x_old, y_old) = tmp_label;
  }

  void press(std::size_t x, std::size_t y)
  {
    ++move_count;
    // check if an empty tile is adjacent
    // up
    if ((y < height - 1) && get_label(x, y + 1) == empty_tile) {
      swap(x, y, x, y + 1);
    }
    // down
    else if ((y > 0) && get_label(x, y - 1) == empty_tile) {
      swap(x, y, x, y - 1);
    }
    // right
    else if ((x < width - 1) && get_label(x + 1, y) == empty_tile) {
      swap(x, y, x + 1, y);
    }
    // left
    else if ((x > 0) && get_label(x - 1, y) == empty_tile) {
      swap(x, y, x - 1, y);
    } else {
      --move_count;
    }
  }

  [[nodiscard]] bool solved() const
  {
    if (get_label(width - 1, height - 1) != empty_tile) { return false; }
    for (std::size_t x = 0; x < width; ++x) {
      for (std::size_t y = 0; y < height; ++y) {
        auto tile_num = (x * width) + y + 1;
        auto label = fmt::format("{:^4}", std::to_string(tile_num));
        if (x == (width - 1) && y == (height - 1)) {
          if (get_label(x, y) != empty_tile) { return false; }
        } else if (get_label(x, y) != label) {
          return false;
        }
      }
    }

    return true;
  }
};

void game_of_fifteen()
{
  auto screen = ftxui::ScreenInteractive::TerminalOutput();

  GameBoard<4, 4> gb;

  std::string quit_text;

  const auto update_quit_text = [&quit_text](const auto &game_board) {
    quit_text = fmt::format("Quit ({} moves)", game_board.move_count);
    if (game_board.solved()) { quit_text += " Solved!"; }
  };

  const auto make_tiles = [&] {
    std::vector<ftxui::Component> tiles;
    for (std::size_t x = 0; x < gb.width; ++x) {
      for (std::size_t y = 0; y < gb.height; ++y) {
        tiles.push_back(ftxui::Button(&gb.get_string(x, y), [=, &gb] {
          if (!gb.solved()) { gb.press(x, y); }
          update_quit_text(gb);
        }));
      }
    }
    return tiles;
  };

  auto tiles = make_tiles();

  auto quit_button = ftxui::Button(&quit_text, screen.ExitLoopClosure());

  auto make_layout = [&] {
    std::vector<ftxui::Element> rows;

    std::size_t idx = 0;

    for (std::size_t x = 0; x < gb.width; ++x) {
      std::vector<ftxui::Element> row;
      for (std::size_t y = 0; y < gb.height; ++y) {
        row.push_back(tiles[idx]->Render());
        ++idx;
      }
      rows.push_back(ftxui::hbox(std::move(row)));
    }

    rows.push_back(ftxui::hbox({ quit_button->Render() }));

    return ftxui::vbox(std::move(rows));
  };


  static constexpr int randomization_iterations = 100;
  std::random_device rd1;
  std::random_device rd2;

  std::mt19937 mt1{ rd1() };
  std::mt19937 mt2{ rd2() };
  std::uniform_int_distribution<std::size_t> x1(static_cast<std::size_t>(0), gb.width - 1);
  std::uniform_int_distribution<std::size_t> x2(static_cast<std::size_t>(0), gb.width - 1);
  std::uniform_int_distribution<std::size_t> y1(static_cast<std::size_t>(0), gb.height - 1);
  std::uniform_int_distribution<std::size_t> y2(static_cast<std::size_t>(0), gb.height - 1);

  for (int i = 0; i < randomization_iterations; ++i) { gb.swap(x1(mt1), y1(mt1), x2(mt2), y2(mt2)); }
  gb.move_count = 0;
  update_quit_text(gb);

  auto all_tiles = tiles;
  all_tiles.push_back(quit_button);
  auto container = ftxui::Container::Horizontal(all_tiles);

  for (std::size_t x = 0; x < gb.width; ++x) {
    for (std::size_t y = 0; y < gb.height; ++y) { std::cout << gb.strings.at(x).at(y); }
  }

  auto renderer = ftxui::Renderer(container, make_layout);

  screen.Loop(renderer);
}

int main(int argc, const char **argv)
{
  try {
    static constexpr auto USAGE =
      R"(intro

    Usage:
          intro 
          intro (-h | --help)
          intro --version
 Options:
          -h --help     Show this screen.
          --version     Show version.
)";

    std::map<std::string, docopt::value> args = docopt::docopt(USAGE,
      { std::next(argv), std::next(argv, argc) },
      true,// show help if requested
      fmt::format("{} {}",
        rsey_game_jam::cmake::project_name,
        rsey_game_jam::cmake::project_version));// version string, acquired
                                                // from config.hpp via CMake

    game_of_fifteen();
  } catch (const std::exception &e) {
    fmt::print("Unhandled exception in main: {}", e.what());
  }
}
