//--------------------------------------------------------------
// Main Header 
//--------------------------------------------------------------
#include "ProgressBar.hpp"
//--------------------------------------------------------------
// Standard cpp library
//--------------------------------------------------------------
#include <iostream>
#include <string_view>
#include <sstream>
#include <iomanip>
#include <climits>
#include <algorithm>
#ifdef HAVE_TBB
    #include <execution>
#endif
#include <stdexcept>
#ifndef _WIN32
    #include <unistd.h>
    #include <sys/ioctl.h>
#endif
#include <cerrno> // for errno
#include <cstring> // for strerror
#include <string_view>
#include <csignal>
#include <numeric>
#include <charconv>
#include <type_traits>
//--------------------------------------------------------------
// Boost library 
//--------------------------------------------------------------
#ifdef USE_BOOST
    #include <boost/circular_buffer.hpp>
#else
    #include "CircularBuffer.hpp"
#endif
//--------------------------------------------------------------
// FMT library
//--------------------------------------------------------------
#ifdef HAVE_FMT
    //--------------------------
    #include <fmt/core.h>
    #include <fmt/color.h>
    #include <fmt/format.h>
    //--------------------------
#endif
//--------------------------------------------------------------
// Definitions 
//--------------------------------------------------------------
// ANSI Colors
//--------------------------
#ifndef HAVE_FMT
    //--------------------------
    #define ANSI_COLOR_BLACK            "\x1b[30m"
    #define ANSI_COLOR_RED              "\x1b[31m"
    #define ANSI_COLOR_GREEN            "\x1b[32m"
    #define ANSI_COLOR_YELLOW           "\x1b[33m"
    #define ANSI_COLOR_BLUE             "\x1b[34m"
    #define ANSI_COLOR_MAGENTA          "\x1b[35m"
    #define ANSI_COLOR_CYAN             "\x1b[36m"
    #define ANSI_COLOR_WHITE            "\x1b[37m"
    #define ANSI_COLOR_RESET            "\x1b[0m"
    //--------------------------
    // ANSI Bold
    //--------------------------
    #define ANSI_BOLD_ON                "\x1b[1m"
    #define ANSI_BOLD_OFF               ANSI_COLOR_RESET
    //--------------------------
#endif
//--------------------------
// ANSI Cursor Up
//--------------------------
#define ANSI_CURSOR_UP_ONE_LINE         "\033[1A"
//--------------------------
// ANSI Clear Line
//--------------------------
#define ANSI_CLEAR_LINE                 "\033[2K"
//--------------------------
// Conversion Constant
//--------------------------
constexpr uint32_t SECONDS_PER_DAY      = 86400U;
//--------------------------
// Boost Constant
//--------------------------
constexpr size_t CIRCULAR_BUFFER        = 10UL;
//--------------------------
// Interval Constant
//--------------------------
constexpr uint8_t UPDATE_INTERVAL       = 5U;
//--------------------------
// Bar Constant
//--------------------------
constexpr float BAR_PERCENTAGE          = 0.15f;
constexpr size_t DEFAULT_WIDTH          = 30UL;
#ifdef HAVE_FMT
    constexpr size_t MIN_WIDTH          = 10UL;
