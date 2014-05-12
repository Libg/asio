//
// thread_pool.hpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ASIO_THREAD_POOL_HPP
#define ASIO_THREAD_POOL_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include "asio/detail/config.hpp"
#include "asio/detail/noncopyable.hpp"
#include "asio/detail/task_io_service.hpp"
#include "asio/detail/thread_group.hpp"
#include "asio/execution_context.hpp"
#include "asio/is_executor.hpp"

#include "asio/detail/push_options.hpp"

namespace asio {

/// A simple fixed-size thread pool.
/**
 * The thread pool class is an execution context where functions are permitted
 * to run on one of a fixed number of threads.
 */
class thread_pool
  : public execution_context
{
public:
  class executor_type;

  /// Constructs a pool with an automatically determined number of threads.
  ASIO_DECL thread_pool();

  /// Constructs a pool with a specified number of threads.
  ASIO_DECL thread_pool(std::size_t num_threads);

  /// Obtains the executor associated with the pool.
  executor_type get_executor() const;

  /// Destructor.
  /**
   * Automatically stops and joins the pool, if not explicitly done beforehand.
   */
  ASIO_DECL ~thread_pool();

  /// Stops the threads.
  /**
   * This function stops the threads as soon as possible. As a result of calling
   * @c stop(), pending function objects may be never be invoked.
   */
  ASIO_DECL void stop();

  /// Joins the threads.
  /**
   * This function blocks until the threads in the pool have completed. If @c
   * stop() is not called prior to @c join(), the @c join() call will wait
   * until the pool has no more outstanding work.
   */
  ASIO_DECL void join();

private:
  friend class executor_type;
  struct thread_function;

  // The underlying scheduler.
  detail::task_io_service& scheduler_;

  // The threads in the pool.
  detail::thread_group threads_;
};

/// Executor used to submit functions to a thread pool.
class thread_pool::executor_type
{
public:
  /// Tracks outstanding work associated with the executor.
  class work;

  /// Obtain the underlying execution context.
  execution_context& context();

  /// Request the thread pool to invoke the given function object.
  /**
   * This function is used to ask the thread pool to execute the given function
   * object. If the current thread belongs to the pool, @c dispatch() executes
   * the function before returning. Otherwise, the function will be scheduled
   * to run on the thread pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   */
  template <typename Function>
  void dispatch(ASIO_MOVE_ARG(Function) f);

  /// Request the thread pool to invoke the given function object.
  /**
   * This function is used to ask the thread pool to execute the given function
   * object. The function object will never be executed inside @c post().
   * Instead, it will be scheduled to run on the thread pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   */
  template <typename Function>
  void post(ASIO_MOVE_ARG(Function) f);

  /// Request the thread pool to invoke the given function object.
  /**
   * This function is used to ask the thread pool to execute the given function
   * object. The function object will never be executed inside @c defer().
   * Instead, it will be scheduled to run on the thread pool.
   *
   * If the current thread belongs to the thread pool, @c defer() will delay
   * scheduling the function object until the current thread returns control to
   * the pool.
   *
   * @param f The function object to be called. The executor will make
   * a copy of the handler object as required. The function signature of the
   * function object must be: @code void function(); @endcode
   */
  template <typename Function>
  void defer(ASIO_MOVE_ARG(Function) f);

private:
  friend class thread_pool;

  // Constructor.
  explicit executor_type(thread_pool& p) : pool_(p) {}

  // The underlying thread pool.
  thread_pool& pool_;
};

/// Class to inform the thread pool when it has work to do.
/**
 * The work class is used to inform the thread pool when work starts and
 * finishes. This ensures that the thread pool's @c join() function will not
 * return while work is underway, and that it does return when there is no
 * unfinished work remaining.
 *
 * The work class is copy-constructible so that it may be used as a data member
 * in a handler class. It is not assignable.
 */
class thread_pool::executor_type::work
{
public:
  /// Constructor notifies the thread pool that work is starting.
  /**
   * The constructor is used to inform the thread pool that some work has
   * begun. This ensures that the thread pool's join() function will not return
   * while the work is underway.
   */
  explicit work(thread_pool::executor_type& e);

  /// Copy constructor notifies the thread pool that work is continuing.
  /**
   * The copy constructor is used to inform the thread pool that some work is
   * continuing. This ensures that the thread pool's join() function will not
   * return while the work is underway.
   */
  work(const work& other);

#if defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)
  /// Copy constructor notifies the thread pool that work is continuing.
  /**
   * The move constructor is used to inform the thread pool that some work is
   * continuing. This ensures that the thread pool's join() function will not
   * return while the work is underway.
   */
  work(work&& other);
#endif // defined(ASIO_HAS_MOVE) || defined(GENERATING_DOCUMENTATION)

  /// Destructor notifies the thread pool that the work is complete.
  /**
   * The destructor is used to inform the thread pool that some work has
   * finished. Once the count of unfinished work reaches zero, the thread
   * pool's join() function is permitted to exit.
   */
  ~work();

private:
  // Prevent assignment.
  void operator=(const work& other);

  // The underlying scheduler.
  detail::task_io_service& scheduler_;
};

#if !defined(GENERATING_DOCUMENTATION)
template <> struct is_executor<thread_pool::executor_type> : true_type {};
#endif // !defined(GENERATING_DOCUMENTATION)

} // namespace asio

#include "asio/detail/pop_options.hpp"

#include "asio/impl/thread_pool.hpp"
#if defined(ASIO_HEADER_ONLY)
# include "asio/impl/thread_pool.ipp"
#endif // defined(ASIO_HEADER_ONLY)

#endif // ASIO_THREAD_POOL_HPP
