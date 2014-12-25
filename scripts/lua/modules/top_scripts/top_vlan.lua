--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "top_talkers"

local top_vlan_intf = {}

local function getTopVLAN(ifid, ifname)
  return getActualTopGroups(ifid, ifname, 10, true, false, "vlan", true)
end

local function getTopVlanClean(ifid, ifname, param)
  top = getActualTopGroups(ifid, ifname, 10, true, false, "vlan", false)
  section_beginning = string.find(top, '%[')
  if (section_beginning == nil) then
    return("[ ]\n")
  else
    return(string.sub(top, section_beginning))
  end
end

local function getTopVLANFromJSON(content)
  correct_section_beginning = string.find(content, '"vlan"')
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

local function getHistoricalTopVLAN(ifid, ifname, epoch)
  if (epoch == nil) then
    return("[ ]\n")
  end
  return getTopVLANFromJSON(ntop.getSampling(ifid, tonumber(epoch)))
end

top_vlan_intf.name = "VLANs"
top_vlan_intf.infoScript = "hosts_stats.lua"
top_vlan_intf.key = "vlan"
top_vlan_intf.getTop = getTopVLAN
top_vlan_intf.getTopClean = getTopVLANClean
top_vlan_intf.getTopFromJSON = getTopVLANFromJSON
top_vlan_intf.getHistoricalTop = getHistoricalTopVLAN
top_vlan_intf.numLevels = 1

return top_vlan_intf