#endif
constexpr size_t MIN_BAR_LENGTH         = 15UL;       // Minimum length of the progress bar for visibility.
//--------------------------
// Default Characters (used when a caller gives an empty one)
//--------------------------
constexpr std::string_view DEFAULT_PROGRESS_CHAR    = "#";
constexpr std::string_view DEFAULT_EMPTY_SPACE_CHAR = "-";
//--------------------------
// Line Buffer Constant
//--------------------------
constexpr size_t LINE_SLACK             = 512UL;      // Room for the escape sequences and the time fields.
#ifdef HAVE_FMT
//--------------------------
// Style Sequences (the bytes fmt writes for bold, bold green and bold red)
//--------------------------
constexpr std::string_view STYLE_BOLD_ON    = "\x1b[1m";
constexpr std::string_view STYLE_GREEN_ON   = "\x1b[1m\x1b[38;2;000;128;000m";
constexpr std::string_view STYLE_RED_ON     = "\x1b[1m\x1b[38;2;255;000;000m";
constexpr std::string_view STYLE_OFF        = "\x1b[0m";
#endif
//--------------------------------------------------------------
size_t ProgressBar::ProgressBar::m_available_width  = 0UL;
size_t ProgressBar::ProgressBar::m_name_length      = 0UL;
size_t ProgressBar::ProgressBar::m_spaces_after_bar = 0UL;
size_t ProgressBar::ProgressBar::m_bar_length       = 0UL;
//--------------------------------------------------------------
ProgressBar::ProgressBar::ProgressBar(  std::string_view name, 
                                        std::string_view progress_char, 
                                        std::string_view empty_space_char) :  m_total(std::numeric_limits<size_t>::max()),
                                                                                m_progress(0UL),
                                                                                m_name(name),
                                                                                m_progress_char(progress_char.empty() ? DEFAULT_PROGRESS_CHAR : progress_char),
                                                                                m_empty_space_char(empty_space_char.empty() ? DEFAULT_EMPTY_SPACE_CHAR : empty_space_char),
                                                                                m_line(),
                                                                                m_start_time(std::chrono::steady_clock::now()){
    //--------------------------
    initializer(name);
    //--------------------------
}//end ProgressBar::ProgressBar::ProgressBar
//--------------------------------------------------------------
ProgressBar::ProgressBar::ProgressBar(  const size_t& total,
                                        std::string_view name, 
                                        std::string_view progress_char, 
                                        std::string_view empty_space_char) :  m_total(total),
                                                                                m_progress(0UL),
                                                                                m_name(name), 
                                                                                m_progress_char(progress_char.empty() ? DEFAULT_PROGRESS_CHAR : progress_char), 
                                                                                m_empty_space_char(empty_space_char.empty() ? DEFAULT_EMPTY_SPACE_CHAR : empty_space_char),
                                                                                m_line(),
                                                                                m_start_time(std::chrono::steady_clock::now()){
    //--------------------------
    initializer(name);
    //--------------------------
}// end ProgressBar::ProgressBar::ProgressBar
//--------------------------------------------------------------
void ProgressBar::ProgressBar::update(void){
    //--------------------------
    tick();
    //--------------------------
}//end void ProgressBar::ProgressBar::update(void)
//--------------------------------------------------------------
bool ProgressBar::ProgressBar::done(void){
    //--------------------------
    return is_done();
    //--------------------------
}// end bool ProgressBar::ProgressBar::done(void)
//--------------------------------------------------------------
void ProgressBar::ProgressBar::initializer(std::string_view name) const{
    //--------------------------
    m_name_length = name.length();
    //--------------------------
    calculate_bar();
    //--------------------------
#ifdef _WIN32
    SetConsoleCtrlHandler(&ProgressBar::ProgressBar::handle_console_signal, TRUE);
#else
    std::signal(SIGWINCH, handle_winch_signal);
#endif
    //--------------------------
}// end void ProgressBar::ProgressBar::initializer(std::string_view name) const
//--------------------------------------------------------------
ProgressBar::ProgressBar::TimeParts ProgressBar::ProgressBar::split_time(const std::chrono::milliseconds::rep& time) const {
    //--------------------------
    // Convert total milliseconds to chrono::milliseconds type
    //--------------------------
    std::chrono::milliseconds total_duration(time);
    //--------------------------
    // Extract days, hours, minutes, seconds, and milliseconds
    //--------------------------
    auto days_duration = std::chrono::duration_cast<std::chrono::duration<std::chrono::milliseconds::rep, std::ratio<SECONDS_PER_DAY>>>(total_duration);
    total_duration -= days_duration;
    auto hours_duration = std::chrono::duration_cast<std::chrono::hours>(total_duration);
    total_duration -= hours_duration;
    auto minutes_duration = std::chrono::duration_cast<std::chrono::minutes>(total_duration);
    total_duration -= minutes_duration;
    auto seconds_duration = std::chrono::duration_cast<std::chrono::seconds>(total_duration);
    total_duration -= seconds_duration;
    auto milliseconds_duration = std::chrono::duration_cast<std::chrono::milliseconds>(total_duration);
    //--------------------------
    return TimeParts{days_duration.count(), hours_duration.count(), minutes_duration.count(), seconds_duration.count(), milliseconds_duration.count()};
    //--------------------------
}// end ProgressBar::ProgressBar::TimeParts ProgressBar::ProgressBar::split_time(const std::chrono::milliseconds::rep& time) const
//--------------------------------------------------------------
void ProgressBar::ProgressBar::put_text(std::string_view text) {
    //--------------------------
    m_line.append(text.data(), text.size());
    //--------------------------
}// end void ProgressBar::ProgressBar::put_text(std::string_view text)
//--------------------------------------------------------------
void ProgressBar::ProgressBar::put_fill(size_t count, std::string_view text) {
    //--------------------------
    // One byte is the common case: one append of the whole run. A longer text (a multi-byte glyph) is written whole, count times.
    //--------------------------
    if (text.size() == 1UL) {
        //--------------------------
        m_line.append(count, text.front());
        //--------------------------
        return;
        //--------------------------
    }// end if (text.size() == 1UL)
    //--------------------------
    for (size_t cell = 0UL; cell < count; ++cell) {
        m_line.append(text.data(), text.size());
    }// end for (size_t cell = 0UL; cell < count; ++cell)
    //--------------------------
}// end void ProgressBar::ProgressBar::put_fill(size_t count, std::string_view text)
//--------------------------------------------------------------
template <typename T>
void ProgressBar::ProgressBar::put_digits(T value, size_t width, std::string_view fill) {
    //--------------------------
    using Unsigned = std::make_unsigned_t<T>;
    //--------------------------
    // Count the characters first, so std::to_chars writes straight into its place in the line buffer.
    //--------------------------
    Unsigned magnitude  = static_cast<Unsigned>(value);
    size_t length       = 0UL;
    //--------------------------
    if constexpr (std::is_signed_v<T>) {
        //--------------------------
        if (value < 0) {
            magnitude   = static_cast<Unsigned>(Unsigned{0} - magnitude);
            length      = 1UL;  // The minus sign.
        }// end if (value < 0)
        //--------------------------
    }// end if constexpr (std::is_signed_v<T>)
    //--------------------------
    for (size_t digit = 0UL; digit < static_cast<size_t>(std::numeric_limits<Unsigned>::digits10) + 1UL; ++digit) {
        //--------------------------
        ++length;
        magnitude /= 10U;
        //--------------------------
        if (magnitude == 0U) {
            break;
        }// end if (magnitude == 0U)
        //--------------------------
    }// end for (size_t digit = 0UL; digit < static_cast<size_t>(std::numeric_limits<Unsigned>::digits10) + 1UL; ++digit)
    //--------------------------
    put_fill(width > length ? width - length : 0UL, fill);  // The padding.
    //--------------------------
    const size_t used = m_line.size();
    //--------------------------
    m_line.resize(used + length);  // The digits are written over these characters in place.
    //--------------------------
    char* const first   = m_line.data() + used;
    const auto result   = std::to_chars(first, first + length, value);
    //--------------------------
    if (result.ec != std::errc{}) {
        //--------------------------
        // A number that cannot be written is shown as question marks of the same width; the line stays whole.
        //--------------------------
        m_line.replace(used, length, length, '?');
        //--------------------------
    }// end if (result.ec != std::errc{})
    //--------------------------
}// end void ProgressBar::ProgressBar::put_digits(T value, size_t width, std::string_view fill)
//--------------------------------------------------------------
void ProgressBar::ProgressBar::put_time(const std::chrono::milliseconds::rep& time, std::string_view label) {
    //--------------------------
    const TimeParts parts = split_time(time);
    //--------------------------
    put_text(label);
    put_text(" ");
    //--------------------------
#ifdef HAVE_FMT
    //--------------------------
    // days:hours:minutes:seconds:milliseconds, every field, as "{:02d}:{:02d}:{:02d}:{:02d}:{:03d}"
    //--------------------------
    put_digits(parts.days, 2UL, "0");
    put_text(":");
    put_digits(parts.hours, 2UL, "0");
    put_text(":");
    put_digits(parts.minutes, 2UL, "0");
    put_text(":");
    put_digits(parts.seconds, 2UL, "0");
    put_text(":");
    put_digits(parts.milliseconds, 3UL, "0");
    //--------------------------
#else
    //--------------------------
    // [days:][hh:][mm:]ss[:mmm ] with the leading fields only when needed
    //--------------------------
    if (parts.days > 0) {
        put_digits(parts.days, 0UL, "0");
        put_text(":");
    }// end if (parts.days > 0)
    //--------------------------
    if (parts.days > 0 or parts.hours > 0) {
        put_digits(parts.hours, 2UL, "0");
        put_text(":");
    }// end if (parts.days > 0 or parts.hours > 0)
    //--------------------------
    if (parts.days > 0 or parts.hours > 0 or parts.minutes > 0) {
        put_digits(parts.minutes, 2UL, "0");
        put_text(":");
    }// end if (parts.days > 0 or parts.hours > 0 or parts.minutes > 0)
    //--------------------------
    put_digits(parts.seconds, 2UL, "0");
    //--------------------------
    if (parts.milliseconds > 0) {
        put_text(":");
        put_digits(parts.milliseconds, 3UL, "0");
        put_text(" ");
    }// end if (parts.milliseconds > 0)
    //--------------------------
#endif
    //--------------------------
}// end void ProgressBar::ProgressBar::put_time(const std::chrono::milliseconds::rep& time, std::string_view label)
//--------------------------------------------------------------
#ifdef HAVE_FMT
    //--------------------------
    void ProgressBar::ProgressBar::display(void) {
        //--------------------------
        // One clock read per update, one line buffer written once.
        //--------------------------
        const auto now = std::chrono::steady_clock::now();
        //--------------------------
        m_line.clear();
        //--------------------------
        size_t position = 0UL, percent = 0UL;
        //--------------------------
        const bool determinate = (m_total != 0UL && m_total != std::numeric_limits<size_t>::max());
        //--------------------------
        if (determinate) {
            m_progress = std::clamp(m_progress, static_cast<size_t>(0UL), m_total);
            const double ratio = m_total > 0UL ? static_cast<double>(m_progress) / static_cast<double>(m_total) : 0.;
            percent = static_cast<size_t>(ratio * 100UL);
            position = static_cast<size_t>(static_cast<double>(m_bar_length) * ratio);
        } else {
            position = m_progress % m_bar_length;
        }// end if (determinate)
        //--------------------------
        //--------------------------
        put_text(STYLE_BOLD_ON);
        put_text("\x1b[A");
        put_text("\r");
        //--------------------------
        if (m_available_width < MIN_WIDTH) {
            //--------------------------
            put_digits(percent, 3UL, " ");
            put_text("% [");
            put_fill(position, m_progress_char);
            put_fill(m_bar_length - position, m_empty_space_char);
            put_text("]");
            //--------------------------
        } else {
            //--------------------------
            put_text(m_name);
            put_text(": ");
            put_digits(percent, 3UL, " ");
            put_text("% [");
            put_text(STYLE_GREEN_ON);
            put_fill(position, m_progress_char);
            put_text(STYLE_OFF);
            put_text(STYLE_RED_ON);
            put_fill(m_bar_length - position, m_empty_space_char);
            put_text(STYLE_OFF);
            put_text("]");
            put_fill(m_spaces_after_bar, " ");
            put_text(" ");
            //--------------------------
        }// end if (m_available_width < MIN_WIDTH)
        //--------------------------
        put_text(" \n");
        put_time(calculate_elapsed(now), "Elapsed:");
        put_text(" ");
        //--------------------------
        if (determinate) {
            put_time(calculate_etc(now), "ETC:");
        } else {
            put_text("ETC: N/A ");
        }// end if (determinate)
        //--------------------------
        put_text(STYLE_OFF);
        //--------------------------
        std::cout.write(m_line.data(), static_cast<std::streamsize>(m_line.size()));
        std::cout.flush();
        //--------------------------
    }// end void ProgressBar::ProgressBar::display(void)
