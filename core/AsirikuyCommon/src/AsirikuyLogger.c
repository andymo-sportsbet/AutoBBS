/**
 * @file
 * @brief     Simple file logger implementation with severity levels
 * 
 * @author    Auto (Implementation)
 * @version   F4.x.x
 * @date      2025
 */

#include <stdlib.h>
#include "AsirikuyLogger.h"
#include "CriticalSection.h"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#ifdef _OPENMP
#include <omp.h>
#endif

// Maximum number of log files (support multiple loggers)
#define MAX_LOG_FILES 4

// Logger state
static FILE* gLogFiles[MAX_LOG_FILES] = {NULL, NULL, NULL, NULL};
static int gSeverityLevel = LOG_INFO; // Default to Info level
static BOOL gInitialized = FALSE;

// Thread-local storage for per-thread log files
// Each thread can have its own log file to avoid critical section contention
#if defined __APPLE__ || defined __linux__
  static __thread FILE* threadLocalLogFile = NULL;
  static __thread int threadLocalSeverityLevel = -1; // -1 means not initialized
  static __thread unsigned int threadLocalLogCounter = 0; // Counter for periodic flushing
#elif defined _WIN32 || defined _WIN64
  static __declspec(thread) FILE* threadLocalLogFile = NULL;
  static __declspec(thread) int threadLocalSeverityLevel = -1;
  static __declspec(thread) unsigned int threadLocalLogCounter = 0;
#else
  // Fallback: use a simple per-thread array (limited to MAX_THREADS)
  #define MAX_THREADS 64
  static FILE* threadLocalLogFiles[MAX_THREADS] = {NULL};
  static int threadLocalSeverityLevels[MAX_THREADS] = {-1};
  static unsigned int threadLocalLogCounters[MAX_THREADS] = {0};
#endif

// Flush frequency: flush every N messages or immediately for critical messages
// Balance between I/O performance and data safety:
// - Lower value (10-20): More frequent flushes, safer but more I/O
// - Higher value (50-100): Less frequent flushes, better performance but risk losing more messages on crash
// Current: 20 messages provides ~95% I/O reduction while maintaining reasonable safety
#define LOG_FLUSH_INTERVAL 20

// Get severity level label
static const char* getSeverityLabel(int severity)
{
  switch(severity)
  {
    case LOG_EMERGENCY: return "EMERGENCY";
    case LOG_ALERT:     return "ALERT";
    case LOG_CRITICAL:  return "CRITICAL";
    case LOG_ERROR:     return "ERROR";
    case LOG_WARNING:   return "WARNING";
    case LOG_NOTICE:    return "NOTICE";
    case LOG_INFO:      return "INFO";
    case LOG_DEBUG:     return "DEBUG";
    default:            return "UNKNOWN";
  }
}

