--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "top_talkers"
require "json"

local top_talkers_intf = {}

if (ntop.isPro()) then
  package.path = dirs.installdir .. "/pro/scripts/lua/modules/top_scripts/?.lua;" .. package.path
  local new = require("top_aggregate")
  -- Add pro methods to local method table
  for k,v in pairs(new) do
    top_talkers_intf[k] = v
  end
end

local function getTopTalkers(ifid, ifname)
  return getActualTopTalkers(ifid, ifname, nil, nil, true)
end

local function getTopTalkersBy(ifid, ifname, filter_col, filter_val)
  return getActualTopTalkers(ifid, ifname, filter_col, filter_val, true)
end

local function getTopTalkersClean(ifid, ifname, param)
  top = getActualTopTalkers(ifid, ifname, nil, nil, false, param)
  section_beginning = string.find(top, '%[')
  if (section_beginning == nil) then
    return("[ ]\n")
  else
    return(string.sub(top, section_beginning))
  end
end

local function getTopTalkersFromJSONDirection(table, wantedDir)
  local elements = ""

  -- For each VLAN, get hosts and concatenate them
  for i,vlan in pairs(table["vlan"]) do
      -- XXX hosts is an array of (senders, receivers) pairs?
      for i2,hostpair in pairs(vlan[top_talkers_intf.JSONkey]) do
        -- hostpair is { "senders": [...], "receivers": [...] }
        for k2,direction in pairs(hostpair) do
          -- direction is "senders": [...] or "receivers": [...]
          if (k2 ~= wantedDir) then goto continue end
          -- scan hosts
          for i2,host in pairs(direction) do
            -- host is { "label": ..., "value": ..., "url": ... }
            elements = elements.."{ "
            for k3,v3 in pairs(host) do
              elements = elements..'"'..k3..'": '
              if (k3 == "value") then
                elements = elements..tostring(v3)
              else
                elements = elements..'"'..v3..'"'
              end
              elements = elements..", "
            end
            elements = string.sub(elements, 1, -3)
            elements = elements.." },\n"
          end
          ::continue::
        end
      end
  end

  return elements
end

local function getTopTalkersFromJSON(content)
  if(content == nil) then return("[ ]\n") end
  local table = parseJSON(content)
  if (table == nil or table["vlan"] == nil) then return "[ ]\n" end

  local elements = "{\n"
  elements = elements..'"senders": [\n'
  elements = elements..getTopTalkersFromJSONDirection(table, "senders")
  elements = string.sub(elements, 1, -3) --remove comma
  elements = elements.."],\n"
  elements = elements..'"receivers": [\n'
  elements = elements..getTopTalkersFromJSONDirection(table, "receivers")
  elements = string.sub(elements, 1, -3) --remove comma
  elements = elements.."]\n"
  elements = elements.."}\n"

  return elements
end

local function getHistoricalTopTalkers(ifid, ifname, epoch)
  if (epoch == nil) then
    return("[ ]\n")
  end
  return getTopTalkersFromJSON(ntop.getMinuteSampling(ifid, tonumber(epoch)))
end

top_talkers_intf.name = "Top Talkers"
top_talkers_intf.infoScript = "host_details.lua"
top_talkers_intf.key = "host"
top_talkers_intf.JSONkey = "hosts"
top_talkers_intf.getTop = getTopTalkers
top_talkers_intf.getTopBy = getTopTalkersBy
top_talkers_intf.getTopClean = getTopTalkersClean
top_talkers_intf.getTopFromJSON = getTopTalkersFromJSON
top_talkers_intf.getHistoricalTop = getHistoricalTopTalkers
top_talkers_intf.numLevels = 2

return top_talkers_intf
