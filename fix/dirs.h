/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2016, WWIV Software Services             */
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
#ifndef __INCLUDED_DIRS_H__
#define __INCLUDED_DIRS_H__

#include "fix.h"

namespace wwiv {
namespace fix {

class FixDirectoriesCommand : public BaseCommand {
public:
    FixDirectoriesCommand(FixConfiguration* config) : BaseCommand(config) {}
    virtual ~FixDirectoriesCommand() {}

    virtual int Execute() override;;
};

}  // namespace fix
}  // namespace wwiv

#endif  // __INCLUDED_DIRS_H__ 
