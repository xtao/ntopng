--
-- (C) 2013 - ntop.org
--

dirs = ntop.getDirs()
package.path = dirs.installdir .. "/scripts/lua/modules/?.lua;" .. package.path
if ( (dirs.scriptdir ~= nil) and (dirs.scriptdir ~= "")) then package.path = dirs.scriptdir .. "/lua/modules/?.lua;" .. package.path end
require "lua_utils"

sendHTTPHeader('text/html')

ntop.dumpFile(dirs.installdir .. "/httpdocs/inc/header.inc")

active_page = "admin"
dofile(dirs.installdir .. "/scripts/lua/inc/menu.lua")

ntop.loadPrefsDefaults()
prefs = ntop.getPrefs()

print [[
   <h2>Runtime Preferences</h2>
   <table class="table">
   ]]

-- ================================================================================
print('<tr><th colspan=2 class="info">Report Visualization</th></tr>')

toggleTableButton("Throughput Unit",
  "Select the throughput unit to be displayed in traffic reports.",
  "Bytes", "bps", "primary","Packets", "pps", "primary","toggle_thpt_content", "ntopng.prefs.thpt_content")

-- ================================================================================
print('<tr><th colspan=2 class="info">Traffic Storage (RRD)</th></tr>')

toggleTableButton("RRDs For Local Hosts",
  "Toggle the creation of RRDs for local hosts. Turn it off to save storage space.",
  "On", "1", "success", "Off", "0", "danger", "toggle_local", "ntopng.prefs.host_rrd_creation")

toggleTableButton("nDPI RRDs For Local Hosts",
  "Toggle the creation of nDPI RRDs for local hosts. Enable their creation allows you to keep application protocol statistics at the cost of using more disk space.",
  "On", "1", "success", "Off", "0", "danger", "toggle_local_ndpi", "ntopng.prefs.host_ndpi_rrd_creation")

-- ================================================================================
print('<tr><th colspan=2 class="info">Alerts</th></tr>')

toggleTableButton("Alerts On Syslog",
  "Toggle the dump of alerts on syslog.",
  "On", "1", "success", "Off", "0", "danger", "toggle_alert_syslog", "ntopng.prefs.alerts_syslog")

-- CONST_MAX_NEW_FLOWS_SECOND
prefsInputField("Host Flow Alert Threshold", "Max number of new flows/sec over which a host is considered a flooder. Default: 25.", "host_max_new_flows_sec_threshold", prefs.host_max_new_flows_sec_threshold)

-- CONST_MAX_NUM_SYN_PER_SECOND
prefsInputField("Host SYN Alert Threshold", "Max number of TCP SYN packets/sec over which a host is considered a flooder. Default: 10.", "host_max_num_syn_sec_threshold", prefs.host_max_num_syn_sec_threshold)

-- ================================================================================
print('<tr><th colspan=2 class="info">Data Purge</th></tr>')
prefsInputField("Local Host Idle Timeout", "Inactivity time after which a local host is considered idle (sec). Default: 300.", "local_host_max_idle", prefs.local_host_max_idle)
prefsInputField("Remote Host Idle Timeout", "Inactivity time after which a remote host is considered idle (sec). Default: 60.", "non_local_host_max_idle", prefs.non_local_host_max_idle)
prefsInputField("Flow Idle Timeout", "Inactivity time after which a flow is considered idle (sec). Default: 60.", "flow_max_idle", prefs.flow_max_idle)

-- ================================================================================

print [[
   </table>
]]

dofile(dirs.installdir .. "/scripts/lua/inc/footer.lua")
