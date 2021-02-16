/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2021, WWIV Software Services             */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/*                                                                        */
/**************************************************************************/

#include "bbs/acs.h"
#include "bbs/bbs.h"
#include "bbs/bbsutl.h"
#include "bbs/bbsutl1.h"
#include "bbs/finduser.h"
#include "bbs/utility.h"
#include "common/com.h"
#include "common/input.h"
#include "common/pause.h"
#include "core/datafile.h"
#include "core/stl.h"
#include "core/strings.h"
#include "fmt/printf.h"
#include "sdk/chains.h"
#include "sdk/names.h"
#include "sdk/user.h"
#include "sdk/usermanager.h"

using std::string;
using namespace wwiv::bbs;
using namespace wwiv::core;
using namespace wwiv::sdk;
using namespace wwiv::stl;
using namespace wwiv::strings;

void delete_chain(size_t chain_num);

static string chaindata(int chain_num) {
  const auto& c = a()->chains->at(chain_num);
  return fmt::sprintf("|#2%2d |#1%-32.32s  |#5%-35.35s", chain_num,
                      stripcolors(c.description), c.filename);
}

static void showchains() {
  bout.cls();
  auto abort = false;
  bout.bpla("|#2NN Description                       Path Name", &abort);
  bout.bpla("|#7== --------------------------------  =================================== ", &abort);
  for (auto chain_num = 0; chain_num < size_int(a()->chains->chains()) && !abort; chain_num++) {
    const auto s = chaindata(chain_num);
    bout.bpla(s, &abort);
  }
}

void ShowChainCommandLineHelp() {
  bout << "|#2  Macro   Value\r\n";
  bout << "|#7 ======== =======================================\r\n";
  bout << "|#1   %     |#9 A single \'%\' Character\r\n";
  bout << "|#1   %1    |#9 CHAIN.TXT full pathname (legacy parameter)\r\n";
  bout << "|#1   %A    |#9 CALLINFO.BBS full pathname \r\n";
  bout << "|#1   %C    |#9 CHAIN.TXT full pathname \r\n";
  bout << "|#1   %D    |#9 DORIFOx.DEF full pathname \r\n";
  bout << "|#1   %E    |#9 DOOR32.SYS full pathname \r\n";
  bout << "|#1   %H    |#9 Socket Handle \r\n";
  bout << "|#1   %I    |#9 TEMP directory for the instance \r\n";
  bout << "|#1   %K    |#9 GFiles Comment File For Archives\r\n";
  bout << "|#1   %M    |#9 Modem Baud Rate\r\n";
  bout << "|#1   %N    |#9 Node (Instance) number\r\n";
  bout << "|#1   %O    |#9 PCBOARD.SYS full pathname\r\n";
  bout << "|#1   %P    |#9 ComPort Number\r\n";
  bout << "|#1   %R    |#9 DOOR.SYS Full Pathname\r\n";
  bout << "|#1   %S    |#9 Com Port Baud Rate\r\n";
  bout << "|#1   %T    |#9 Minutes Remaining\r\n";
  bout << "|#1   %U    |#9 Users Handle (primary name)\r\n";
  bout.nl();
}

static std::string YesNoStringList(bool b, const std::string& yes, const std::string& no) {
  return b ? yes : no;
}

static void modify_chain_sponsors(int chain_num, chain_t& c) {
  if (!a()->HasConfigFlag(OP_FLAGS_CHAIN_REG)) {
    return;
  }
  bool done = false;
  do {
    bout.cls();
    bout.litebar(fmt::format("Editing Chain #{}", chain_num));
    const auto& r = c.regby;
    User regUser;
    if (!r.empty()) {
      auto it = r.begin();
      auto first = *r.begin();
      a()->users()->readuser(&regUser, *it++);
      bout << "|#9L) Registered by: |#2" << a()->names()->UserName(first) << wwiv::endl;
      for (; it != std::end(r); ++it) {
        const auto rbc = *it;
        if (rbc != 0) {
          if (a()->users()->readuser(&regUser, rbc)) {
            bout << string(18, ' ') << "|#2" << a()->names()->UserName(rbc) << wwiv::endl;
          }
        }
      }
    } else {
      bout << "|#9L) Registered by: |#2AVAILABLE" << wwiv::endl;
    }
    bout.nl();
    bout << "|#9(A)dd, (R)emove, (Q)uit: Which (A,R,Q) ? ";
    auto ch = onek("QARQ", true); 
    switch (ch) {
    case 'Q':
      return;
    case 'A': {
      if (c.regby.size() > 5) {
        bout << "|#6Only 5 sponsors allowed." << wwiv::endl;
        bout.pausescr();
        break;
      }
      auto nn = bin.input(30, true);
      auto user_number = finduser1(nn);
      if (user_number > 0) {
        c.regby.insert(static_cast<int16_t>(user_number));
      }
    } break;
    case 'R': {
      auto nn = bin.input(30, true);
      auto user_number = finduser1(nn);
      if (user_number > 0) {
        c.regby.erase(static_cast<int16_t>(user_number));
      }
    } break;
    }
  } while (!done && !a()->sess().hangup());
}

