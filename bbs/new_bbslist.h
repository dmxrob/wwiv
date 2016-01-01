/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*                Copyright (C)2014-2016 WWIV Software Services           */
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
#ifndef __INCLUDED_NEW_BBSLIST_H__
#define __INCLUDED_NEW_BBSLIST_H__

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace wwiv {
namespace bbslist {

enum class ConnectionType { TELNET, SSH, MODEM };

struct BbsListEntry {
  int id;
  std::string name;
  std::string software;
  std::string sysop_name;
  std::string location;
  std::map<ConnectionType, std::string> addresses;
};

bool LoadFromJSON(const std::string& dir, const std::string& filename,
                  std::vector<std::unique_ptr<BbsListEntry>>* entries);

bool SaveToJSON(const std::string& dir, const std::string& filename, 
                const std::vector<std::unique_ptr<BbsListEntry>>& entries);

void NewBBSList();

}  // bbslist
}  // wwiv

#endif  // __INCLUDED_NEW_BBSLIST_H__
