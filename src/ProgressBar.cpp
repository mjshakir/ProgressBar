//--------------------------------------------------------------
// Main Header 
//--------------------------------------------------------------
#include "ProgressBar.hpp"
//--------------------------------------------------------------
// Standard cpp library
//--------------------------------------------------------------
#include <string>
#include <sstream>
#include <iomanip>
#include <climits>
#include <algorithm>
#ifdef HAVE_TBB
    #include <execution>
#endif
#include <stdexcept>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cerrno> // for errno
#include <cstring> // for strerror
#include <string_view>
#include <csignal>
#include <numeric>
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
#define CIRCULAR_BUFFER                 10
//--------------------------
// Interval Constant
//--------------------------
#define UPDATE_INTERVAL                 5
//--------------------------
// Bar Constant
//--------------------------
#define BAR_PERCENTAGE                  0.15
#define DEFAULT_WIDTH                   30
#define MIN_WIDTH                       10
constexpr size_t MIN_BAR_LENGTH =       15;       // Minimum length of the progress bar for visibility.
//--------------------------------------------------------------
size_t ProgressBar::ProgressBar::m_available_width = 0;
size_t ProgressBar::ProgressBar::m_name_length = 0;
size_t ProgressBar::ProgressBar::m_spaces_after_bar = 0;
size_t ProgressBar::ProgressBar::m_bar_length = 0;
//--------------------------------------------------------------
ProgressBar::ProgressBar::ProgressBar(  const std::string& name, 
                                        const std::string& progress_char, 
                                        const std::string& empty_space_char) :  m_total(std::numeric_limits<size_t>::max()),
                                                                                m_progress(0),
                                                                                m_name(name),
                                                                                m_progress_char(progress_char),
                                                                                m_empty_space_char(empty_space_char),
                                                                                m_start_time(std::chrono::steady_clock::now()){
    //--------------------------
    initializer(name);
    //--------------------------
}//end ProgressBar::ProgressBar::ProgressBar
//--------------------------------------------------------------
ProgressBar::ProgressBar::ProgressBar(  const size_t& total,
                                        const std::string& name, 
                                        const std::string& progress_char, 
                                        const std::string& empty_space_char) :  m_total(total),
                                                                                m_progress(0),
                                                                                m_name(name), 
                                                                                m_progress_char(progress_char), 
                                                                                m_empty_space_char(empty_space_char),
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
void ProgressBar::ProgressBar::initializer(const std::string& name) const{
    //--------------------------
    m_name_length = name.length();
    //--------------------------
    calculate_bar();
    //--------------------------
    std::signal(SIGWINCH, handle_winch_signal);
    //--------------------------
}// end void ProgressBar::ProgressBar::initializer(const std::string& name) const
//--------------------------------------------------------------
std::chrono::milliseconds::rep ProgressBar::ProgressBar::calculate_etc(void) {
    //--------------------------
    if (m_progress == 0 or m_total == 0) {
        return std::chrono::milliseconds::max().count();  // Indicate that ETC is not applicable
    }// end if (m_progress <= 0 or m_total <= 0)
    //--------------------------
#ifdef USE_BOOST
    static boost::circular_buffer<std::chrono::milliseconds::rep> m_delta_times(m_total > 2*CIRCULAR_BUFFER ? CIRCULAR_BUFFER : static_cast<size_t>(m_total/2));
#else
    static CircularBuffer<std::chrono::milliseconds::rep> m_delta_times( m_total > 2*CIRCULAR_BUFFER ? CIRCULAR_BUFFER : static_cast<size_t>(m_total/2));
#endif
    //--------------------------
    static auto m_last_etc = std::numeric_limits<double>::max();
    //--------------------------
    static uint8_t m_update_counter = 0;
    //--------------------------
    static std::chrono::steady_clock::time_point m_last_tick_time = std::chrono::steady_clock::now();
    //--------------------------
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start_time).count();
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
        auto recent_avg_time = m_delta_times.mean().value();
