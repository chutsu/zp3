#include "display.hpp"

void menu_init(menu_t &menu, const std::vector<std::string> entries) {
  menu.configured = true;
  menu.entries = entries;
}

void menu_clear(menu_t &menu) {
  menu.configured = false;
  menu.entries.clear();
}

std::vector<std::string> menu_get_page(menu_t &menu, const int index) {
  const int nb_entries = menu.entries.size();
  const int max_entries = menu.max_entries;
  const int menu_page = static_cast<int>(index / max_entries);

  const auto idx_start = max_entries * menu_page;
  const auto remaining = min(nb_entries - idx_start, max_entries);
  const auto idx_end = idx_start + remaining;

  return std::vector<std::string>{menu.entries.begin() + idx_start,
                                  menu.entries.begin() + idx_end};
}

display_t::display_t() {
  display_init();
}

void display_init() {
#if ZP3_DISPLAY == DISPLAY_SDL
  ssd1351_128x128_spi_init(3, 4, 5);
#elif ZP3_DISPLAY == DISPLAY_HARDWARE
  // Raspberry mode (gpio25=RST, 0=CE, gpio24=D/C)
  ssd1351_128x128_spi_init(25, 0, 24);
#endif

  // Initialize display
  ssd1351_setMode(LCD_MODE_NORMAL);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_clearScreen();
}

static void display_menu_entry(NanoCanvas8 &canvas,
                               const std::string &entry,
                               const int menu_idx,
                               const int rel_idx,
                               const int x,
                               const int y,
                               const int scroll_idx,
                               const size_t max_chars,
                               const size_t max_entries) {
  // // Scroll text
  std::string text = entry;
  // if (menu_idx == rel_idx && entry.length() > max_chars) {
  //   text = entry.substr(scroll_idx, -1);
  // }

  // Print text
  if (menu_idx == rel_idx) {
    // Draw white background
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    const int screen_width = ssd1306_displayWidth();
    const int x1 = x - 1;
    const int y1 = y - 3;
    const int x2 = screen_width - (2 * x) + 1;
    const int y2 = y + 10;
    canvas.fillRect(x1, y1, x2, y2);

    // Set text color to black
    canvas.setColor(RGB_COLOR8(0, 0, 0));
  } else {
    // Set text color to white
    canvas.setColor(RGB_COLOR8(255, 255, 255));
  }

  // Draw text
  canvas.printFixed(x - scroll_idx, y, text.c_str());
}

void display_menu(display_t &display, const int selection_idx, const int scroll_idx) {
  const int max_chars = display.menu.max_chars;
  const int max_entries = display.menu.max_entries;

  uint8_t buffer[display.width * display.height] = {0};
  NanoCanvas8 canvas(display.width, display.height, buffer);
  canvas.setMode(CANVAS_MODE_TRANSPARENT);

  // Display menu
  const int x = 1;
  int y = 2;
  int menu_idx = 0;
  const auto menu_page = menu_get_page(display.menu, selection_idx);

  // Calculate relative index within a page of entries. This is the local index
  // within a page of entries. Where selection_idx is the global index.
  const int idx_end = display.menu.entries.size() - 1;
  int rel_idx = (selection_idx < 0) ? 0 : selection_idx;
  rel_idx = (rel_idx > idx_end) ? idx_end : rel_idx;
  rel_idx = rel_idx % max_entries;

  for (const auto &entry : menu_page) {
    display_menu_entry(canvas,
                       entry,
                       menu_idx,
                       rel_idx,
                       x,
                       y,
                       scroll_idx,
                       max_chars,
                       max_entries);
    y += 12;
    menu_idx++;
  }

  canvas.setColor(RGB_COLOR8(255, 255, 255));
  canvas.blt();
}

