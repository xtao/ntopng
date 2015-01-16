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
  return getCurrentTopGroups(ifid, ifname, 10, true, false,
                             nil, nil, top_asn_intf.key, true)
end

local function getTopASBy(ifid, ifname, filter_col, filter_val)
  return getCurrentTopGroups(ifid, ifname, 10, true, false,
                             filter_col, filter_val, top_asn_intf.key, true)
end

local function getTopASClean(ifid, ifname, param)
  top = getCurrentTopGroups(ifid, ifname, 10, true, false,
                            nil, nil, top_asn_intf.key, false)
  section_beginning = string.find(top, '%[')
  if (section_beginning == nil) then
    return("[ ]\n")
  else
    return(string.sub(top, section_beginning))
  end
end

local function topASSectionInTableOP(tblarray, arithOp)
  local ret = {}
  local num_glob = 1

  for _,tbl in pairs(tblarray) do
    for _,record in pairs(tbl) do
      local found = false
      for _,el in pairs(ret) do
        if (found == false and el["label"] == record["label"]) then
          el["value"] = arithOp(el["value"], record["value"])
          found = true
        end
      end
      if (found == false) then
        ret[num_glob] = record
        num_glob = num_glob + 1
      end
    end
  end

  return ret
end

local function printTopASTable(tbl)
  local rsp = ""

  local keys = getKeys(tbl, "value")
  for tv,ti in pairsByKeys(keys, rev) do
    rv = tbl[ti]
    rsp = rsp.."{ "
    for k,v in pairs(rv) do
      rsp = rsp..'"'..k..'": '
      if (k == "value") then
        rsp = rsp..tostring(v)
      else
        rsp = rsp..'"'..v..'"'
      end
      rsp = rsp..", "
    end
    rsp = string.sub(rsp, 1, -3)
    rsp = rsp.."},\n"
  end

  rsp = string.sub(rsp, 1, -3)

  return rsp
end

local function getTopASFromJSON(content, add_vlan)
  if(content == nil) then return("[ ]\n") end
  local table = parseJSON(content)
  if (table == nil or table["vlan"] == nil) then return "[ ]\n" end

  local records = 0
  local elements = "[\n"

  -- For each VLAN, get ASN and concatenate them
  for i,vlan in pairs(table["vlan"]) do
      local vlanname = vlan["name"]
      local vlanid = vlan["label"]
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
        if (add_vlan ~= nil) then
          elements = elements..'"vlan": "'..vlanid..'", '
          elements = elements..'"vlanm": "'..vlanname..'", '
        end
        elements = string.sub(elements, 1, -3)
        elements = elements.." },\n"
        records = records + 1
      end
  end
  if (records > 0) then
    elements = string.sub(elements, 1, -3) -- remove comma
  end
  elements = elements.."\n]"
  return elements
end

local function getHistoricalTopAS(ifid, ifname, epoch, add_vlan)
  if (epoch == nil) then
    return("[ ]\n")
  end
  return getTopASFromJSON(ntop.getMinuteSampling(ifid, tonumber(epoch)), add_vlan)
end

top_asn_intf.name = "ASN"
top_asn_intf.infoScript = "hosts_stats.lua"
top_asn_intf.key = "asn"
top_asn_intf.JSONkey = "asn"
top_asn_intf.getTop = getTopAS
top_asn_intf.getTopBy = getTopASBy
top_asn_intf.getTopClean = getTopASClean
top_asn_intf.getTopFromJSON = getTopASFromJSON
top_asn_intf.printTopTable = printTopASTable
top_asn_intf.getHistoricalTop = getHistoricalTopAS
top_asn_intf.topSectionInTableOp = topASSectionInTableOP
top_asn_intf.numLevels = 1

return top_asn_intf
