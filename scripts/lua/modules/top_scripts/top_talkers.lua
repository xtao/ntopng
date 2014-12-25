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

top_talkers_intf.getTop = getTopTalkers
top_talkers_intf.getTopFromJSON = getTopTalkersFromJSON
top_talkers_intf.getHistoricalTop = getHistoricalTopTalkers

return top_talkers_intf