//--------------------------------------------------------------
#else
//--------------------------
    void ProgressBar::ProgressBar::display(void) {
        //--------------------------
        // One clock read per update, one line buffer written once.
        //--------------------------
        const auto now = std::chrono::steady_clock::now();
        //--------------------------
        m_line.clear();
        //--------------------------
        const bool determinate      = (m_total != 0UL and m_total != std::numeric_limits<size_t>::max());
        size_t position = 0UL;
        //--------------------------
        put_text(ANSI_CURSOR_UP_ONE_LINE);  // ANSI escape code to move up one line.
        put_text("\r");
        //--------------------------
        if(determinate) {
            //--------------------------
            m_progress = std::clamp(m_progress, static_cast<size_t>(0UL), m_total);
            //--------------------------
            const double ratio      = m_total > 0UL ? static_cast<double>(m_progress) / static_cast<double>(m_total) : 0.;
            const size_t percent    = static_cast<size_t>(ratio * 100UL);
            position                = static_cast<size_t>(static_cast<double>(m_bar_length) * ratio);
            //--------------------------
            put_text(m_name);
            put_text(": ");
            put_digits(percent, 3UL, " ");
            put_text("% [" ANSI_BOLD_ON ANSI_COLOR_GREEN);
            put_fill(position, m_progress_char);
            put_text(ANSI_COLOR_RESET ANSI_COLOR_RED);
            put_fill(m_bar_length - position, m_empty_space_char);
            put_text(ANSI_COLOR_RESET);
            //--------------------------
        } else {
            //--------------------------
            position = m_progress % m_bar_length;
            //--------------------------
            // The cells before the position are empty, the cell at the position is the progress character.
            //--------------------------
            put_text(m_name);
            put_text(": [" ANSI_BOLD_ON ANSI_COLOR_GREEN);
            put_fill(position, m_empty_space_char);
            put_text(ANSI_COLOR_RESET ANSI_COLOR_RED);
            put_fill(1UL, m_progress_char);
            put_fill(m_bar_length - position - 1UL, m_empty_space_char);
            put_text(ANSI_COLOR_RESET);
            //--------------------------
        }// end if(determinate)
        //--------------------------
        put_text("] " ANSI_BOLD_OFF "\n");  // New line after the progress bar
        //--------------------------
        put_time(calculate_elapsed(now), "Elapsed:");
        //--------------------------
        if(determinate) {
            //--------------------------
            put_time(calculate_etc(now), "ETC:");
            //--------------------------
        }// end if(determinate)
        //--------------------------
        std::cout.write(m_line.data(), static_cast<std::streamsize>(m_line.size()));
        std::cout.flush();
        //--------------------------
    }// end void ProgressBar::ProgressBar::display(void)
    //--------------------------
