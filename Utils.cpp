/*
 *
 * (C) 2013-14 - ntop.org
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "ntop_includes.h"

/* ****************************************************** */

char* Utils::jsonLabel(int label, const char *label_str,char *buf, u_int buf_len){

  if(ntop->getPrefs()->get_json_symbolic_labels()) {
    snprintf(buf, buf_len, "%s", label_str);
  }else
    snprintf(buf, buf_len, "%d", label);

  return(buf);

}


char* Utils::formatTraffic(float numBits, bool bits, char *buf, u_int buf_len) {
  char unit;

  if(bits)
    unit = 'b';
  else
    unit = 'B';

  if(numBits < 1024) {
    snprintf(buf, buf_len, "%lu %c", (unsigned long)numBits, unit);
  } else if(numBits < 1048576) {
    snprintf(buf, buf_len, "%.2f K%c", (float)(numBits)/1024, unit);
  } else {
    float tmpMBits = ((float)numBits)/1048576;

    if(tmpMBits < 1024) {
      snprintf(buf, buf_len, "%.2f M%c", tmpMBits, unit);
    } else {
      tmpMBits /= 1024;

      if(tmpMBits < 1024) {
	snprintf(buf, buf_len, "%.2f G%c", tmpMBits, unit);
      } else {
	snprintf(buf, buf_len, "%.2f T%c", (float)(tmpMBits)/1024, unit);
      }
    }
  }

  return(buf);
}

/* ****************************************************** */

char* Utils::formatPackets(float numPkts, char *buf, u_int buf_len) {
  if(numPkts < 1000) {
    snprintf(buf, buf_len, "%.2f", numPkts);
  } else if(numPkts < 1000000) {
    snprintf(buf, buf_len, "%.2f K", numPkts/(float)1000);
  } else {
    numPkts /= 1000000;
    snprintf(buf, buf_len, "%.2f M", numPkts);
  }

  return(buf);
}

/* ****************************************************** */

char* Utils::l4proto2name(u_int8_t proto) {
  static char proto_string[8];

  switch(proto) {
  case 0:   return((char*)"IP");
  case 1:   return((char*)"ICMP");
  case 2:   return((char*)"IGMP");
  case 6:   return((char*)"TCP");
  case 17:  return((char*)"UDP");
  case 47:  return((char*)"GRE");
  case 50:  return((char*)"ESP");
  case 58:  return((char*)"IPv6-ICMP");
  case 89:  return((char*)"OSPF");
  case 103: return((char*)"PIM");
  case 112: return((char*)"VRRP");

  default:
    snprintf(proto_string, sizeof(proto_string), "%u", proto);
    return(proto_string);
  }
}

/* ****************************************************** */

bool Utils::isIPAddress(char *ip) {
  struct in_addr addr4;
  struct in6_addr addr6;

  if(strchr(ip, ':') != NULL) { /* IPv6 */
    if(inet_pton(AF_INET6, ip, &addr6) == 1)
      return(true);
  } else {
    if(inet_pton(AF_INET, ip, &addr4) == 1)
      return(true);
  }

  return(false);
}

/* ****************************************************** */

void Utils::setThreadAffinity(pthread_t thread, int core_id) {
#ifdef linux
  u_int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
  u_long core = core_id % num_cores;
  cpu_set_t cpu_set;

  if(num_cores > 1) {
    CPU_ZERO(&cpu_set);
    CPU_SET(core, &cpu_set);
    pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpu_set);
  }
#endif
}

/* ****************************************************** */

char *Utils::trim(char *s) {
  char *end;

  while(isspace(s[0])) s++;
  if(s[0] == 0) return s;

  end = &s[strlen(s) - 1];
  while(end > s && isspace(end[0])) end--;
  end[1] = 0;

  return s;
}

/* ****************************************************** */

u_int32_t Utils::hashString(char *key) {
  u_int32_t hash = 0, len = strlen(key);

  for(u_int32_t i=0; i<len; i++)
    hash += ((u_int32_t)key[i])*i;

  return(hash);
}

/* ****************************************************** */

float Utils::timeval2ms(struct timeval *tv) {
  return((float)tv->tv_sec*1000+(float)tv->tv_usec/1000);
}

/* ****************************************************** */

bool Utils::mkdir_tree(char *path) {
  int permission = 0777, rc;
  struct stat s;

  ntop->fixPath(path);

  if(stat(path, &s) != 0) {
    /* Start at 1 to skip the root */
    for(int i=1; path[i] != '\0'; i++)
      if(path[i] == CONST_PATH_SEP) {
#ifdef WIN32
	/* Do not create devices directory */
	if((i > 1) && (path[i-1] == ':')) continue;
#endif

	path[i] = '\0';
	rc = ntop_mkdir(path, permission);
	path[i] = CONST_PATH_SEP;
      }

    rc = ntop_mkdir(path, permission);

    return(rc == 0 ? true : false);
  } else
    return(true); /* Already existing */
}