static std::string chain_exec_mode_to_string(const chain_exec_mode_t& t) {
  std::vector<string> names{"|#2DOOR32 (Socket)", "|#1Emulate DOS Interrupts", "|#5SyncFoss", "|#3STDIO", "|#5NetFoss"};
  try {
    return names.at(static_cast<size_t>(t));
  } catch (std::out_of_range&) {
    return names.at(0);
  }
}

static void modify_chain(ssize_t chain_num) {
  auto c = a()->chains->at(chain_num);
  auto done = false;
  do {
    bout.cls();
    bout.litebar(fmt::format("Editing Chain #{}", chain_num));

    bout << "|#9A) Description  : |#2" << c.description << wwiv::endl;
    bout << "|#9B) Filename     : |#2" << c.filename << wwiv::endl;
    bout << "|#9C) ACS          : |#2" << c.acs << wwiv::endl;
    bout << "|#9D) ANSI         : |#2" << (c.ansi ? "|#2Required" : "|#5Optional") << wwiv::endl;
    bout << "|#9E) Exec Mode    : |#2" << chain_exec_mode_to_string(c.exec_mode) << wwiv::endl;
    const auto launch_from = YesNoStringList(c.dir == chain_exec_dir_t::temp, "Temp/Node Directory",
                                             "BBS Root Directory");
    if (c.exec_mode == chain_exec_mode_t::netfoss) {
      bout << "|#9F) Launch From  : |08Temp/Node Directory" << wwiv::endl;
    } else {
      bout << "|#9F) Launch From  : |#2" << launch_from << wwiv::endl;
    }
    bout << "|#9G) Local only   : |#2" << YesNoString(c.local_only) << wwiv::endl;
    bout << "|#9H) Multi user   : |#2" << YesNoString(c.multi_user) << wwiv::endl;
    bout << "|#9I) Usage        : |#2" << c.usage << wwiv::endl;
    std::string allowed = "QABCDEFGHIKL[]";
    if (a()->HasConfigFlag(OP_FLAGS_CHAIN_REG)) {
      const auto& r = c.regby;
      if (!r.empty()) {
        bout << "|#9J) Registered by: ";
        auto need_space = false;
        for (auto it = std::begin(r); it != std::end(r) && *it != 0; ++it) {
          User regUser;
          if (a()->users()->readuser(&regUser, *it)) {
            if (!need_space) {
              need_space = true;
            } else {
              bout << string(18, ' ');
            }
            bout << "|#2" << a()->names()->UserName(*it) << wwiv::endl;
          }
        }
      } else {
        bout << "|#9J) Registered by: |#2AVAILABLE" << wwiv::endl;
      }
#ifdef _WIN32
    if (c.exec_mode == chain_exec_mode_t::netfoss || c.exec_mode == chain_exec_mode_t::fossil) {
      bout << "|#9K) Local CP437  : |08Yes" << wwiv::endl;
    } else {
      bout << "|#9K) Local CP437  : |#2" << YesNoString(c.local_console_cp437) << wwiv::endl;
    }
#endif
      bout << "|#9L) Pause after  : |#2" << YesNoString(c.pause) << wwiv::endl;
      bout.nl();
      bout << "|#7(|#2Q|#7=|#1Quit|#7) Which (|#1A|#7-|#1JL|#7,|#1[|#7=|#1Prev|#7,|#1]|#7=|#1Next|#7) : ";
      allowed.push_back('J');
    } else {
      bout.nl();
      bout << "|#7(|#2Q|#7=|#1Quit|#7) Which (|#1A|#7-|#1I|#7,|#1K-L|#7,|#1[|#7=|#1Prev|#7,|#1]|#7=|#1Next|#7) : ";
    }
    const auto ch = onek(allowed, true);
    switch (ch) {
    case 'Q': {
      done = true;
    } break;
    case '[':
      a()->chains->at(chain_num) = c;
      if (--chain_num < 0) {
        chain_num = ssize(a()->chains->chains()) - 1;
      }
      c = a()->chains->at(chain_num);
      break;
    case ']':
      a()->chains->at(chain_num) = c;
      if (++chain_num >= ssize(a()->chains->chains())) {
        chain_num = 0;
      }
      c = a()->chains->at(chain_num);
      break;
    case 'A': {
      bout.nl();
      bout << "|#7New Description? ";
      auto descr = bin.input_text(c.description, 40);
      if (!descr.empty()) {
        c.description = descr;
      }
    } break;
    case 'B': {
      bout.cls();
      ShowChainCommandLineHelp();
      bout << "\r\n|#9Enter Command Line.\r\n|#7:";
      c.filename = bin.input_cmdline(c.filename, 79);
    } break;
    case 'C': {
      bout.nl();
      c.acs = input_acs(bin, bout, "New ACS?", c.acs, 78);
    } break;
    case 'D':
      c.ansi = !c.ansi;
      break;
    case 'E':
      ++c.exec_mode;
#ifdef _WIN32
      if (c.exec_mode == chain_exec_mode_t::stdio) {
        ++c.exec_mode;
      }
#else
      if (c.exec_mode == chain_exec_mode_t::dos) {
        c.exec_mode++;
      }
      if (c.exec_mode == chain_exec_mode_t::fossil) {
        c.exec_mode++;
      }
      if (c.exec_mode == chain_exec_mode_t::netfoss) {
        c.exec_mode++;
      }
#endif
      break;
    case 'F':
      if (c.exec_mode == chain_exec_mode_t::netfoss) {
        c.dir = chain_exec_dir_t::temp;
      } else {
        ++c.dir;
      }
      break;
    case 'G':
      c.local_only = !c.local_only;
      break;
    case 'H':
      c.multi_user = !c.multi_user;
      break;
    case 'I': {
      bout.nl();
      bout << "|#5Times Run : ";
      c.usage = bin.input_number(c.usage);
    } break;
    case 'J':
      if (!a()->HasConfigFlag(OP_FLAGS_CHAIN_REG)) {
        break;
      }
      modify_chain_sponsors(chain_num, c);
      break;
    case 'K': {
#ifdef _WIN32
      if (!(c.exec_mode == chain_exec_mode_t::netfoss || c.exec_mode == chain_exec_mode_t::fossil)) {
        c.local_console_cp437 = !c.local_console_cp437;
      }
#endif
    } break;
    case 'L': 
      c.pause = !c.pause;
      break;
    
    }
  } while (!done && !a()->sess().hangup());
  a()->chains->at(chain_num) = c;
}