#endif
        //--------------------------
        // Use the recent average to estimate current ETC
        //--------------------------
        auto recent_etc = recent_avg_time * (m_total - m_progress) / m_progress;
        //--------------------------
        // Average of overall and recent ETC
        //--------------------------
        auto combined_etc = (overall_etc + recent_etc) / 2; // Averaging ETCs
        //--------------------------
        m_last_tick_time = now; // Update the last tick time
        m_last_etc = combined_etc; // Store the calculated ETC
        m_update_counter = 0; // Reset the counter after updating
        //--------------------------
        return combined_etc; // Return combined ETC in milliseconds
    }// end if (++m_update_counter % UPDATE_INTERVAL == 0)
    //--------------------------
    // If not the time to update, return the last calculated ETC
    return m_last_etc; // ETC in milliseconds
    //--------------------------
}// end double ProgressBar::ProgressBar::calculate_etc(void) 
//--------------------------------------------------------------
std::chrono::milliseconds::rep ProgressBar::ProgressBar::calculate_elapsed(void) {
    //--------------------------
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_start_time).count();
    //--------------------------
}// end double ProgressBar::ProgressBar::calculate_elapsed(void)
//--------------------------------------------------------------
#ifndef HAVE_FMT
    //--------------------------
    void ProgressBar::ProgressBar::append_time(std::stringstream& ss, const std::chrono::milliseconds::rep& time, const std::string& label) {
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
        // Create a string stream to format the output
        //--------------------------
        ss << label << " ";
        //--------------------------
        if (days_duration.count() > 0) {
            ss << days_duration.count() << ":";
        }// end if (days_duration.count() > 0)
        //--------------------------
        if (days_duration.count() > 0 or hours_duration.count() > 0) {
            ss << std::setw(2) << std::setfill('0') << hours_duration.count() << ":";
        }// end if (days_duration.count() > 0 or hours_duration.count() > 0)
        //--------------------------
        if (days_duration.count() > 0 or hours_duration.count() > 0 or minutes_duration.count() > 0) {
            ss << std::setw(2) << std::setfill('0') << minutes_duration.count() << ":";
        }// end if (days_duration.count() > 0 or hours_duration.count() > 0 or minutes_duration.count() > 0)
        //--------------------------
        ss << std::setw(2) << std::setfill('0') << seconds_duration.count();
        if (milliseconds_duration.count() > 0) {
            ss << ":" << std::setw(3) << std::setfill('0') << milliseconds_duration.count() << " ";
        }// end if (milliseconds_duration.count() > 0)
        //--------------------------
    }// end void ProgressBar::ProgressBar::append_time(std::stringstream& ss, double time, const std::string& label)
    //--------------------------
#endif
//--------------------------------------------------------------
#ifdef HAVE_FMT
    //--------------------------
    std::string ProgressBar::ProgressBar::append_time(const std::chrono::milliseconds::rep& time, const std::string& label) {
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
        // Format and return the string
        return fmt::format(
            "{} {:02d}:{:02d}:{:02d}:{:02d}:{:03d}", // formatting for days:hours:minutes:seconds:milliseconds
                        label, 
                        days_duration.count(), 
                        hours_duration.count(), 
                        minutes_duration.count(), 
                        seconds_duration.count(), 
                        milliseconds_duration.count());
        //--------------------------
    }// end std::string ProgressBar::ProgressBar::append_time(double time, const std::string& label)
    //--------------------------
#endif
//--------------------------------------------------------------
#ifdef HAVE_FMT
    //--------------------------
    void ProgressBar::ProgressBar::display(void) {
        //--------------------------
        size_t position = 0, percent = 0;
        //--------------------------
        if (m_total != 0 and m_total != std::numeric_limits<size_t>::max()) {
            m_progress = std::clamp(m_progress, static_cast<size_t>(0), m_total);
            auto ratio = m_total > 0 ? static_cast<double>(m_progress) / m_total : 0;
            percent = static_cast<size_t>(ratio * 100);
            position = static_cast<size_t>(m_bar_length * ratio);
        } else {
            position = m_progress % m_bar_length;
        }// end if (m_total != 0 and m_total != std::numeric_limits<size_t>::max())
        //--------------------------
        std::string completed_bar(position, m_progress_char[0]); // Constructs a string of 'position' count of progress chars.
        std::string incomplete_bar(m_bar_length - position, m_empty_space_char[0]); // Remaining part.
        //--------------------------
        std::string elapsed_time = append_time(calculate_elapsed(), "Elapsed:");
        std::string etc_time = (m_total != 0 and m_total != std::numeric_limits<size_t>::max()) ? append_time(calculate_etc(), "ETC:") : "ETC: N/A ";
        //--------------------------
        // Format the progress bar with colors.
        std::string green_part = fmt::format(fmt::emphasis::bold | fmt::fg(fmt::color::green), "{}", completed_bar);
        std::string red_part = fmt::format(fmt::emphasis::bold | fmt::fg(fmt::color::red), "{}", incomplete_bar);
        //--------------------------
        std::string colored_bar;
        colored_bar.reserve(green_part.size() + red_part.size() + 1);  // +1 for the "]" character
        colored_bar = green_part + red_part + "]";
        //--------------------------
        std::string formatted_bar = fmt::format("\r{}: {:3d}% [{}{} ", m_name, percent, colored_bar, std::string(m_spaces_after_bar, ' '));
        //--------------------------
        if (m_available_width < MIN_WIDTH) {
            formatted_bar = fmt::format("\r{:3d}% [{}]", percent, completed_bar + incomplete_bar);
        }// end if (m_available_width < MIN_WIDTH)
        //--------------------------
        std::string formatted_time = fmt::format("{} {}", elapsed_time, etc_time);
        //--------------------------
        // Use bold emphasis for printing.
        //--------------------------
        std::cout << fmt::format(fmt::emphasis::bold, "\x1b[A{} \n{}", formatted_bar, formatted_time);
        std::cout.flush();
        //--------------------------
    }// end void ProgressBar::ProgressBar::display(void)