void display_song(display_t &display,
                  const int player_state,
                  const song_t &song,
                  const float song_time,
                  const float song_length) {
  // Setup canvas
  const int track_scroll_counter = 0;
  uint8_t buffer[display.width * display.height] = {0};
  NanoCanvas8 canvas(display.width, display.height, buffer);
  canvas.setMode(CANVAS_TEXT_WRAP_LOCAL);

  // Track name
  {
    const int x = 2 - track_scroll_counter;
    const int y = 20;
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.printFixed(x, y, song.title.c_str(), STYLE_NORMAL);
  }

  // Track artist
  {
    const int x = 2 - track_scroll_counter;
    const int y = 35;
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.printFixed(x, y, song.artist.c_str(), STYLE_NORMAL);
  }

  // Track album
  {
    const int x = 2 - track_scroll_counter;
    const int y = 50;
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.printFixed(x, y, song.album.c_str(), STYLE_NORMAL);
  }

  // Track progress
  {
    float track_progress = song_time / song_length;
    if (track_progress < 0 || song_length < 0.01) {
      track_progress = 0.0;
    } else if (track_progress > 1.0) {
      track_progress = 1.0;
    }
    // -- Progress outline
    const int top_left[2] = {10, 72};
    const int bottom_right[2] = {display.width - 10, top_left[1] + 10};
    ssd1306_setColor(RGB_COLOR8(255, 255, 255));
    canvas.drawRect(top_left[0], top_left[1], bottom_right[0], bottom_right[1]);
    // -- Progress bar
    const int progress_bar_width = (display.width - 10) - 10;
    const int progress_x = 10 + (progress_bar_width * track_progress);
    ssd1306_setColor(RGB_COLOR8(255, 255, 255));
    canvas.fillRect(top_left[0], top_left[1], progress_x, bottom_right[1]);
  }

  // Track status
  if (player_state == PLAYER_PAUSE) {
    // Pause box
    {
      const int width = 14;
      const int center[2] = {display.width / 2, 105};
      const int p1[2] = {center[0] - width / 2, center[1] - width / 2};
      const int p2[2] = {center[0] + width / 2, center[1] + width / 2};
      canvas.setColor(RGB_COLOR8(255, 255, 255));
      canvas.fillRect(p1[0], p1[1], p2[0], p2[1]);
    }

    // Pause split
    {
      const int height = 14;
      const int width = 4;
      const int center[2] = {display.width / 2, 105};
      const int p1[2] = {center[0] - width / 2, center[1] - height / 2};
      const int p2[2] = {center[0] + width / 2, center[1] + height / 2};
      canvas.setColor(RGB_COLOR8(0, 0, 0));
      canvas.fillRect(p1[0], p1[1], p2[0], p2[1]);
    }

  } else if (player_state == PLAYER_PLAY) {
    const int width = 14;
    const int center[2] = {display.width / 2, 105};
    const int p1[2] = {center[0] - width / 2, center[1] - width / 2};
    const int p2[2] = {center[0] - width / 2, center[1] + width / 2};
    const int p3[2] = {center[0] + width / 2, center[1]};
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.drawLine(p1[0], p1[1], p2[0], p2[1]);
    canvas.drawLine(p2[0], p2[1], p3[0], p3[1]);
    canvas.drawLine(p3[0], p3[1], p1[0], p1[1]);

    // The following is pretty dumb but the canvas object provides
    // no api to draw a polygon and fill it with a color. This painful hack
    // will do for now. It essentially fills in the play symbol by filling
    // every single pixel, from left to right, top to bottom.
    for (int i = 0; i < 12; i++) {
      canvas.putPixel(p1[0] + 1, (center[1] - width / 2 + 1) + i);
    }
    for (int i = 0; i < 12; i++) {
      canvas.putPixel(p1[0] + 2, (center[1] - width / 2 + 2) + i);
    }
    for (int i = 0; i < 10; i++) {
      canvas.putPixel(p1[0] + 3, (center[1] - width / 2 + 3) + i);
    }
    for (int i = 0; i < 9; i++) {
      canvas.putPixel(p1[0] + 4, (center[1] - width / 2 + 3) + i);
    }
    for (int i = 0; i < 9; i++) {
      canvas.putPixel(p1[0] + 5, (center[1] - width / 2 + 3) + i);
    }
    for (int i = 0; i < 7; i++) {
      canvas.putPixel(p1[0] + 6, (center[1] - width / 2 + 4) + i);
    }
    for (int i = 0; i < 7; i++) {
      canvas.putPixel(p1[0] + 7, (center[1] - width / 2 + 4) + i);
    }
    for (int i = 0; i < 6; i++) {
      canvas.putPixel(p1[0] + 8, (center[1] - width / 2 + 5) + i);
    }
    for (int i = 0; i < 5; i++) {
      canvas.putPixel(p1[0] + 9, (center[1] - width / 2 + 5) + i);
    }
    canvas.putPixel(p1[0] + 10, (center[1] - width / 2 + 6) + 0);
    canvas.putPixel(p1[0] + 10, (center[1] - width / 2 + 6) + 1);
    canvas.putPixel(p1[0] + 10, (center[1] - width / 2 + 6) + 2);
    canvas.putPixel(p1[0] + 11, (center[1] - width / 2 + 7) + 0);
    canvas.putPixel(p1[0] + 11, (center[1] - width / 2 + 7) + 1);
    canvas.putPixel(p1[0] + 12, (center[1] - width / 2 + 7));

  } else if (player_state == PLAYER_STOP) {
    const int width = 14;
    const int center[2] = {display.width / 2, 105};
    const int p1[2] = {center[0] - width / 2, center[1] - width / 2};
    const int p2[2] = {center[0] + width / 2, center[1] + width / 2};
    canvas.setColor(RGB_COLOR8(255, 255, 255));
    canvas.fillRect(p1[0], p1[1], p2[0], p2[1]);
  }

  // Display canvas
  canvas.blt();
}