/* **************************************************** */

const char* Utils::trend2str(ValueTrend t) {
  switch(t) {
  case trend_up:
    return("Up");
    break;

  case trend_down:
    return("Down");
    break;

  case trend_stable:
    return("Stable");
    break;

  default:
  case trend_unknown:
    return("Unknown");
    break;
  }
}

/* **************************************************** */

void Utils::dropPrivileges() {
#ifndef WIN32
  struct passwd *pw = NULL;
  const char *username;

  if(getgid() && getuid()) {
    ntop->getTrace()->traceEvent(TRACE_NORMAL, "Privileges are not dropped as we're not superuser");
    return;
  }

  username = ntop->getPrefs()->get_user();
  pw = getpwnam(username);

  if(pw == NULL) {
    username = "anonymous";
    pw = getpwnam(username);
  }

  if(pw != NULL) {
    /* Drop privileges */
    if((setgid(pw->pw_gid) != 0) || (setuid(pw->pw_uid) != 0)) {
      ntop->getTrace()->traceEvent(TRACE_WARNING, "Unable to drop privileges [%s]",
				   strerror(errno));
    } else
      ntop->getTrace()->traceEvent(TRACE_NORMAL, "User changed to %s", username);
  } else {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "Unable to locate user %s", username);
  }

  umask(0);
#endif
}


/* **************************************************** */

/* http://www.adp-gmbh.ch/cpp/common/base64.html */
static const std::string base64_chars = 
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  "abcdefghijklmnopqrstuvwxyz"
  "0123456789+/";


static inline bool is_base64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

/* **************************************************** */

