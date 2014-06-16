--
-- (C) 2014 - ntop.org
--
local debug = true


local table_key = "ntopng.prefs.table"

flow_table_key        = "flow"
host_table_key        = "host"


function tablePreferences(key, value)
  
  if (value == nil) then
    -- Get preferences
    return ntop.getHashCache(table_key, key)
  else
    -- Set preferences
    ntop.setHashCache(table_key, key, value)
  end

end


