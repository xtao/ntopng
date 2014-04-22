--
-- (C) 2014 - ntop.org
--

-- Trace Level
TRACE_LEVEL = 0


-- Login & session
degub_login = false
debug_session = false
debug_host = false

-------------------------------- Trace Event ----------------------------------
-- Trace level
TRACE_ERROR    = 0
TRACE_WARNING  = 1
TRACE_NORMAL   = 2
TRACE_INFO     = 3
TRACE_DEBUG    = 6

MAX_TRACE_LEVEL = 6
-- Trace mode
TRACE_CONSOLE = 0
TRACE_WEB = 1

function traceError(p_trace_level, p_trace_mode,p_message)
  currentline = debug.getinfo(2).currentline
  what =  debug.getinfo(2).what
  src =  debug.getinfo(2).short_src
  filename = src
  for str in (string.gmatch(src, '([^/]+)')) do
    filename = str
  end
  date = os.date("%d/%b/%Y %X")

  trace_prefix = ''

  if (p_trace_level == TRACE_ERROR) then trace_prefix = 'ERROR: ' end
  if (p_trace_level == TRACE_WARNING) then trace_prefix = 'WARNING: ' end
  if (p_trace_level == TRACE_INFO) then trace_prefix = 'INFO: ' end
  if (p_trace_level == TRACE_DEBUG) then trace_prefix = 'DEBUG: ' end

  if ((p_trace_level <= MAX_TRACE_LEVEL) and (p_trace_level <= TRACE_LEVEL) )then
    if (p_trace_mode == TRACE_WEB) then
      print(date..' ['..filename..':'..currentline..'] '..trace_prefix..p_message)
    elseif (p_trace_mode == TRACE_CONSOLE) then
      io.write(date..' ['..filename..':'..currentline..'] '..trace_prefix..p_message)
    end
  end
end

function setTraceLevel(p_trace_level) 
  if (p_trace_level <= MAX_TRACE_LEVEL) then
    TRACE_LEVEL = p_trace_level
  end
end

function resetTraceLevel()
  TRACE_LEVEL = 0
end

--------------------------------