#pragma once

//--------------------------------------------------------------
// Standard cpp library
//--------------------------------------------------------------
#include <cstddef>
#include <cstdbool>
#include <string>
#include <chrono>
//--------------------------------------------------------------
// Windows library
//--------------------------------------------------------------
#ifdef _WIN32
    #define NOMINMAX // Prevent windows.h from defining min and max macros
    #include <windows.h>
#endif
//--------------------------------------------------------------
namespace ProgressBar {
    //--------------------------------------------------------------
    /**
     * @class ProgressBar
     * @brief A class to display a progress bar in the console.
     *
     * @details The ProgressBar class is designed to provide a visual indication of progress
     * in console applications. It shows the current progress, the elapsed time,
     * and an estimate of the time to completion (ETC). The display is updated
     * in real-time as the progress is updated.
     */
    class ProgressBar {
        //--------------------------------------------------------------
        public:
            //--------------------------------------------------------------
            /**
             * @brief Deleted default constructor.
             * 
             * @details The default constructor is explicitly deleted to prevent creating a ProgressBar without a name.
             */
            ProgressBar(void)                           = delete;
            //--------------------------
            /**
             * @brief Construct a new ProgressBar with a given name, progress character, and empty space character.
             *
             * @details The progress bar is initialized to the maximum size representable by size_t, effectively making it
             * an indefinite progress bar until the total count is set.
             *
             * @param name The name of the task associated with the progress bar. Default is "Progress".
             * @param progress_char The character used to represent completed progress. Default is "#".
             * @param empty_space_char The character used to represent incomplete progress. Default is "-".
             *
             * @note This constructor initializes an indefinite progress bar.
             * 
             * @example:
             * @code{.cpp}
             * ProgressBar pb("Downloading", "=", " ");
             * while(....) {
             *     pb.update();
             * }
             * // Output:
             * // Downloading [=========================]
             * @endcode
             */
            explicit ProgressBar(   std::string_view name = "Progress", 
                                    std::string_view progress_char = "#", 
                                    std::string_view empty_space_char = "-");
            //--------------------------
            /**
             * @brief Construct a new ProgressBar with a total count, name, progress character, and empty space character.
             *
             * @details This constructor initializes the progress bar with a specific total count, which defines when the
             * progress bar will reach 100% upon updating.
             *
             * @param total The total number of steps to reach completion.
             * @param name The name of the task associated with the progress bar. Default is "Progress".
             * @param progress_char The character used to represent completed progress. Default is "#".
             * @param empty_space_char The character used to represent incomplete progress. Default is "-".
             *
             * @example:
             * @code{.cpp}
             * ProgressBar pb(100, "Downloading", "=", " ");
             * for(int i = 0; i <= 100; ++i) {
             *     pb.update();
             *     // Perform some work here
             * }
             * // Output:
             * // Downloading [=========================] 100%
             * @endcode
             */
            explicit ProgressBar(   const size_t& total,
                                    std::string_view name = "Progress", 
                                    std::string_view progress_char = "#", 
                                    std::string_view empty_space_char = "-");
            //--------------------------
            ProgressBar           (ProgressBar const&) = default;
            ProgressBar& operator=(ProgressBar const&) = delete;
            //--------------------------
            ProgressBar           (ProgressBar&&)      = default;
            ProgressBar& operator=(ProgressBar&&)      = delete;
            //--------------------------
            ~ProgressBar(void)                         = default;
            //--------------------------
            /**
             * @brief Increments the progress count and updates the display.
             *
             * @details This function should be called whenever a unit of work is completed. It increments the internal
             * progress counter and updates the visual display of the progress bar in the console. If the total
             * work is set to `std::numeric_limits<size_t>::max()`, this function can be used to display an indefinite
             * progress bar that will update until the `done()` function is called.
             *
             * @note This function does nothing if the progress has already reached the total count.
             *
             * @example:
             * @code{.cpp}
             * // Suppose we have a task that processes 100 items.
             * ProgressBar pb(100, "Processing", "=", " ");
             * for (int i = 0; i < 100; ++i) {
             *     // Perform some unit of work here...
             *     pb.update(); // Increment the progress bar and update the display
             * }
             * // Once the loop is completed, the progress bar would display 100% completion.
             * @endcode
             *
             * @example: for an indefinite progress bar:
             * @code{.cpp}
             * // For a task with an unknown number of items, the ProgressBar can be used indefinitely.
             * ProgressBar pb("Working", "+", "-");
             * while (//some condition) {
             *     // Perform some unit of work here...
             *     pb.update(); // Increment the progress bar and update the display
             *     // The bar will keep updating without reaching 100% until the loop condition is false.
             * }
             * @endcode
             */
            void update(void);
            //--------------------------
            /**
             * @brief Checks if the progress has reached the total count.
             *
             * @details This function returns a boolean indicating whether the progress has reached or exceeded
             * the total work count that was set when the ProgressBar was initialized. If the progress
             * has reached the total, it returns true, otherwise it returns false. This is typically used
             * to determine if a task is completed when looping through a set number of iterations.
             *
             * @return True if the progress is complete, false otherwise.
             *
             * @example:
             * @code{.cpp}
             * // Initialize a ProgressBar for 100 units of work.
             * ProgressBar pb(100, "Processing", "=", " ");
             * while (!pb.is_done()) {
             *     // Perform some unit of work here...
             *     pb.tick(); // Increment the progress bar and update the display.
             * }
             * // When the loop exits, it indicates the ProgressBar has reached 100%.
             * @endcode
             *
             * @note In cases where the total count is `std::numeric_limits<size_t>::max()`,
             * `done()` will always return false unless manually checked against a condition.
             */
            bool done(void);
            //--------------------------------------------------------------
        protected:
            //--------------------------------------------------------------
            void initializer(std::string_view name) const;
            //--------------------------
            #ifndef HAVE_FMT
                //--------------------------
                void append_time(std::ostringstream& ss, const std::chrono::milliseconds::rep& time, std::string_view label);
                //--------------------------
            #endif
            //--------------------------
            #ifdef HAVE_FMT
                //--------------------------
                std::string append_time(const std::chrono::milliseconds::rep& time, std::string_view label);
                //--------------------------
            #endif
            //--------------------------
            void display(void);
            //--------------------------
            void tick(void);
            //--------------------------
            inline bool is_done(void) const ;
            //--------------------------
            std::chrono::milliseconds::rep calculate_etc(void);
            //--------------------------
            inline std::chrono::milliseconds::rep calculate_elapsed(void) const;
            //--------------------------
            static inline size_t get_terminal_width(void);
            //--------------------------
            static void calculate_bar(void); 
            //--------------------------------------------------------------
        private:
            //--------------------------------------------------------------
            const size_t m_total;
            size_t m_progress;
            //--------------------------
            const std::string m_name, m_progress_char, m_empty_space_char;
            //--------------------------
            const std::chrono::steady_clock::time_point m_start_time;
            //--------------------------
            static size_t m_name_length, m_bar_length, m_available_width, m_spaces_after_bar;
            //--------------------------
            static inline void clear_lines(bool line = false);
            //--------------------------
#ifdef _WIN32
            static BOOL WINAPI handle_console_signal(DWORD event);
#else
            static void handle_winch_signal(int signum);
#endif
        //--------------------------------------------------------------
    };// end class ProgressBar
    //--------------------------------------------------------------
}//end namespace ProgressBar
//--------------------------------------------------------------