#endif
//--------------------------------------------------------------
void ProgressBar::ProgressBar::tick(void) {
    //--------------------------
    if(m_total == std::numeric_limits<size_t>::max() or !is_done()) {
        //--------------------------
        ++m_progress;
        //--------------------------
    }// end if(m_total == std::numeric_limits<size_t>::max() or !is_done())
    //--------------------------
    // The line buffer is sized once, and again only when the name or the width needs more room.
    //--------------------------
    const size_t needed = m_name.size() + m_bar_length * std::max(m_progress_char.size(), m_empty_space_char.size()) + m_spaces_after_bar + LINE_SLACK;
    //--------------------------
    if (m_line.capacity() < needed) {
        m_line.reserve(needed);
    }// end if (m_line.capacity() < needed)
    //--------------------------
    display();
    //--------------------------
}// end void ProgressBar::ProgressBar::tick(void)
//--------------------------------------------------------------
inline bool ProgressBar::ProgressBar::is_done(void) const{
    //--------------------------
    return m_progress >= m_total;
    //--------------------------
}// end bool ProgressBar::ProgressBar::is_done(void)
//--------------------------------------------------------------
std::chrono::milliseconds::rep ProgressBar::ProgressBar::calculate_etc(const std::chrono::steady_clock::time_point& now) {
    //--------------------------
    if (!m_progress or !m_total) {
        return std::chrono::milliseconds::max().count();  // Indicate that ETC is not applicable
    }// end if (m_progress <= 0 or m_total <= 0)
    //--------------------------
#ifdef USE_BOOST
    static boost::circular_buffer<std::chrono::milliseconds::rep> m_delta_times(m_total > 2*CIRCULAR_BUFFER ? CIRCULAR_BUFFER : static_cast<size_t>(m_total/2));
#else
    static CircularBuffer::CircularBuffer<std::chrono::milliseconds::rep> m_delta_times( m_total > 2*CIRCULAR_BUFFER ? CIRCULAR_BUFFER : static_cast<size_t>(m_total/2));
#endif
    //--------------------------
    static auto m_last_etc = std::numeric_limits<double>::max();
    //--------------------------
    static uint8_t m_update_counter = 0U;
    //--------------------------
    static auto m_last_tick_time = m_start_time;
    //--------------------------
    const auto elapsed = calculate_elapsed(now);
    //--------------------------
    // Calculating overall ETA based on the overall progress and time
    //--------------------------
    auto overall_etc = elapsed * (m_total - m_progress) / m_progress; // This gives ETC in milliseconds
    //--------------------------
    // If it's the initial state or the buffer doesn't have enough entries,
    // we use the overall ETC calculated from the start.
    //--------------------------
    if (m_update_counter == 0 or m_delta_times.size() < CIRCULAR_BUFFER) {
        //--------------------------
        m_last_tick_time = now; // Update the last tick time
        m_last_etc = overall_etc; // Store the calculated ETC
        //--------------------------
        return overall_etc; // ETC in milliseconds
        //--------------------------
    }// end if (m_update_counter == 0 or m_delta_times.size() < CIRCULAR_BUFFER)
    //--------------------------
    if (++m_update_counter % UPDATE_INTERVAL == 0) {
        //--------------------------
        // Calculate time taken since last update
        //--------------------------
        auto elapsed_since_last = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_last_tick_time).count();
#ifdef USE_BOOST
        m_delta_times.push_back(std::move(elapsed_since_last));
    #ifdef HAVE_TBB
        // Calculate the average time between updates
        auto recent_avg_time = std::reduce(std::execution::par, m_delta_times.begin(), m_delta_times.end(), 0LL) / m_delta_times.size();
    #else
        // Calculate the average time between updates
        auto recent_avg_time = std::reduce(m_delta_times.begin(), m_delta_times.end(), 0LL) / m_delta_times.size();
    #endif
#else
        m_delta_times.push(std::move(elapsed_since_last));
        const double recent_avg_time = m_delta_times.mean().value_or(0.);
#endif
        //--------------------------
        // Use the recent average to estimate current ETC
        //--------------------------
        const double recent_etc     = recent_avg_time * (static_cast<double>(m_total) - static_cast<double>(m_progress)) / static_cast<double>(m_progress);
        //--------------------------
        // Average of overall and recent ETC
        //--------------------------
        const double combined_etc   = (static_cast<double>(overall_etc) + recent_etc) * 0.5; // Averaging ETCs
        //--------------------------
        m_last_tick_time    = now;          // Update the last tick time
        m_last_etc          = combined_etc; // Store the calculated ETC
        m_update_counter    = 0U;            // Reset the counter after updating
        //--------------------------
        return combined_etc; // Return combined ETC in milliseconds
        //--------------------------
    }// end if (++m_update_counter % UPDATE_INTERVAL == 0)
    //--------------------------
    // If not the time to update, return the last calculated ETC
    //--------------------------
    return m_last_etc; // ETC in milliseconds
    //--------------------------
}// end double ProgressBar::ProgressBar::calculate_etc(const std::chrono::steady_clock::time_point& now)
//--------------------------------------------------------------
inline std::chrono::milliseconds::rep ProgressBar::ProgressBar::calculate_elapsed(const std::chrono::steady_clock::time_point& now) const{
    //--------------------------
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - m_start_time).count();
    //--------------------------
}// end std::chrono::milliseconds::rep ProgressBar::ProgressBar::calculate_elapsed(const std::chrono::steady_clock::time_point& now)
//--------------------------------------------------------------
inline size_t ProgressBar::ProgressBar::get_terminal_width(void) {
#ifdef _WIN32
    //--------------------------
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    //--------------------------
    int columns;
    //--------------------------
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        //--------------------------
        columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        //--------------------------
    } else {
        //--------------------------
        columns = DEFAULT_WIDTH; // Default width if unable to get console size
        //--------------------------
    }// end if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
    //--------------------------
    return columns;
    //--------------------------
