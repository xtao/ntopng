--
-- (C) 2014 - ntop.org
--

-- This file contains the description of all functions
-- used to trigger host alerts


function bytes(old, new)   return((new["sent"]["bytes"]+new["rcvd"]["bytes"])-(old["sent"]["bytes"]+old["rcvd"]["bytes"]))         end
function packets(old, new) return((new["sent"]["packets"]+new["rcvd"]["packets"])-(old["sent"]["packets"]+old["rcvd"]["packets"])) end


alert_functions_description = {
   ["bytes"]   = "Difference in bytes (sent + received)",
   ["packets"] = "Difference in packets (sent + received)"
}
