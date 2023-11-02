#pragma once

//--------------------------------------------------------------
// Standard cpp library
//--------------------------------------------------------------
#include <iostream>
#include <deque>
#include <vector>
#include <type_traits>
#include <numeric>
#include <algorithm>
#include <optional>
#include <cmath>
#ifdef HAVE_TBB
    #include <execution>
#endif
//--------------------------------------------------------------
namespace ProgressBar{
    //--------------------------------------------------------------
    /**
     * @brief CircularBuffer is a template class that represents a circular buffer of a fixed size.
     *
     * @details When the buffer is full and a new element is inserted, the oldest element is overwritten.
     * The CircularBuffer class is useful for keeping a history of values up to a certain limit,
     * such as for rolling statistics or buffers for audio processing.
     *
     * @tparam T The type of elements stored in the buffer.
     */
    template <typename T>
    class CircularBuffer {
        //--------------------------------------------------------------
        public:
            //--------------------------------------------------------------
            /**
             * @brief Construct a new Circular Buffer object.
             *
             * The constructor initializes the buffer with a maximum size. Elements added to the buffer
             * will fill the buffer until the maximum size is reached; thereafter, new elements will
             * replace the oldest ones.
             *
             * @param size The maximum size of the circular buffer.
             *
             * @note This constructor is explicit to prevent accidental conversions from size_t to CircularBuffer.
             *
             * @example
             * @details ProgressBar::CircularBuffer<int> buffer(10);
             * // Fill the buffer
             * for(int i = 0; i < 10; ++i) {
             *     buffer.push(i);
             * }
             */
            explicit CircularBuffer(size_t size) : m_max_size(size) {
                //--------------------------
            }// end explicit CircularBuffer(size_t size)
            //--------------------------
            CircularBuffer           (CircularBuffer const&) = default;
            CircularBuffer& operator=(CircularBuffer const&) = default;
            //--------------------------
            CircularBuffer           (CircularBuffer&&)      = default;
            CircularBuffer& operator=(CircularBuffer&&)      = default;
            //--------------------------
            CircularBuffer(void)                             = delete;
            //--------------------------
            ~CircularBuffer(void)                            = default;
            //--------------------------
            /**
             * @brief Adds an element to the end of the buffer.
             * 
             * @details This method adds a copy of the given item to the end of the buffer. If the buffer has reached 
             * its maximum size, the oldest element (at the front) will be removed before adding the new element.
             * This maintains the fixed size of the buffer, ensuring it acts as a circular buffer.
             * 
             * @tparam T The type of elements stored in the buffer.
             * @param item A reference to the item to be added to the buffer.
             * 
             * @note The method uses copy semantics. If move semantics are desired for efficiency with large or
             * non-copyable types, consider using push(T&& item) instead.
             * 
             * @example
             * 
             * ProgressBar::CircularBuffer<int> buffer(5);
             * int value = 42;
             * 
             * // Using copy semantics
             * buffer.push(value);
             * 
             * // Using move semantics with a temporary
             * buffer.push(43);
             * 
             * // The buffer now contains 42, 43
             */
            void push(const T& item) {
                push_back(item);
            }// end void push(const T& item) 
            //--------------------------
            /**
             * @brief Adds an element to the end of the buffer using move semantics.
             * 
             * @details This method moves the given item into the buffer, which can be more efficient than copying for
             * large or non-copyable types. If the buffer has reached its maximum size, the oldest element
             * (at the front) will be removed before adding the new element. This maintains the fixed size
             * of the buffer, ensuring it acts as a circular buffer.
             * 
             * @tparam T The type of elements stored in the buffer.
             * @param item An r-value reference to the item to be added to the buffer.
             * 
             * @note The method uses move semantics, which is more efficient for types that support it,
             * but leaves the moved-from object in an unspecified state.
             * 
             * @example
             * 
             * ProgressBar::CircularBuffer<int> buffer(5);
             * int value = 42;
             * 
             * // Using copy semantics
             * buffer.push(value);
             * 
             * // Using move semantics with a temporary
             * buffer.push(43);
             * 
             * // The buffer now contains 42, 43
             */
            void push(T&& item) {
                push_back(std::move(item));
            }// end void push(T&& item)
            //--------------------------
            /**
             * @brief Constructs an element in-place at the end of the buffer.
             * 
             * @details This method creates an element at the end of the buffer using the provided arguments. It forwards
             * the arguments to the constructor of the element type, which allows for in-place construction 
             * without the need to copy or move the element into the buffer. If the buffer has reached its 
             * maximum size, the oldest element (at the front) is removed before the new element is constructed.
             * This maintains the fixed size of the buffer, ensuring it acts as a circular buffer.
             * 
             * @tparam Args Variadic template arguments corresponding to the constructor parameters
             *              of the element type.
             * @param args Arguments to forward to the constructor of the element.
             * 
             * @note This method is useful when you want to construct an object in-place, avoiding the cost
             *       of copying or moving the object. This is particularly useful for objects that are expensive
             *       to copy or cannot be copied at all.
             * 
             * @example
             * 
             * CircularBuffer<MyClass> buffer(5);
             * 
             * // Construct an object of MyClass in-place with an int and a double argument.
             * buffer.emplace(42, 3.14);
             * 
             * // For a class that has a constructor like MyClass(int, double),
             * // The buffer now contains an instance of MyClass constructed with 42 and 3.14
             */
            template <typename... Args>
            void emplace(Args&&... args) {
                emplace_back(std::forward<Args>(args)...);
            }// end void emplace(Args&&... args)
            //--------------------------
            /**
             * @brief Removes and returns the element at the front of the buffer.
             * 
             * @details This method removes the element at the front of the buffer and returns it.
             * If the buffer is empty, it returns `std::nullopt` to indicate that there
             * are no elements to pop. This method provides a safe way to attempt to remove
             * and retrieve an element without the risk of undefined behavior from popping
             * from an empty container.
             * 
             * @tparam T The type of the elements stored in the buffer.
             * @return std::optional<T> An optional containing the value of the front element
             *         if the buffer is not empty, or `std::nullopt` if the buffer is empty.
             * 
             * @note The method returns an std::optional<T> instead of throwing an exception
             *       or returning a sentinel value, providing a way to check for success or
             *       failure without exception handling.
             * 
             * @example
             * 
             * CircularBuffer<int> buffer;
             * buffer.push(10);
             * buffer.push(20);
             * 
             * auto front = buffer.pop(); // front now holds std::optional with value 10
             * if(front) {
             *     std::cout << "Front value: " << *front << std::endl;
             * } else {
             *     std::cout << "Buffer was empty!" << std::endl;
             * }
             * 
             * front = buffer.pop(); // front now holds std::optional with value 20
             * front = buffer.pop(); // front now holds std::nullopt, buffer is empty
             */
            std::optional<T> pop(void) {
                return pop_front();
            }// end std::optional<T> pop(void)
            //--------------------------
            /**
             * @brief Calculates the arithmetic mean of the elements if the buffer type is arithmetic.
             * 
             * @details Computes the average value of the arithmetic elements contained within the buffer. This method
             * is enabled only for buffers where the element type is arithmetic (i.e., integral or floating-point types).
             * If the buffer is empty, it returns `std::nullopt` to indicate that no average can be computed.
             * The computation is performed in parallel using TBB if available and enabled (`HAVE_TBB` is defined).
             * 
             * @tparam U The type of the elements (defaults to T, the template parameter of the buffer).
             *           This parameter is used to enable this method only for arithmetic types.
             * @return std::optional<double> An optional containing the mean value as double if the buffer
             *         is not empty and U is an arithmetic type, or `std::nullopt` otherwise.
             * 
             * @note The use of `std::reduce` with execution policies requires C++17 or later and is only
             *       enabled if `HAVE_TBB` is defined. Without TBB, the reduction is performed sequentially.
             * 
             * @example
             * 
             * // Assuming CircularBuffer is a template class that includes mean function.
             * CircularBuffer<int> intBuffer;
             * intBuffer.push(1);
             * intBuffer.push(2);
             * intBuffer.push(3);
             * 
             * auto meanValue = intBuffer.mean(); // meanValue will hold std::optional with the value 2.0
             * if (meanValue) {
             *     std::cout << "Mean value: " << *meanValue << std::endl;
             * } else {
             *     std::cout << "Buffer is empty or type U is not arithmetic!" << std::endl;
             * }
             * 
             * CircularBuffer<std::string> stringBuffer; // This instantiation will not compile if mean is attempted to be used
             * // because std::string is not an arithmetic type.
             */
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> mean(void) const {
                return mean_local();
            }// end std::optional<T> mean(void) const 
            //--------------------------
            /**
             * @brief Calculates the median of the elements if the buffer type is arithmetic.
             * 
             * @details This function calculates the median value of the arithmetic elements contained within the buffer.
             * The function is enabled only for buffers where the element type is arithmetic.
             * The median is determined by sorting the buffer and then selecting the middle value
             * (or the average of the two middle values if the buffer size is even).
             * If the buffer is empty, it returns `std::nullopt` to indicate the absence of a median.
             * The sorting is done in parallel using TBB if available and enabled (`HAVE_TBB` is defined).
             * 
             * @tparam U The type of the elements (defaults to T, the template parameter of the buffer).
             *           This template parameter is used to enable this method only for arithmetic types.
             * @return std::optional<double> An optional containing the median value as a double if the buffer
             *         is not empty and U is an arithmetic type, or `std::nullopt` otherwise.
             * 
             * @note The use of `std::sort` with execution policies requires C++17 or later and is only
             *       enabled if `HAVE_TBB` is defined. Without TBB, the sorting is performed sequentially.
             * 
             * @example
             * 
             * // Assuming CircularBuffer is a template class that includes median function.
             * CircularBuffer<int> intBuffer;
             * intBuffer.push(3);
             * intBuffer.push(1);
             * intBuffer.push(2);
             * 
             * auto medianValue = intBuffer.median(); // medianValue will hold std::optional with the value 2.0
             * if (medianValue) {
             *     std::cout << "Median value: " << *medianValue << std::endl;
             * } else {
             *     std::cout << "Buffer is empty or type U is not arithmetic!" << std::endl;
             * }
             * 
             * CircularBuffer<std::string> stringBuffer; // This instantiation will not compile if median_local is attempted to be used
             * // because std::string is not an arithmetic type.
             */
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> median(void) const {
                return median_local();
            }// end std::optional<T> median(void) const
            //--------------------------
            /**
             * @brief Calculates the minimum element of the buffer if the element type is arithmetic.
             * 
             * @details This function searches for and returns the minimum value contained in an arithmetic buffer.
             * The function will only compile if the type U is arithmetic, thanks to SFINAE (Substitution Failure Is Not An Error).
             * The calculation is done by using `std::min_element` to find the smallest element.
             * If `HAVE_TBB` is defined, the search is done in parallel, otherwise, it's done sequentially.
             * If the buffer is empty, `std::nullopt` is returned.
             *
             * @tparam U The type of the elements (defaults to T, the template parameter of the buffer).
             *           The method is enabled only if U is an arithmetic type.
             * @return std::optional<U> The minimum element in the buffer wrapped in an std::optional,
             *         or `std::nullopt` if the buffer is empty.
             * 
             * @note This function requires TBB for parallel execution, enabled with `HAVE_TBB`. If TBB is not
             *       available, or if `HAVE_TBB` is not defined, the function will revert to sequential execution.
             * 
             * @example
             * 
             * // Assuming CircularBuffer is a template class that includes minimum function.
             * CircularBuffer<int> intBuffer;
             * intBuffer.push(10);
             * intBuffer.push(5);
             * intBuffer.push(7);
             * 
             * auto minValue = intBuffer.minimum(); // minValue will hold std::optional with the value 5
             * if (minValue) {
             *     std::cout << "Minimum value: " << *minValue << std::endl;
             * } else {
             *     std::cout << "Buffer is empty!" << std::endl;
             * }
             * 
             * CircularBuffer<std::string> stringBuffer; // This instantiation will not compile if minimum is attempted to be used
             * // because std::string is not an arithmetic type.
             */
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<U>> minimum(void) const {
                return minimum_local();
            }// end std::optional<T> minimum(void) const
            //--------------------------
            /**
             * @brief Finds the maximum element in the buffer if the elements are of arithmetic type.
             *
             * @details This function is enabled only if the buffer contains arithmetic types (thanks to SFINAE).
             * It calculates the maximum value present in the buffer using `std::max_element`.
             * If parallelism is enabled (through `HAVE_TBB`), the operation is performed in parallel.
             * If the buffer is empty, the function returns `std::nullopt`.
             *
             * @tparam U The element type (defaults to T, the template parameter of the buffer).
             *           The function is enabled only for arithmetic types.
             * @return std::optional<U> The maximum value found in the buffer, wrapped in an optional;
             *                          `std::nullopt` if the buffer is empty.
             * 
             * @note Parallelism is supported if `HAVE_TBB` is defined and TBB (Threading Building Blocks) is used.
             *       If TBB is not available or `HAVE_TBB` is not defined, it falls back to a sequential execution.
             * 
             * @example
             * 
             * // Assuming `CircularBuffer` is the name of the class template which contains this function.
             * CircularBuffer<double> buffer;
             * buffer.push(3.14);
             * buffer.push(2.71);
             * buffer.push(1.41);
             * 
             * auto maxVal = buffer.maximum(); // maxVal would hold an optional with the value 3.14
             * if (maxVal.has_value()) {
             *     std::cout << "Maximum value: " << maxVal.value() << std::endl;
             * } else {
             *     std::cout << "The buffer is empty." << std::endl;
             * }
             *
             * CircularBuffer<std::string> stringBuffer; // Will not compile if maximum is called
             * // since std::string is not an arithmetic type.
             */
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<U>> maximum(void) const {
                return maximum_local();
            }// end std::optional<T> maximum(void) const
            //--------------------------
            /**
             * @brief Computes the variance of the elements in the buffer.
             *
             * @details This method calculates the variance of the arithmetic elements present in the buffer.
             * Variance is a measure of the dispersion of the elements relative to their mean.
             * This function is enabled only for arithmetic types via SFINAE.
             * The variance is calculated using the `std::transform_reduce` algorithm.
             * For an empty buffer, it returns `std::nullopt`.
             *
             * @tparam U The element type (defaults to T, the template parameter of the buffer).
             *           It must be an arithmetic type to enable this method.
             * @return std::optional<double> The variance of the buffer's elements, wrapped in an optional;
             *                               `std::nullopt` if the buffer is empty.
             *
             * @note When `HAVE_TBB` is defined, the computation is carried out in parallel using Intel TBB.
             *       Without TBB, it falls back to sequential execution.
             * 
             * @example
             * 
             * // Assuming `CircularBuffer` is a template class that contains this function.
             * CircularBuffer<int> buffer;
             * buffer.push(1);
             * buffer.push(2);
             * buffer.push(3);
             * 
             * auto var = buffer.variance(); // var would hold an optional with the value 0.666...
             * if (var.has_value()) {
             *     std::cout << "Variance: " << var.value() << std::endl;
             * } else {
             *     std::cout << "The buffer is empty." << std::endl;
             * }
             *
             * CircularBuffer<std::string> stringBuffer; // Will not compile, as strings are not arithmetic types.
             */
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> variance(void) const {
                //--------------------------
                return variance_local();
                //--------------------------
            }// end std::optional<double> variance(void) const
            //--------------------------
            /**
             * @brief Computes the standard deviation of the elements in the buffer.
             *
             * @details This method calculates the standard deviation of the arithmetic elements present in the buffer.
             * Standard deviation is a measure of the amount of variation or dispersion of a set of values.
             * This function is enabled only for arithmetic types via SFINAE (Substitution Failure Is Not An Error).
             * The standard deviation is derived from the variance computed by the `variance_local` method.
             * If the buffer is empty, the result is `std::nullopt`.
             *
             * @tparam U The element type (defaults to T, the template parameter of the buffer).
             *           Must be an arithmetic type to enable this method.
             * @return std::optional<double> The standard deviation of the buffer's elements, wrapped in an optional;
             *                               `std::nullopt` if the buffer is empty or variance cannot be calculated.
             *
             * @example
             * 
             * // Assuming `CircularBuffer` is a template class that contains this function.
             * CircularBuffer<int> buffer;
             * buffer.push(4);
             * buffer.push(8);
             * buffer.push(6);
             * 
             * auto sd = buffer.standardDeviation(); // sd would hold an optional with the calculated standard deviation
             * if (sd.has_value()) {
             *     std::cout << "Standard Deviation: " << sd.value() << std::endl;
             * } else {
             *     std::cout << "The buffer is empty or variance is not available." << std::endl;
             * }
             *
             * CircularBuffer<std::string> stringBuffer; // Will not compile, as strings are not arithmetic types.
             */
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> standardDeviation(void) const {
                return standardDeviation_local();
            }// end std::optional<double> standardDeviation(void) const
            //--------------------------
            /**
             * @brief Checks if the buffer is empty.
             *
             * @details This method is used to query whether the buffer contains any elements.
             * @return true If the buffer is empty. false If the buffer contains one or more elements.
             *
             * @example
             * CircularBuffer<int> buffer;
             * // ... some operations that may or may not empty the buffer ...
             * if (buffer.empty()) {
             *     std::cout << "Buffer is empty." << std::endl;
             * } else {
             *     std::cout << "Buffer is not empty." << std::endl;
             * }
             */
            bool empty(void) const {
                return m_buffer.empty();
            }// end bool empty(void) const
            //--------------------------
            /**
             * @brief Retrieves the number of elements in the buffer.
             *
             * @details This method returns the current number of elements stored in the buffer.
             * @return size_t The number of elements in the buffer.
             *
             * @example
             * CircularBuffer<int> buffer;
             * // ... operations to add elements to the buffer ...
             * std::cout << "The buffer contains " << buffer.size() << " elements." << std::endl;
             */
            size_t size(void) const {
                return m_buffer.size();
            } // end size_t size(void) const
            //--------------------------
            /**
             * @brief Clears all elements from the buffer.
             *
             * @details This method will remove all the elements from the buffer, effectively resetting it.
             * @note After a reset, `empty()` will return true and `size()` will return 0.
             *
             * @example
             * CircularBuffer<int> buffer;
             * // ... operations to add elements to the buffer ...
             * buffer.reset();
             * // Now buffer.empty() returns true and buffer.size() returns 0.
             */
            void reset(void) {
                m_buffer.clear();
            }// end void reset(void)
            //--------------------------
            /**
             * @brief Gets an iterator pointing to the first element in the buffer.
             *
             * @details This iterator will allow both reading and modifying the elements of the buffer.
             * @return An iterator to the beginning of the buffer.
             *
             * @example
             * CircularBuffer<int> buffer;
             * for (auto it = buffer.begin(); it != buffer.end(); ++it) {
             *     *it = ... // Modify the buffer's contents
             * }
             */
            typename std::deque<T>::iterator begin(void) {
                return m_buffer.begin();
            }//end typename std::deque<T>::iterator begin(void)
            //--------------------------
            /**
             * @brief Gets an iterator referring to the past-the-end element in the buffer.
             *
             * @details The iterator does not refer to any element, and thus shall not be dereferenced.
             * @return An iterator to the element following the last element of the buffer.
             *
             * @example
             * CircularBuffer<int> buffer;
             * for (auto it = buffer.begin(); it != buffer.end(); ++it) {
             *     // Use *it...
             * }
             */
            typename std::deque<T>::iterator end(void) {
                return m_buffer.end();
            }// end typename std::deque<T>::iterator
            //--------------------------
            /**
             * @brief Gets a constant iterator pointing to the first element in the buffer.
             *
             * @details This iterator will only allow reading elements of the buffer, not modifying them.
             * @return A constant iterator to the beginning of the buffer.
             *
             * @example
             * const CircularBuffer<int> buffer;
             * for (auto it = buffer.begin(); it != buffer.end(); ++it) {
             *     std::cout << *it << std::endl; // Display the buffer's contents
             * }
             */
            typename std::deque<T>::const_iterator begin(void) const {
                return m_buffer.begin();
            }// end typename std::deque<T>::const_iterator begin(void) const
            //--------------------------
            /**
             * @brief Gets a constant iterator referring to the past-the-end element in the buffer.
             *
             * @details The iterator shall not be dereferenced as it does not refer to any element.
             * @return A constant iterator to the element following the last element of the buffer.
             *
             * @example
             * const CircularBuffer<int> buffer;
             * for (auto it = buffer.begin(); it != buffer.end(); ++it) {
             *     // Display *it...
             * }
             */
            typename std::deque<T>::const_iterator end(void) const {
                return m_buffer.end();
            }//end typename std::deque<T>::const_iterator end(void) const
            //--------------------------
            /**
             * @brief Gets a constant iterator pointing to the first element in the buffer.
             *
             * @details This is the constant version of begin() and can be used where a constant iterator is required.
             * @return A constant iterator to the beginning of the buffer.
             *
             * @example
             * const CircularBuffer<int> buffer;
             * for (auto it = buffer.cbegin(); it != buffer.cend(); ++it) {
             *     std::cout << *it << std::endl; // Read-only access to display the buffer's contents
             * }
             */
            typename std::deque<T>::const_iterator cbegin(void) const {
                return m_buffer.cbegin();
            }//end typename std::deque<T>::const_iterator cbegin(void) const
            //--------------------------
            /**
             * @brief Gets a constant iterator referring to the past-the-end element in the buffer.
             *
             * @details This is the constant version of end() and can be used where a constant iterator is required.
             * @return A constant iterator to the element following the last element of the buffer.
             *
             * @example
             * const CircularBuffer<int> buffer;
             * for (auto it = buffer.cbegin(); it != buffer.cend(); ++it) {
             *     // Read-only access to display *it...
             * }
             */
            typename std::deque<T>::const_iterator cend(void) const {
                return m_buffer.cend();
            }//end typename std::deque<T>::const_iterator cend(void) const
            //--------------------------
        protected:
            //--------------------------
            void push_back(const T& item) {
                //--------------------------
                if (m_buffer.size() == m_max_size) {
                    m_buffer.pop_front();
                }// end if (m_buffer.size() == m_max_size)
                //--------------------------
                m_buffer.push_back(item);
                //--------------------------
            }// end void push_back(const T& item)
            //--------------------------
            void push_back(T&& item) {
                //--------------------------
                if (m_buffer.size() == m_max_size) {
                    m_buffer.pop_front();
                }// end if (m_buffer.size() == m_max_size)
                //--------------------------
                m_buffer.push_back(std::move(item));
                //--------------------------
            }// end void push_back(T&& item)
            //--------------------------
            template <typename... Args>
            void emplace_back(Args&&... args) {
                //--------------------------
                if (m_buffer.size() == m_max_size) {
                    m_buffer.pop_front();
                }//end if (m_buffer.size() == m_max_size)
                //--------------------------
                m_buffer.emplace_back(std::forward<Args>(args)...);
                //--------------------------
            }// end void emplace_back(Args&&... args)
            //--------------------------
            std::optional<T> pop_front(void) {
                //--------------------------
                if (m_buffer.empty()) {
                    return std::nullopt;
                }// end if (m_buffer.empty())
                //--------------------------
                T value = m_buffer.front();
                m_buffer.pop_front();
                //--------------------------
                return value;
                //--------------------------
            }// end std::optional<T> pop_front(void)
            //--------------------------------------------------------------
            // Math
            //--------------------------
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> mean_local(void) const {
                //--------------------------
                if (m_buffer.empty()) {return std::nullopt;}
                //--------------------------
#ifdef HAVE_TBB
                return std::reduce(std::execution::par, m_buffer.begin(), m_buffer.end(), 0.0) / static_cast<double>(m_buffer.size());
#else
                return std::reduce(m_buffer.begin(), m_buffer.end(), 0.0) / static_cast<double>(m_buffer.size());
#endif
                //--------------------------
            }// end std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> mean_local(void) const
            //--------------------------------------------------------------
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> median_local(void) const {
                //--------------------------
                if (m_buffer.empty()){ return std::nullopt;}
                //--------------------------
                std::vector<T> sorted(m_buffer.begin(), m_buffer.end());
#ifdef HAVE_TBB
                std::sort(std::execution::par, sorted.begin(), sorted.end());
#else
                std::sort(sorted.begin(), sorted.end());
#endif
                //--------------------------
                auto middle = static_cast<size_t>(sorted.size() / 2);
                //--------------------------
                if (sorted.size() % 2 == 0) {
                    return static_cast<double>((sorted.at(middle - 1) + sorted.at(middle))) / 2.;
                } else {
                    return static_cast<double>(sorted.at(middle));
                }// end if (sorted.size() % 2 == 0)
                //--------------------------
            }// end std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> median_local(void) const
            //--------------------------------------------------------------
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<U>> minimum_local(void) const {
                //--------------------------
                if (m_buffer.empty()){return std::nullopt;}
                //--------------------------
#ifdef HAVE_TBB
                return *std::min_element(std::execution::par, m_buffer.begin(), m_buffer.end());
#else
                return *std::min_element(m_buffer.begin(), m_buffer.end());
#endif
                //--------------------------
            }// end std::enable_if_t<std::is_arithmetic<U>::value, std::optional<U>> minimum_local(void) const
            //--------------------------------------------------------------
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<U>> maximum_local(void) const {
                //--------------------------
                if (m_buffer.empty()){return std::nullopt;}
                //--------------------------
#ifdef HAVE_TBB
                return *std::max_element(std::execution::par, m_buffer.begin(), m_buffer.end());
#else
                return *std::max_element(m_buffer.begin(), m_buffer.end());
#endif
                //--------------------------
            }// end std::enable_if_t<std::is_arithmetic<U>::value, std::optional<U>> maximum_local(void) const
            //--------------------------
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> variance_local(void) const {
                //--------------------------
                if (m_buffer.empty()){return std::nullopt;}
                //--------------------------
                auto meanValue = static_cast<double>(mean().value_or(T()));
                double var = 0.;
                //--------------------------
#ifdef HAVE_TBB
                var = std::transform_reduce(std::execution::par, m_buffer.begin(), m_buffer.end(), 0., std::plus<>(),
                                            [&meanValue](const T& value) {return std::pow(value - meanValue, 2);});
#else
                var = std::transform_reduce(m_buffer.begin(), m_buffer.end(), 0.,std::plus<>(),
                                            [&meanValue](const T& value) {return std::pow(value - meanValue, 2);});
#endif
                //--------------------------
                return var / m_buffer.size();
                //--------------------------
            }// end std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> variance_local(void) const
            //--------------------------
            template <typename U = T>
            std::enable_if_t<std::is_arithmetic<U>::value, std::optional<double>> standardDeviation_local(void) const {
                //--------------------------
                auto var = variance_local();
                //--------------------------
                if (!var){return std::nullopt;}
                //--------------------------
                return std::sqrt(var.value());
                //--------------------------
            }// end std::optional<double> standardDeviation_local(void) const
            //--------------------------
        private:
            //--------------------------
            size_t m_max_size;
            std::deque<T> m_buffer;
        //--------------------------------------------------------------
    };// end class CircularBuffer
    //--------------------------------------------------------------
}// end namespace ProgressBar
//--------------------------------------------------------------