--
-- (C) 2013-14 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path

require "lua_utils"
require "top_talkers"

sendHTTPHeader('text/html; charset=iso-8859-1')

top_talkers = getTopTalkers(getInterfaceId(ifname), ifname, _GET["epoch"])
correct_section_beginning = string.find(top_talkers, '"asn"')
if (correct_section_beginning == nil) then
  print(top_talkers)
else
  correct_section = string.sub(top_talkers, correct_section_beginning)
  elements_beginning = string.find(correct_section, "\n")
  elements = string.sub(correct_section, elements_beginning-1)
  correct_section_end = string.find(elements, '\n"')
  correct_section = string.sub(elements, 1, correct_section_end-2)
  print(correct_section)
end
