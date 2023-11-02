#include "ProgressBar.hpp" // Include the ProgressBar header
#include <thread>  // Used for simulating work (sleep)
#include <chrono>  // Used for timing and sleeping

int main(void) {
    // Start timing the execution with progress bar
    auto start_with_bar = std::chrono::high_resolution_clock::now();

    // Example 1: Basic usage with a predefined total
    {
        constexpr size_t total = 100;
        ProgressBar::ProgressBar bar(total, "Example quick constant time test", "#", "-");

        for (size_t i = 0; i <= total; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Simulate work
            bar.update(); // Update the progress bar
        }
    }

    // Example 2: Testing different characters for progress and empty space
    {
        constexpr size_t total = 500;
        ProgressBar::ProgressBar bar(total, "Example long constant time test", "=", " ");

        for (size_t i = 0; i <= total; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate longer work
            bar.update(); // Update the progress bar
        }
    }

    // Example 3: Dynamic update time test
    {
        constexpr size_t total = 500;
        ProgressBar::ProgressBar bar(total, "Example adding time test", "-", "-");

        for (size_t i = 0; i <= total; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1 + i)); // Simulate incrementally longer work
            bar.update(); // Update the progress bar
        }
    }

    // Example 4: Infinite progress bar (without predefined total)
    {
        ProgressBar::ProgressBar bar("Example without total", "#", "-");

        // Loop to simulate an operation without a known endpoint
        for (size_t i = 0; i <= 150; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Simulate consistent work
            bar.update(); // Update the progress bar
            // Condition to exit the loop must be defined according to actual task completion
        }
    }

    auto end_with_bar = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_with_bar = end_with_bar - start_with_bar;

    std::cout << "\n\nTesting without the progress bar" << std::endl;

    // Repeat the work without progress bar updates to get the base time for comparison
    auto start_without_bar = std::chrono::high_resolution_clock::now();

    // Simulate the same work as above but without progress bar updates
    {
        for (size_t i = 0; i <= 100; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
    {
        constexpr size_t total = 500;
        for (size_t i = 0; i <= total; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate longer work
        }
    }
    {
        constexpr size_t total = 500;
        for (size_t i = 0; i <= total; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1 + i)); // Simulate incrementally longer work
        }
    }

    // Example 4: Infinite progress bar (without predefined total)
    {
        // Loop to simulate an operation without a known endpoint
        for (size_t i = 0; i <= 150; ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Simulate consistent work
        }
    }

    auto end_without_bar = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> elapsed_without_bar = end_without_bar - start_without_bar;

    // Calculate and display the overhead introduced by the progress bar
    double overhead_time = elapsed_with_bar.count() - elapsed_without_bar.count();
    std::cout << "\nExecution time with progress bar: " << elapsed_with_bar.count() << " ms\n";
    std::cout << "Execution time without progress bar: " << elapsed_without_bar.count() << " ms\n";
    std::cout << "Overhead due to progress bar: " << overhead_time << " ms\n";

    // Terminate the program
    return 0;
}// end int main(void)