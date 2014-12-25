--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "top_talkers"

local top_talkers_intf = {}

local function getTopTalkers(ifid, ifname)
  return getActualTopTalkers(ifid, ifname, true)
end

local function getTopTalkersClean(ifid, ifname, param)
  top = getActualTopTalkers(ifid, ifname, false, param)
  section_beginning = string.find(top, '%[')
  if (section_beginning == nil) then
    return("[ ]\n")
  else
    return(string.sub(top, section_beginning))
  end
end

local function getTopTalkersFromJSON(content)
  correct_section_beginning = string.find(content, '"hosts"')
  if (correct_section_beginning == nil) then
    return("[ ]\n")
  else
    correct_section = string.sub(content, correct_section_beginning)
    sbeginning,send = string.find(correct_section,
                                  '%[%s*{%s*"senders":%s*%[.*%],%s*"receivers":%s*%[.*%]%s*}%s*%],')
    if (sbeginning == nil) then
      sbeginning,send = string.find(correct_section,
                                  '%[%s*{%s*"senders":%s*%[.*%],%s*"receivers":%s*%[.*%]%s*}%s*%]%s*}')
      if (sbeginning == nil) then
        elements = "[ ]\n"
      else
        elements = string.sub(correct_section, sbeginning+1, send-3)
      end
    else
      elements = string.sub(correct_section, sbeginning+1, send-3)
    end
    return(elements)
  end
end

local function getHistoricalTopTalkers(ifid, ifname, epoch)
  if (epoch == nil) then
    return("[ ]\n")
  end
  return getTopTalkersFromJSON(ntop.getSampling(ifid, tonumber(epoch)))
end

top_talkers_intf.name = "Top Talkers"
top_talkers_intf.infoScript = "host_details.lua"
top_talkers_intf.key = "host"
top_talkers_intf.getTop = getTopTalkers
top_talkers_intf.getTopClean = getTopTalkersClean
top_talkers_intf.getTopFromJSON = getTopTalkersFromJSON
top_talkers_intf.getHistoricalTop = getHistoricalTopTalkers
top_talkers_intf.numLevels = 2

return top_talkers_intf
