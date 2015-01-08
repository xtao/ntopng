--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "top_talkers"

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

local function getTopTalkersClean(ifid, ifname, param)
  top = getActualTopTalkers(ifid, ifname, nil, nil, false, param)
  section_beginning = string.find(top, '%[')
  if (section_beginning == nil) then
    return("[ ]\n")
  else
    return(string.sub(top, section_beginning))
  end
end

local function getTopTalkersFromJSON(content)
  local offset_b = 1
  local offset_e = -3

  if(content == nil) then return("[ ]\n") end

  correct_section_beginning = string.find(content, '"'..top_talkers_intf.JSONkey..'"')
  if (correct_section_beginning == nil) then
    return("[ ]\n")
  else
    correct_section = string.sub(content, correct_section_beginning)
    sbeginning,send = string.find(correct_section,
                                  '%[%s*{%s*"senders":%s*%[.*%],%s*"receivers":%s*%[.*%]%s*}%s*%],')
    if (sbeginning == nil) then
      sbeginning,send = string.find(correct_section,
                                    '%[%s*{%s*"receivers":%s*%[.*%],%s*"senders":%s*%[.*%]%s*}%s*%],')
      if (sbeginning == nil) then
        sbeginning,send = string.find(correct_section,
                                      '{%s*"receivers":%s*%[.*%],%s*"senders":%s*%[.*%]%s*},')
        if (sbeginning == nil) then
          sbeginning,send = string.find(correct_section,
                                        '{%s*"senders":%s*%[.*%],%s*"receivers":%s*%[.*%]%s*},')
          if (sbeginning == nil) then
            sbeginning,send = string.find(correct_section,
                                          '{%s*"senders":%s*%[.*%],%s*"receivers":%s*%[.*%]%s*}%s*}')
            if (sbeginning == nil) then
              sbeginning,send = string.find(correct_section,
                                            '{%s*"receivers":%s*%[.*%],%s*"senders":%s*%[.*%]%s*}%s*}')
              if (sbeginning == nil) then
                sbeginning,send = string.find(correct_section,
                                              '%[%s*{%s*"senders":%s*%[.*%],%s*"receivers":%s*%[.*%]%s*}%s*%]%s*}')
                if (sbeginning == nil) then
                  sbeginning,send = string.find(correct_section,
                                                '%[%s*{%s*"receivers":%s*%[.*%],%s*"senders":%s*%[.*%]%s*}%s*%]%s*}')
                  if (sbeginning == nil) then
                    return("[ ]\n")
                  end
                end
              end
            else -- {%s*"senders":%s*%[.*%],%s*"receivers":%s*%[.*%]%s*}%s*}
              offset_b = 0
              offset_e = -1
            end
          else -- {%s*"receivers":%s*%[.*%],%s*"senders":%s*%[.*%]%s*}%s*}
            offset_b = 0
            offset_e = -1
          end
        else -- {%s*"senders":%s*%[.*%],%s*"receivers":%s*%[.*%]%s*},
          offset_b = 0
          offset_e = -1
        end
      else -- {%s*"receivers":%s*%[.*%],%s*"senders":%s*%[.*%]%s*},
        offset_b = 0
        offset_e = -1
      end
    end
    elements = string.sub(correct_section, sbeginning+offset_b, send+offset_e)
    return(elements)
  end
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
top_talkers_intf.getTopClean = getTopTalkersClean
top_talkers_intf.getTopFromJSON = getTopTalkersFromJSON
top_talkers_intf.getHistoricalTop = getHistoricalTopTalkers
top_talkers_intf.numLevels = 2

return top_talkers_intf