//--------------------------------------------------------------
#else
//--------------------------
    void ProgressBar::ProgressBar::display(void) {
        //--------------------------
        std::stringstream ss;
        ss.str().reserve(200);
        size_t position = 0;
        //--------------------------
        ss << '\r';
        //--------------------------
        if(m_total != 0 and m_total != std::numeric_limits<size_t>::max()) {
            //--------------------------
            m_progress = std::clamp(m_progress, static_cast<size_t>(0), m_total);
            //--------------------------
            double ratio = m_total > 0 ? static_cast<double>(m_progress) / m_total : 0;
            auto percent = static_cast<size_t>(ratio * 100);
            position = m_bar_length * ratio;
            //--------------------------
            ss << m_name << ": " << std::setw(3) << percent << "% [" << ANSI_BOLD_ON;
            //--------------------------
            // Construct the progress string parts with repetitive character constructor.
            std::string progress_str(position, m_progress_char[0]);
            std::string remaining_str(m_bar_length - position, m_empty_space_char[0]);
            
            ss  << ANSI_COLOR_GREEN << progress_str << ANSI_COLOR_RESET
                << ANSI_COLOR_RED << remaining_str << ANSI_COLOR_RESET;
            //--------------------------
        } else {
            //--------------------------
            position = m_progress % m_bar_length;
            // Use string constructor to create the 'bar' string with set characters.
            std::string bar(m_bar_length, m_empty_space_char[0]);
            bar[position] = m_progress_char[0]; // Set the progress character
            
            ss  << m_name << ": [" << ANSI_BOLD_ON 
                << ANSI_COLOR_GREEN << bar.substr(0, position) << ANSI_COLOR_RESET
                << ANSI_COLOR_RED << bar.substr(position) << ANSI_COLOR_RESET;
            //--------------------------
        }// end if(m_total != 0 and m_total != std::numeric_limits<size_t>::max())
        //--------------------------
        ss << "] " << ANSI_BOLD_OFF << "\n";  // New line after the progress bar
        //--------------------------
        append_time(ss, calculate_elapsed(), "Elapsed:");
        //--------------------------
        if(m_total != 0 and m_total != std::numeric_limits<size_t>::max()) {
            //--------------------------
            append_time(ss, calculate_etc(), "ETC:");
            //--------------------------
        }// end if(m_total != 0 and m_total != std::numeric_limits<size_t>::max())
        //--------------------------
        std::cout << ANSI_CURSOR_UP_ONE_LINE;  // ANSI escape code to move up two lines.
        std::cout << ss.str();
        std::cout.flush();
        //--------------------------
    }// end void ProgressBar::display(void)
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
    display();
    //--------------------------
}// end void ProgressBar::ProgressBar::tick(void)
//--------------------------------------------------------------
bool ProgressBar::ProgressBar::is_done(void) const{
    //--------------------------
    return m_progress >= m_total;
    //--------------------------
}// end bool ProgressBar::ProgressBar::is_done(void)
//--------------------------------------------------------------
size_t ProgressBar::ProgressBar::get_terminal_width(void) {
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
    return size.ws_col;
    //--------------------------
}// end void ProgressBar::ProgressBar::print_terminal_width(void)
//--------------------------------------------------------------
void ProgressBar::ProgressBar::calculate_bar(void) {
    //--------------------------
    auto terminal_width = get_terminal_width();
    //--------------------------
    // Constants for fixed characters and escape sequences.
    //--------------------------
    constexpr size_t fixed_characters = 10;  // "100% []" and any padding.
    constexpr size_t ansi_sequences = 14;    // "\033[1m", "\033[32m", etc.
    //--------------------------
    size_t fixed_and_name_characters = m_name_length + fixed_characters;
    m_available_width = terminal_width - fixed_and_name_characters - ansi_sequences;
    //--------------------------
    // Calculate the length of the bar based on BAR_PERCENTAGE.
    //--------------------------
    m_bar_length = static_cast<size_t>(m_available_width * BAR_PERCENTAGE);
    m_bar_length = std::max(m_bar_length, MIN_BAR_LENGTH);
    m_bar_length = (m_bar_length % 2 == 0) ? m_bar_length : m_bar_length - 1;
    //--------------------------
    #ifdef HAVE_FMT
        //--------------------------
        m_spaces_after_bar = m_available_width - m_bar_length;  // Spaces to provide visual separation.
        //--------------------------
    #endif
    //--------------------------
}// end void ProgressBar::ProgressBar::calculate_bar(void)
//--------------------------------------------------------------
void ProgressBar::ProgressBar::clear_lines(bool line){
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
//--------------------------------------------------------------