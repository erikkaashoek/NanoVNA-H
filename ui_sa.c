#include "nanoVNA.h"

#ifdef __SA__

static void choose_active_trace(void)
{
  int i;
  if (trace[uistat.current_trace].enabled)
    // do nothing
    return;
  for (i = 0; i < TRACE_COUNT ; i++)
    if (trace[i].enabled) {
      uistat.current_trace = i;
      return;
    }
}

static int32_t get_marker_frequency(int marker)
{
  if (marker < 0 || marker >= MARKER_COUNT)
    return -1;
  if (!markers[marker].enabled)
    return -1;
  return frequencies[markers[marker].index];
}

static void active_marker_select(int item)
{
  if (item == -1) {
    active_marker = previous_marker;
    previous_marker = -1;
    if (active_marker == -1) {
      choose_active_marker();
    }
  } else {
    if (previous_marker != active_marker)
      previous_marker = active_marker;
    active_marker = item;
  }
}


// ===[MENU DEFINITION]=========================================================

static const menuitem_t menu_top[] = {
//  MENUITEM_MENU("CONFIG",    menu_config),
  //MENUITEM_CLOSE,
  MENUITEM_END
};


void
show_logo(void)
{
  int x = 15, y = 30;
  ili9341_fill(0, 0, 320, 240, 0);
#if !defined(ANTENNA_ANALYZER)
  ili9341_drawstring_size(BOARD_NAME, x+60, y, RGBHEX(0x0000FF), 0x0000, 4);
  y += 25;

  ili9341_drawstring_size("NANOVNA.COM", x+100, y += 10, 0xffff, 0x0000, 2);
  ili9341_drawstring_5x7("https://github.com/hugen79/NanoVNA-H", x, y += 20, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Based on edy555 design", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("2016-2019 Copyright @edy555", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Licensed under GPL. See: https://github.com/ttrftech/NanoVNA", x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Version: " VERSION, x, y += 10, 0xffff, 0x0000);
  ili9341_drawstring_5x7("Build Time: " __DATE__ " - " __TIME__, x, y += 10, 0xffff, 0x0000);
//  y += 5;
//  ili9341_drawstring_5x7("Kernel: " CH_KERNEL_VERSION, x, y += 10, 0xffff, 0x0000);
//  ili9341_drawstring_5x7("Architecture: " PORT_ARCHITECTURE_NAME " Core Variant: " PORT_CORE_VARIANT_NAME, x, y += 10, 0xffff, 0x0000);
//  ili9341_drawstring_5x7("Port Info: " PORT_INFO, x, y += 10, 0xffff, 0x0000);
//  ili9341_drawstring_5x7("Platform: " PLATFORM_NAME, x, y += 10, 0xffff, 0x0000);

#else
  ili9341_drawstring_size(BOARD_NAME, x+80, y, RGBHEX(0x0000FF), 0x0000, 2);
    y += 14;

    ili9341_drawstring_7x13("NANOVNA.COM", x+100, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("https://github.com/hugen79/NanoVNA-H", x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("Based on edy555 design", x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("2016-2019 Copyright @edy555", x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("https://github.com/ttrftech/NanoVNA", x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("Version: " VERSION, x, y += 15, 0xffff, 0x0000);
    ili9341_drawstring_7x13("Build Time: " __DATE__ " - " __TIME__, x, y += 15, 0xffff, 0x0000);
#endif
}


#endif



static void menu_item_modify_attribute(
    const menuitem_t *menu, int item, uint16_t *fg, uint16_t *bg)
{
#ifdef __VNA__
  if (menu == menu_trace && item < TRACE_COUNT  && item < MARKER_COUNT) {
    if (trace[item].enabled)
      *bg = config.trace_color[item];
  } else if (menu == menu_marker_sel && item < 4&& item < MARKER_COUNT) {
    if (markers[item].enabled) {
      *bg = 0x0000;
      *fg = 0xffff;
    }   
  } else if (menu == menu_calop) {
    if ((item == 0 && (cal_status & CALSTAT_OPEN))
        || (item == 1 && (cal_status & CALSTAT_SHORT))
        || (item == 2 && (cal_status & CALSTAT_LOAD))
        || (item == 3 && (cal_status & CALSTAT_ISOLN))
        || (item == 4 && (cal_status & CALSTAT_THRU))) {
      domain_mode = (domain_mode & ~DOMAIN_MODE) | DOMAIN_FREQ;
      *bg = 0x0000;
      *fg = 0xffff;
    }
  } else if (menu == menu_stimulus) {
    if (item == 5 /* PAUSE */ && !sweep_enabled) {
      *bg = 0x0000;
      *fg = 0xffff;
    }
  } else if (menu == menu_cal) {
    if (item == 3 /* CORRECTION */ && (cal_status & CALSTAT_APPLY)) {
      *bg = 0x0000;
      *fg = 0xffff;
    }
  } else if (menu == menu_transform) {
      if ((item == 0 && (domain_mode & DOMAIN_MODE) == DOMAIN_TIME)
       || (item == 1 && (domain_mode & TD_FUNC) == TD_FUNC_LOWPASS_IMPULSE)
       || (item == 2 && (domain_mode & TD_FUNC) == TD_FUNC_LOWPASS_STEP)
       || (item == 3 && (domain_mode & TD_FUNC) == TD_FUNC_BANDPASS)
       ) {
        *bg = 0x0000;
        *fg = 0xffff;
      }
  } else if (menu == menu_transform_window) {
      if ((item == 0 && (domain_mode & TD_WINDOW) == TD_WINDOW_MINIMUM)
       || (item == 1 && (domain_mode & TD_WINDOW) == TD_WINDOW_NORMAL)
       || (item == 2 && (domain_mode & TD_WINDOW) == TD_WINDOW_MAXIMUM)
       ) {
        *bg = 0x0000;
        *fg = 0xffff;
      }
  }
#endif
}

static void fetch_numeric_target(void)
{
  switch (keypad_mode) {
  case KM_START:
    uistat.value = get_sweep_frequency(ST_START);
    break;
  case KM_STOP:
    uistat.value = get_sweep_frequency(ST_STOP);
    break;
  case KM_CENTER:
    uistat.value = get_sweep_frequency(ST_CENTER);
    break;
  case KM_SPAN:
    uistat.value = get_sweep_frequency(ST_SPAN);
    break;
  case KM_CW:
    uistat.value = get_sweep_frequency(ST_CW);
    break;
  case KM_SCALE:
    uistat.value = get_trace_scale(uistat.current_trace) * 1000;
    break;
  case KM_REFPOS:
    uistat.value = get_trace_refpos(uistat.current_trace) * 1000;
    break;
#ifdef __VNA__
  case KM_EDELAY:
    uistat.value = get_electrical_delay();
    break;
  case KM_VELOCITY_FACTOR:
    uistat.value = velocity_factor;
    break;
  case KM_SCALEDELAY:
    uistat.value = get_trace_scale(uistat.current_trace) * 1e12;
    break;
#endif
  }
  
  {
    uint32_t x = uistat.value;
    int n = 0;
    for (; x >= 10 && n < 9; n++)
      x /= 10;
    uistat.digit = n;
  }
  uistat.previous_value = uistat.value;
}