#else
    //--------------------------
    struct winsize size;
    //--------------------------
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1) {
        //--------------------------
        std::cerr << "Error getting terminal size: " << std::strerror(errno) << "\n";
        return DEFAULT_WIDTH;
        //--------------------------
    }// end if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1)
    //--------------------------
    //--------------------------
    // A terminal that reports no width (zero columns) gets the default width.
    //--------------------------
    return size.ws_col == 0U ? DEFAULT_WIDTH : size.ws_col;
    //--------------------------
#endif
}// end void ProgressBar::ProgressBar::print_terminal_width(void)
//--------------------------------------------------------------
void ProgressBar::ProgressBar::calculate_bar(void) {
    //--------------------------
    auto terminal_width = get_terminal_width();
    //--------------------------
    // Constants for fixed characters and escape sequences.
    //--------------------------
    static constexpr size_t fixed_characters   = 10UL;  // "100% []" and any padding.
    static constexpr size_t ansi_sequences     = 14UL;    // "\033[1m", "\033[32m", etc.
    //--------------------------
    const size_t fixed_and_name_characters  = m_name_length + fixed_characters;
    const size_t reserved_width              = fixed_and_name_characters + ansi_sequences;
    m_available_width                       = (terminal_width > reserved_width) ? (terminal_width - reserved_width) : 0UL;
    //--------------------------
    // Calculate the length of the bar based on BAR_PERCENTAGE.
    //--------------------------
    m_bar_length = static_cast<size_t>(m_available_width * BAR_PERCENTAGE);
    m_bar_length = std::max(m_bar_length, MIN_BAR_LENGTH);
    m_bar_length = (m_bar_length % 2UL == 0UL) ? m_bar_length : m_bar_length - 1UL;
    //--------------------------
    #ifdef HAVE_FMT
        //--------------------------
        m_spaces_after_bar = (m_available_width > m_bar_length) ? (m_available_width - m_bar_length) : 0UL;  // Spaces to provide visual separation.
        //--------------------------
    #endif
    //--------------------------
}// end void ProgressBar::ProgressBar::calculate_bar(void)
//--------------------------------------------------------------
inline void ProgressBar::ProgressBar::clear_lines(bool line){
    //--------------------------
    if(line){
#ifdef HAVE_FMT
    fmt::print("{}", ANSI_CURSOR_UP_ONE_LINE ANSI_CLEAR_LINE ANSI_CURSOR_UP_ONE_LINE ANSI_CLEAR_LINE);
#else
    std::cout << ANSI_CURSOR_UP_ONE_LINE ANSI_CLEAR_LINE ANSI_CURSOR_UP_ONE_LINE ANSI_CLEAR_LINE;  // ANSI escape sequences for "move up 1 line" and "clear line"
#endif
    } else {
    //--------------------------
#ifdef HAVE_FMT
    fmt::print("{}", ANSI_CLEAR_LINE);
#else
    std::cout << ANSI_CLEAR_LINE;  // ANSI escape sequences for "move up 1 line" and "clear line"
#endif
    }// END if(line)
    //--------------------------
}// end void ProgressBar::ProgressBar::clear_lines(void)
//--------------------------------------------------------------
#ifdef _WIN32
    //--------------------------
    BOOL WINAPI ProgressBar::ProgressBar::handle_console_signal(DWORD event) {
        //--------------------------
        if (event == CTRL_C_EVENT or event == CTRL_BREAK_EVENT or event == CTRL_CLOSE_EVENT or event == CTRL_LOGOFF_EVENT or event == CTRL_SHUTDOWN_EVENT) {
            //--------------------------
            clear_lines();
            //--------------------------
            calculate_bar();
            //--------------------------
            clear_lines(true);
            //--------------------------
        }//end if (event == CTRL_WINDOW_EVENT)
        //--------------------------
        return TRUE;
    }// end BOOL WINAPI ProgressBar::ProgressBar::handle_console_signal(DWORD event)
    //--------------------------
#else
    //--------------------------   
    void ProgressBar::ProgressBar::handle_winch_signal(int signum) {
        //--------------------------
        if (signum == SIGWINCH) {
            //--------------------------
            clear_lines();
            //--------------------------
            calculate_bar();
            //--------------------------
            clear_lines(true);
            //--------------------------
        }//end if (signum == SIGWINCH)
        //--------------------------
    }// end void ProgressBar::ProgressBar::handle_winch_signal(int signum)
#endif
//--------------------------------------------------------------