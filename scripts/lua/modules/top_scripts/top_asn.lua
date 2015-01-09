--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "top_talkers"
require "json"

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
  return getActualTopGroups(ifid, ifname, 10, true, false,
                            nil, nil, top_asn_intf.key, true)
end

local function getTopASBy(ifid, ifname, filter_col, filter_val)
  return getActualTopGroups(ifid, ifname, 10, true, false,
                            filter_col, filter_val, top_asn_intf.key, true)
end

local function getTopASClean(ifid, ifname, param)
  top = getActualTopGroups(ifid, ifname, 10, true, false,
                           nil, nil, top_asn_intf.key, false)
  section_beginning = string.find(top, '%[')
  if (section_beginning == nil) then
    return("[ ]\n")
  else
    return(string.sub(top, section_beginning))
  end
end

local function getTopASFromJSON(content)
  if(content == nil) then return("[ ]\n") end
  local table = parseJSON(content)
  if (table == nil or table["vlan"] == nil) then return "[ ]\n" end
  local elements = "[\n"
  -- For each VLAN, get ASN and concatenate them
  for i,vlan in pairs(table["vlan"]) do
      for k2,v2 in pairs(vlan[top_asn_intf.JSONkey]) do
        -- scan ASNs
        elements = elements.."{ "
        for key,value in pairs(v2) do
          elements = elements..'"'..key..'": '
          if (key == "value") then
            elements = elements..tostring(value)
          else
            elements = elements..'"'..value..'"'
          end
          elements = elements..", "
        end
        elements = string.sub(elements, 1, -3)
        elements = elements.." },\n"
      end
  end
  elements = string.sub(elements, 1, -3) -- remove comma
  elements = elements.."\n]"
  return elements
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
top_asn_intf.getTopBy = getTopASBy
top_asn_intf.getTopClean = getTopASClean
top_asn_intf.getTopFromJSON = getTopASFromJSON
top_asn_intf.getHistoricalTop = getHistoricalTopAS
top_asn_intf.numLevels = 1

return top_asn_intf