static void insert_chain(size_t pos) {
  chain_t c{};
  c.description = "** NEW CHAIN **";
  c.filename = "REM";
  c.acs = "user.sl >= 10";
  c.exec_mode = chain_exec_mode_t::none;
  a()->chains->insert(pos, c);
  modify_chain(pos);
}

void delete_chain(size_t chain_num) {
  a()->chains->erase(chain_num);
}

void chainedit() {
  if (!ValidateSysopPassword()) {
    return;
  }
  showchains();
  auto done = false;
  do {
    bout.nl();
    bout << "|#7Chains: (D)elete, (I)nsert, (M)odify, (Q)uit, ? : ";
    const auto ch = onek("QDIM?");
    switch (ch) {
    case '?':
      showchains();
      break;
    case 'Q':
      done = true;
      break;
    case 'M': {
      bout.nl();
      bout << "|#2(Q=Quit) Chain number? ";
      auto r = bin.input_number_hotkey(0, {'Q'}, 0, size_int(a()->chains->chains()), false);
      if (r.key != 'Q' && r.num < ssize(a()->chains->chains())) {
        modify_chain(r.num);
      }
    } break;
    case 'I': {
      if (a()->chains->chains().size() < a()->max_chains) {
        bout.nl();
        bout << "|#2(Q=Quit) Insert before which chain ('$' for end) : ";
        auto r = bin.input_number_hotkey(0, {'$', 'Q'}, 0, size_int(a()->chains->chains()), false);
        if (r.key == 'Q') {
          break;
        }
        const auto chain = (r.key == '$') ? size_int(a()->chains->chains()) : r.num;
        if (chain >= 0 && chain <= ssize(a()->chains->chains())) {
          insert_chain(chain);
        }
      }
    } break;
    case 'D': {
      bout.nl();
      bout << "|#2(Q=Quit) Delete which chain? ";
      auto r = bin.input_number_hotkey(0, {'$', 'Q'}, 0, size_int(a()->chains->chains()), false);
      if (r.key == 'Q') {
        break;
      }
      if (r.num >= 0 && r.num < ssize(a()->chains->chains())) {
        bout.nl();
        bout << "|#5Delete " << a()->chains->at(r.num).description << "? ";
        if (bin.yesno()) {
          delete_chain(r.num);
        }
      }
    } break;
    }
  } while (!done && !a()->sess().hangup());

  a()->chains->Save();
}
