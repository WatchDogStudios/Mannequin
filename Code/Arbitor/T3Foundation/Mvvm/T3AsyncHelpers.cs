/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Threading;
using System.Threading.Tasks;

namespace T3Foundation.Mvvm
{
  /// <summary>
  /// Debounces an action so it only fires after a quiet period.
  /// Resets the timer on every call. Useful for search-as-you-type filters.
  /// </summary>
  public sealed class DebounceAction : IDisposable
  {
    private readonly Action _action;
    private readonly int _delayMs;
    private Timer? _timer;

    public DebounceAction(Action action, int delayMs = 300)
    {
      _action = action ?? throw new ArgumentNullException(nameof(action));
      _delayMs = delayMs;
    }

    /// <summary>
    /// Schedule (or reschedule) the debounced action.
    /// </summary>
    public void Invoke()
    {
      _timer?.Dispose();
      _timer = new Timer(_ =>
      {
        try { _action(); }
        catch (Exception ex) { T3Core.Log($"DebounceAction error: {ex.Message}", T3LogLevel.Error); }
      }, null, _delayMs, Timeout.Infinite);
    }

    public void Dispose()
    {
      _timer?.Dispose();
      _timer = null;
    }
  }

  /// <summary>
  /// Throttles an action so it fires at most once per interval.
  /// </summary>
  public sealed class ThrottleAction : IDisposable
  {
    private readonly Action _action;
    private readonly int _intervalMs;
    private Timer? _timer;
    private bool _pending;
    private readonly object _lock = new();

    public ThrottleAction(Action action, int intervalMs = 300)
    {
      _action = action ?? throw new ArgumentNullException(nameof(action));
      _intervalMs = intervalMs;
    }

    /// <summary>
    /// Request the throttled action. If the interval has not elapsed,
    /// the action will fire once the interval expires.
    /// </summary>
    public void Invoke()
    {
      lock (_lock)
      {
        if (_timer == null)
        {
          Fire();
          _timer = new Timer(_ => OnTimerElapsed(), null, _intervalMs, Timeout.Infinite);
        }
        else
        {
          _pending = true;
        }
      }
    }

    private void OnTimerElapsed()
    {
      lock (_lock)
      {
        if (_pending)
        {
          _pending = false;
          Fire();
          _timer?.Change(_intervalMs, Timeout.Infinite);
        }
        else
        {
          _timer?.Dispose();
          _timer = null;
        }
      }
    }

    private void Fire()
    {
      try { _action(); }
      catch (Exception ex) { T3Core.Log($"ThrottleAction error: {ex.Message}", T3LogLevel.Error); }
    }

    public void Dispose()
    {
      lock (_lock)
      {
        _timer?.Dispose();
        _timer = null;
      }
    }
  }

  /// <summary>
  /// Lightweight async lock using SemaphoreSlim. Usage:
  /// <code>using (await asyncLock.LockAsync()) { ... }</code>
  /// </summary>
  public sealed class AsyncLock
  {
    private readonly SemaphoreSlim _semaphore = new(1, 1);

    public async Task<IDisposable> LockAsync(CancellationToken ct = default)
    {
      await _semaphore.WaitAsync(ct);
      return new Releaser(_semaphore);
    }

    private sealed class Releaser : IDisposable
    {
      private readonly SemaphoreSlim _semaphore;
      public Releaser(SemaphoreSlim semaphore) => _semaphore = semaphore;
      public void Dispose() => _semaphore.Release();
    }
  }

  /// <summary>
  /// Extension methods for Task.
  /// </summary>
  public static class TaskExtensions
  {
    /// <summary>
    /// Fire-and-forget a task, logging any exceptions via T3Core.
    /// </summary>
    public static async void FireAndForget(this Task task, Action<Exception>? onError = null)
    {
      try
      {
        await task;
      }
      catch (Exception ex)
      {
        T3Core.Log($"FireAndForget exception: {ex.Message}", T3LogLevel.Error);
        onError?.Invoke(ex);
      }
    }

    /// <summary>
    /// Attach a cancellation token to an arbitrary task.
    /// </summary>
    public static async Task WithCancellation(this Task task, CancellationToken ct)
    {
      var tcs = new TaskCompletionSource<bool>();
      using (ct.Register(() => tcs.TrySetCanceled()))
      {
        var completed = await Task.WhenAny(task, tcs.Task);
        if (completed == tcs.Task)
          throw new OperationCanceledException(ct);
        await task; // propagate exceptions
      }
    }

    /// <summary>
    /// Attach a cancellation token to an arbitrary task with a result.
    /// </summary>
    public static async Task<T> WithCancellation<T>(this Task<T> task, CancellationToken ct)
    {
      var tcs = new TaskCompletionSource<bool>();
      using (ct.Register(() => tcs.TrySetCanceled()))
      {
        var completed = await Task.WhenAny(task, tcs.Task);
        if (completed == tcs.Task)
          throw new OperationCanceledException(ct);
        return await task;
      }
    }
  }
}
