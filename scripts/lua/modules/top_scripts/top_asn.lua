--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "top_talkers"

local top_asn_intf = {}

if (ntop.isPro()) then
  package.path = dirs.installdir .. "/pro/scripts/lua/modules/top_scripts/?.lua;" .. package.path
  local new = require("top_aggregate")
  -- Add pro methods to local method table
  for k,v in pairs(new) do
    top_asn_intf[k] = v
  end
end

local function getTopAS(ifid, ifname)
  return getActualTopGroups(ifid, ifname, 10, true, false, nil, nil, "asn", true)
end

local function getTopASClean(ifid, ifname, param)
  top = getActualTopGroups(ifid, ifname, 10, true, false, nil, nil, "asn", false)
  section_beginning = string.find(top, '%[')
  if (section_beginning == nil) then
    return("[ ]\n")
  else
    return(string.sub(top, section_beginning))
  end
end

local function getTopASFromJSON(content)
  correct_section_beginning = string.find(content, '"'..top_asn_intf.JSONkey..'"')
  if (correct_section_beginning == nil) then
    return("[ ]\n")
  else
    correct_section = string.sub(content, correct_section_beginning)
    sbeginning,send = string.find(correct_section, '%[.-%],')
    if (sbeginning == nil) then
      sbeginning,send = string.find(correct_section, '%[.-%]%s*}')
      if (sbeginning == nil) then
        elements = "[ ]\n"
      else
        elements = string.sub(correct_section, sbeginning, send-1)
      end
    else
      elements = string.sub(correct_section, sbeginning, send-1)
    end
    return(elements)
  end
end

local function getHistoricalTopAS(ifid, ifname, epoch)
  if (epoch == nil) then
    return("[ ]\n")
  end
  return getTopASFromJSON(ntop.getMinuteSampling(ifid, tonumber(epoch)))
end

top_asn_intf.name = "ASN"
top_asn_intf.infoScript = "hosts_stats.lua"
top_asn_intf.key = "asn"
top_asn_intf.JSONkey = "asn"
top_asn_intf.getTop = getTopAS
top_asn_intf.getTopClean = getTopASClean
top_asn_intf.getTopFromJSON = getTopASFromJSON
top_asn_intf.getHistoricalTop = getHistoricalTopAS
top_asn_intf.numLevels = 1

return top_asn_intf
