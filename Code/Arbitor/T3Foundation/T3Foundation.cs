#nullable enable

using System;
using System.Collections.Generic;
using System.Linq;

namespace T3Foundation
{
  /// <summary>
  /// Core initialization and configuration for the T3 Framework.
  /// Provides centralized services: logging, configuration, and backend management.
  /// </summary>
  public static class T3Core
  {
    private static readonly List<string> s_LogMessages = new List<string>();
    private static readonly object s_LogLock = new object();
    private static bool s_Initialized;

    /// <summary>
    /// Event raised when a new log message is added.
    /// </summary>
    public static event Action<string, T3LogLevel>? OnLogMessage;

    /// <summary>
    /// Whether the framework has been initialized.
    /// </summary>
    public static bool IsInitialized => s_Initialized;

    /// <summary>
    /// Initialize the T3 Framework core services.
    /// </summary>
    public static void Initialize()
    {
      if (s_Initialized) return;
      s_Initialized = true;
      Log("T3 Framework initialized.", T3LogLevel.Info);
    }

    /// <summary>
    /// Gracefully shut down the T3 Framework.
    /// Clears log buffers and resets state.
    /// </summary>
    public static void Shutdown()
    {
      if (!s_Initialized) return;
      Log("T3 Framework shutting down.", T3LogLevel.Info);
      s_Initialized = false;
    }

    /// <summary>
    /// Log a message to the framework log.
    /// </summary>
    public static void Log(string message, T3LogLevel level = T3LogLevel.Info)
    {
      string formatted = $"[{DateTime.Now:HH:mm:ss.fff}] [{level}] {message}";
      lock (s_LogLock)
      {
        s_LogMessages.Add(formatted);
      }
      OnLogMessage?.Invoke(formatted, level);
    }

    /// <summary>
    /// Get all logged messages.
    /// </summary>
    public static IReadOnlyList<string> GetLogMessages()
    {
      lock (s_LogLock)
      {
        return s_LogMessages.ToList().AsReadOnly();
      }
    }

    /// <summary>
    /// Clear all logged messages.
    /// </summary>
    public static void ClearLog()
    {
      lock (s_LogLock)
      {
        s_LogMessages.Clear();
      }
    }
  }

  /// <summary>
  /// Severity levels for framework logging.
  /// </summary>
  public enum T3LogLevel
  {
    Debug,
    Info,
    Warning,
    Error,
    Critical
  }
}