// Get current timestamp string (thread-safe)
static void getTimestamp(char* buffer, size_t bufferSize)
{
  time_t now;
  struct tm timeinfo;
  
  time(&now);
  
#if defined _WIN32 || defined _WIN64
  localtime_s(&timeinfo, &now);
  snprintf(buffer, bufferSize, "%04d-%02d-%02d %02d:%02d:%02d",
           timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
           timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
#else
  localtime_r(&now, &timeinfo);
  strftime(buffer, bufferSize, "%Y-%m-%d %H:%M:%S", &timeinfo);
#endif
}

// Helper function to create directory if it doesn't exist
static void ensureDirectoryExists(const char* filePath)
{
  char dirPath[MAX_FILE_PATH_CHARS] = "";
  char* lastSlash = NULL;
  
  strncpy(dirPath, filePath, MAX_FILE_PATH_CHARS - 1);
  dirPath[MAX_FILE_PATH_CHARS - 1] = '\0';
  
  // Find last slash
  lastSlash = strrchr(dirPath, '/');
#if defined _WIN32 || defined _WIN64
  if(lastSlash == NULL)
  {
    lastSlash = strrchr(dirPath, '\\');
  }
#endif
  
  if(lastSlash != NULL)
  {
    *lastSlash = '\0';
    
    // Create directory
#if defined _WIN32 || defined _WIN64
    CreateDirectoryA(dirPath, NULL);
#elif defined __APPLE__ || defined __linux__
    char mkdirCmd[MAX_FILE_PATH_CHARS + 10] = "";
    snprintf(mkdirCmd, sizeof(mkdirCmd), "mkdir -p \"%s\"", dirPath);
    system(mkdirCmd);
#endif
  }
}

int asirikuyLoggerInit(const char* pLogFilePath, int severityLevel)
{
  // Create test file to verify this function is called
  FILE* testFile = fopen("/tmp/asirikuyLoggerInit_called.log", "a");
  if(testFile != NULL)
  {
    fprintf(testFile, "asirikuyLoggerInit called: path='%s', severity=%d\n", 
            pLogFilePath ? pLogFilePath : "NULL", severityLevel);
    fclose(testFile);
  }
  
  // Debug: Log initialization attempt
  fprintf(stderr, "[DEBUG] asirikuyLoggerInit called: path='%s', severity=%d\n", 
          pLogFilePath ? pLogFilePath : "NULL", severityLevel);
  fflush(stderr);
  
  // Thread-safe access to shared logger state
  enterCriticalSection();
  
  // Update severity level (use lowest/most restrictive severity if multiple loggers)
  // Lower numbers = more restrictive (only critical errors), higher numbers = less restrictive (everything)
  // If this is the first initialization or the new severity is more restrictive, use it
  if(!gInitialized || severityLevel < gSeverityLevel)
  {
    gSeverityLevel = severityLevel;
  }

  // Open log file if path provided
  if(pLogFilePath != NULL && strlen(pLogFilePath) > 0)
  {
    fprintf(stderr, "[DEBUG] asirikuyLoggerInit: Opening log file: %s\n", pLogFilePath);
    fflush(stderr);
    // Check if this log file is already open
    int i;
    for(i = 0; i < MAX_LOG_FILES; i++)
    {
      if(gLogFiles[i] != NULL)
      {
        // Check if this is the same file (simple string comparison)
        // Note: This is a simple check - in practice, we'd need to track file paths
        // For now, we'll allow multiple files to be opened
      }
    }
    
    // Find an empty slot for the new log file
    int slot = -1;
    for(i = 0; i < MAX_LOG_FILES; i++)
    {
      if(gLogFiles[i] == NULL)
      {
        slot = i;
        break;
      }
    }
    
    if(slot == -1)
    {
      // All slots full, replace the first one (or we could append to existing)
      fprintf(stderr, "[WARNING] Maximum log files (%d) reached. Reusing first slot.\n", MAX_LOG_FILES);
      if(gLogFiles[0] != NULL && gLogFiles[0] != stderr)
      {
        fclose(gLogFiles[0]);
      }
      slot = 0;
    }
    
    // Ensure directory exists
    ensureDirectoryExists(pLogFilePath);
    
    // Open log file in append mode
    gLogFiles[slot] = fopen(pLogFilePath, "a");
    if(gLogFiles[slot] == NULL)
    {
      // If append fails, try write mode
      gLogFiles[slot] = fopen(pLogFilePath, "w");
    }
    
    if(gLogFiles[slot] != NULL)
    {
      fprintf(stderr, "[DEBUG] asirikuyLoggerInit: Successfully opened log file in slot %d: %s\n", slot, pLogFilePath);
      fflush(stderr);
      // Write header (bypass severity check for initialization)
      char timestamp[32] = "";
      getTimestamp(timestamp, sizeof(timestamp));
      fprintf(gLogFiles[slot], "\n=== Asirikuy Logger Started ===\n");
      fprintf(gLogFiles[slot], "[%s] Log file: %s\n", timestamp, pLogFilePath);
      fprintf(gLogFiles[slot], "[%s] Severity level: %d (%s)\n", timestamp, severityLevel, getSeverityLabel(severityLevel));
      fprintf(gLogFiles[slot], "[%s] All messages with severity <= %d will be logged\n", timestamp, severityLevel);
      fflush(gLogFiles[slot]);
    }
    else
    {
      fprintf(stderr, "[WARNING] Failed to open log file: %s. Logging to stderr only.\n", pLogFilePath);
      fprintf(stderr, "[WARNING] Error details: errno=%d, path='%s'\n", errno, pLogFilePath);
      fflush(stderr);
      gLogFiles[slot] = NULL;
    }
  }

  gInitialized = TRUE;
  
  leaveCriticalSection();
  return 0;
}

int asirikuyLoggerInitThreadLocal(const char* pLogFilePath, int severityLevel)
{
  // Debug: Log initialization attempt
  fprintf(stderr, "[DEBUG] asirikuyLoggerInitThreadLocal called: path='%s', severity=%d\n", 
          pLogFilePath ? pLogFilePath : "NULL", severityLevel);
  fflush(stderr);

  // Close existing thread-local log file if any
  if(threadLocalLogFile != NULL)
  {
    fclose(threadLocalLogFile);
    threadLocalLogFile = NULL;
  }

  // Set thread-local severity level
  threadLocalSeverityLevel = severityLevel;

  // Open thread-local log file if path provided
  if(pLogFilePath != NULL && strlen(pLogFilePath) > 0)
  {
    fprintf(stderr, "[DEBUG] asirikuyLoggerInitThreadLocal: Opening thread-local log file: %s\n", pLogFilePath);
    fflush(stderr);
    
    // Ensure directory exists
    ensureDirectoryExists(pLogFilePath);
    
    // Open log file in append mode
    threadLocalLogFile = fopen(pLogFilePath, "a");
    if(threadLocalLogFile == NULL)
    {
      // If append fails, try write mode
      threadLocalLogFile = fopen(pLogFilePath, "w");
    }
    
    if(threadLocalLogFile != NULL)
    {
      fprintf(stderr, "[DEBUG] asirikuyLoggerInitThreadLocal: Successfully opened thread-local log file: %s\n", pLogFilePath);
      fflush(stderr);
      // Write header
      char timestamp[32] = "";
      getTimestamp(timestamp, sizeof(timestamp));
      fprintf(threadLocalLogFile, "\n=== Asirikuy Thread-Local Logger Started ===\n");
      fprintf(threadLocalLogFile, "[%s] Thread ID: ", timestamp);
      #ifdef _OPENMP
      fprintf(threadLocalLogFile, "%d", omp_get_thread_num());
      #else
      fprintf(threadLocalLogFile, "main");
      #endif
      fprintf(threadLocalLogFile, "\n");
      fprintf(threadLocalLogFile, "[%s] Log file: %s\n", timestamp, pLogFilePath);
      fprintf(threadLocalLogFile, "[%s] Severity level: %d (%s)\n", timestamp, severityLevel, getSeverityLabel(severityLevel));
      fflush(threadLocalLogFile);
    }
    else
    {
      fprintf(stderr, "[WARNING] Failed to open thread-local log file: %s. Logging to stderr only.\n", pLogFilePath);
      fprintf(stderr, "[WARNING] Error details: errno=%d, path='%s'\n", errno, pLogFilePath);
      fflush(stderr);
    }
  }
  else
  {
    fprintf(stderr, "[DEBUG] asirikuyLoggerInitThreadLocal: No file path provided, using stderr only\n");
    fflush(stderr);
  }

  return 0;
}

void asirikuyLoggerCloseThreadLocal(void)
{
  if(threadLocalLogFile != NULL)
  {
    // Flush any remaining buffered log data before closing
    fflush(threadLocalLogFile);
    fclose(threadLocalLogFile);
    threadLocalLogFile = NULL;
  }
  threadLocalSeverityLevel = -1;
  threadLocalLogCounter = 0; // Reset counter
}

void asirikuyLogMessage(int severity, const char* format, ...)
{
  va_list args;
  char timestamp[32] = "";
  char messageBuffer[1024] = "";
  // Increased buffer size to accommodate timestamp (32) + severity label (10) + message (1024) + formatting (50) + null terminator
  char logLine[2048] = "";
  
  // Get timestamp (thread-safe)
  getTimestamp(timestamp, sizeof(timestamp));
  
  // Format the message
  va_start(args, format);
  vsnprintf(messageBuffer, sizeof(messageBuffer) - 1, format, args);
  messageBuffer[sizeof(messageBuffer) - 1] = '\0';
  va_end(args);
  
  // Format the full log line with timestamp and severity
  // Use snprintf with explicit size limit to prevent overflow
  int written = snprintf(logLine, sizeof(logLine) - 1, "[%s] [%s] %s", 
                         timestamp, getSeverityLabel(severity), messageBuffer);
  if (written < 0 || written >= (int)sizeof(logLine)) {
    // Truncation occurred or error - ensure null termination
    logLine[sizeof(logLine) - 1] = '\0';
  }
  
  // Ensure newline
  size_t len = strlen(logLine);
  if(len > 0 && logLine[len - 1] != '\n')
  {
    if(len < sizeof(logLine) - 1)
    {
      logLine[len] = '\n';
      logLine[len + 1] = '\0';
    }
  }
  
  // Check thread-local logger first (no lock needed!)
  // Only active if explicitly initialized (multi-threaded optimization)
  if(threadLocalSeverityLevel >= 0 && severity <= threadLocalSeverityLevel)
  {
    if(threadLocalLogFile != NULL)
    {
      // Write to thread-local file (NO CRITICAL SECTION!)
      fprintf(threadLocalLogFile, "%s", logLine);
      
      // Increment counter for periodic flushing
      threadLocalLogCounter++;
      
      // Flush immediately for critical messages, or periodically to reduce I/O overhead
      // This significantly reduces disk I/O while ensuring critical messages are written immediately
      if(severity <= LOG_ERROR || threadLocalLogCounter >= LOG_FLUSH_INTERVAL)
      {
        fflush(threadLocalLogFile);
        threadLocalLogCounter = 0; // Reset counter after flush
      }
      
      // Also write to stderr for critical messages
      if(severity <= LOG_ERROR)
      {
        fprintf(stderr, "%s", logLine);
        fflush(stderr);
      }
      
      // Thread-local logging complete, return early
      return;
    }
  }
  
  // Fall back to global logger (NO critical section needed)
  // Used for single-threaded runs or when thread-local not initialized
  // Note: asirikuyLoggerInit() already protects initialization with critical section.
  // Runtime logging is single-threaded (no concurrent access to gLogFiles[] or gSeverityLevel),
  // so no synchronization needed here.
  
  // Check if this severity level should be logged
  if(severity > gSeverityLevel)
  {
    return; // Skip logging for levels above the threshold
  }
  
  // Write to stderr (always, for critical messages)
  if(severity <= LOG_ERROR)
  {
    fprintf(stderr, "%s", logLine);
  }
  
  // Write to all open log files (no lock needed - single-threaded runtime)
  int i;
  for(i = 0; i < MAX_LOG_FILES; i++)
  {
    if(gLogFiles[i] != NULL && gLogFiles[i] != stderr)
    {
      fprintf(gLogFiles[i], "%s", logLine);
      fflush(gLogFiles[i]); // Ensure immediate write
    }
  }
}