std::string Utils::base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
  std::string ret;
  int i = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--) {
    char_array_3[i++] = *(bytes_to_encode++);
    if(i == 3) {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if(i) {
      for(int j = i; j < 3; j++)
	char_array_3[j] = '\0';

      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(int j = 0; (j < i + 1); j++)
	ret += base64_chars[char_array_4[j]];

      while((i++ < 3))
	ret += '=';

    }

  return ret;
}

/* **************************************************** */

std::string Utils::base64_decode(std::string const& encoded_string) {
  int in_len = encoded_string.size();
  int i = 0, in_ = 0;
  unsigned char char_array_4[4], char_array_3[3];
  std::string ret;

  while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
    char_array_4[i++] = encoded_string[in_]; in_++;

    if(i == 4) {
      for(i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for(i = 0; (i < 3); i++)
        ret += char_array_3[i];
      i = 0;
    }
  }

  if(i) {
    int j;

    for(j = i; j <4; j++)
      char_array_4[j] = 0;

    for(j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for(j = 0; (j < i - 1); j++) ret += char_array_3[j];
  }

  return ret;
}

/* **************************************************** */

bool Utils::dumpHostToDB(IpAddress *host, LocationPolicy policy) {
  bool do_dump = false;
  int16_t network_id;

  switch(policy) {
  case location_local_only:
    if(host->isLocalHost(&network_id)) do_dump = true;
    break;
  case location_remote_only:
    if(!host->isLocalHost(&network_id)) do_dump = true;
    break;
  case location_all:
    do_dump = true;
    break;
  case location_none:
    do_dump = false;
    break;
  }

  return(do_dump);
}

/* *************************************** */

double Utils::pearsonValueCorrelation(u_int8_t *x, u_int8_t *y) {
  double ex = 0, ey = 0, sxx = 0, syy = 0, sxy = 0, tiny_value = 1e-2;

  for(size_t i = 0; i < CONST_MAX_ACTIVITY_DURATION; i++) {
    /* Find the means */
    ex += x[i], ey += y[i];
  }

  ex /= CONST_MAX_ACTIVITY_DURATION, ey /= CONST_MAX_ACTIVITY_DURATION;

  for(size_t i = 0; i < CONST_MAX_ACTIVITY_DURATION; i++) {
    /* Compute the correlation coefficient */
    double xt = x[i] - ex, yt = y[i] - ey;

    sxx += xt * xt, syy += yt * yt, sxy += xt * yt;
  }

  return (sxy/(sqrt(sxx*syy)+tiny_value));
}

/* *************************************** */
/* XXX: it assumes that the vectors are bitmaps */
double Utils::JaccardSimilarity(u_int8_t *x, u_int8_t *y) { 
  size_t inter_card = 0, union_card = 0;

  for(size_t i = 0; i < CONST_MAX_ACTIVITY_DURATION; i++) {
    union_card += x[i] | y[i];
    inter_card += x[i] & y[i];
  }

  if(union_card == 0)
    return(1e-2);

  return ((double)inter_card/union_card);
}

/* *************************************** */

#ifdef WIN32
const char *strcasestr(const char *haystack, const char *needle) {
  int i=-1;

  while (haystack[++i] != '\0') {
    if(tolower(haystack[i]) == tolower(needle[0])) {
      int j=i, k=0, match=0;
      while (tolower(haystack[++j]) == tolower(needle[++k])) {
	match=1;
	// Catch case when they match at the end
	//printf("j:%d, k:%d\n",j,k);
	if(haystack[j] == '\0' && needle[k] == '\0') {
	  //printf("Mj:%d, k:%d\n",j,k);
	  return &haystack[i];
	}
      }
      // Catch normal case
      if(match && needle[k] == '\0'){
	// printf("Norm j:%d, k:%d\n",j,k);
	return &haystack[i];
      }
    }
  }

  return NULL;
}
#endif

/* **************************************************** */

u_int8_t Utils::ifname2id(const char *name) {
  char rsp[256];

  if(name == NULL) return(DUMMY_IFACE_ID);

  if(ntop->getRedis()->hashGet((char*)CONST_IFACE_ID_PREFS, (char*)name, rsp, sizeof(rsp)) == 0) {
    /* Found */
    return(atoi(rsp));
  } else {
    for(u_int8_t idx=0; idx<255; idx++) {
      char key[256];

      snprintf(key, sizeof(key), "%u", idx);
      if(ntop->getRedis()->hashGet((char*)CONST_IFACE_ID_PREFS, key, rsp, sizeof(rsp)) < 0) {
	/* Free Id */
	
	snprintf(rsp, sizeof(rsp), "%u", idx);
	ntop->getRedis()->hashSet((char*)CONST_IFACE_ID_PREFS, (char*)name, rsp);
	ntop->getRedis()->hashSet((char*)CONST_IFACE_ID_PREFS, rsp, (char*)name);
	return(idx);
      }
    }
  }
  
  return(DUMMY_IFACE_ID); /* This can't happen, hopefully */
}

/* **************************************************** */

/* http://en.wikipedia.org/wiki/Hostname */

char* Utils::sanitizeHostName(char *str) {
  int i;

  for(i=0; str[i] != '\0'; i++) {
    if(((str[i] >= 'a') && (str[i] <= 'z'))
       || ((str[i] >= 'A') && (str[i] <= 'Z'))
       || ((str[i] >= '0') && (str[i] <= '9'))
       || (str[i] == '-')
       || (str[i] == '_')
       || (str[i] == '.')
       || (str[i] == ':') /* Used in HTTP host:port */
       || (str[i] == '@') /* Used by DNS but not a valid char */)
      ;
    else if(str[i] == '_') {
      str[i] = '\0';
      break;
    } else
      str[i] = '_';
  }

  return(str);
}

/* **************************************************** */

char* Utils::urlDecode(const char *src, char *dst, u_int dst_len) {
  char *ret = dst;
  u_int i = 0;

  dst_len--; /* Leave room for \0 */
  dst[dst_len] = 0;

  while((*src) && (i < dst_len)) {
    char a, b;

    if((*src == '%') &&
       ((a = src[1]) && (b = src[2]))
       && (isxdigit(a) && isxdigit(b))) {
      if(a >= 'a') a -= 'a'-'A';
      if(a >= 'A') a -=('A' - 10);
      else         a -= '0';

      if(b >= 'a') b -= 'a'-'A';
      if(b >= 'A') b -=('A' - 10);
      else         b -= '0';

      *dst++ = 16*a+b;

      src += 3;
    } else
      *dst++ = *src++;

    i++;
  }

  *dst++ = '\0';
  return(ret);
}

/* **************************************************** */

/**
 * @brief Check if the current user is an administrator
 *
 * @param vm   The lua state.
 * @return true if the current user is an administrator, false otherwise.
 */
bool Utils::isUserAdministrator(lua_State* vm) {
  char *username;
  char key[64], val[64];

  lua_getglobal(vm, "user");
  if((username = (char*)lua_touserdata(vm, lua_gettop(vm))) == NULL) {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "%s(%s): NO", __FUNCTION__, "???");
    return(false); /* Unknown */
  }

  snprintf(key, sizeof(key), CONST_STR_USER_GROUP, username);
  if(ntop->getRedis()->get(key, val, sizeof(val)) >= 0) {
    return(strcmp(val, CONST_ADMINISTRATOR_USER) ? false : true);
  } else {
    ntop->getTrace()->traceEvent(TRACE_WARNING, "%s(%s): NO", __FUNCTION__, username);
    return(false); /* Unknown */
  }
}