void display_clear(display_t &display) {
  menu_clear(display.menu);
  ssd1306_clearScreen();
}

void display_show_menu(display_t &display, const int index) {
  std::vector<std::string> menu_items = {"Songs", "Artists", "Albums"};

#if ZP3_DISPLAY == DISPLAY_CONSOLE
  system("clear");
  int menu_index = 0;
  for (const auto &entry : menu_items) {
    if (menu_index == index) {
      printf("%s", KGRN);
    }
    printf("%s\n", entry.c_str());
    if (menu_index == index) {
      printf("%s", KNRM);
    }
    menu_index++;
  }
  printf("\n");
#elif ZP3_DISPLAY == DISPLAY_SDL
  menu_init(display.menu, menu_items);
  display_menu(display, index);
#endif
}

void display_show_songs(display_t &display,
                        const songs_t &songs,
                        const int index) {
#if ZP3_DISPLAY == DISPLAY_CONSOLE
  int song_index = 0;
  system("clear");
  for (const auto &song : songs) {
    if (song_index == index) {
      printf("%s", KGRN);
    }
    printf("%s - [%s]", song.artist.c_str(), song.title.c_str());
    if (song_index == index) {
      printf("%s", KNRM);
    }
    song_index++;
    printf("\n");
  }
  printf("\n");
#elif ZP3_DISPLAY == DISPLAY_SDL || ZP3_DISPLAY == DISPLAY_HARDWARE
  std::vector<std::string> menu_items;
  for (const auto &song : songs) {
    menu_items.emplace_back(song.title);
  }
  menu_init(display.menu, menu_items);
  display_menu(display, index);
#endif
}

std::string display_show_artists(display_t &display,
                                 const artists_t &artists,
                                 const int index) {
  const auto keys = extract_keys<std::string, std::set<std::string>>(artists);

#if ZP3_DISPLAY == DISPLAY_CONSOLE
  int artist_index = 0;
  system("clear");
  for (const auto &key : keys) {
    if (artist_index == index) {
      printf("%s", KGRN);
    }
    printf("Artist: %s", key.c_str());
    if (artist_index == index) {
      printf("%s", KNRM);
    }
    artist_index++;
    printf("\n");
  }
  printf("\n");
#elif ZP3_DISPLAY == DISPLAY_SDL || ZP3_DISPLAY == DISPLAY_HARDWARE
  menu_init(display.menu, keys);
  display_menu(display, index);
#endif

  return keys[index];
}

std::string display_show_albums(display_t &display,
                                const std::vector<std::string> &albums,
                                const int index) {
#if ZP3_DISPLAY == DISPLAY_CONSOLE
  int album_index = 0;
  system("clear");
  for (const auto &album : albums) {
    if (album_index == index) {
      printf("%s", KGRN);
    }
    printf("%s", album.c_str());
    if (album_index == index) {
      printf("%s", KNRM);
    }
    album_index++;
    printf("\n");
  }
  printf("\n");
#elif ZP3_DISPLAY == DISPLAY_SDL || ZP3_DISPLAY == DISPLAY_HARDWARE
  menu_init(display.menu, albums);
  display_menu(display, index);
#endif

  return albums[index];
}
