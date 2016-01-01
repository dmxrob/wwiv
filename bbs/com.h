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
/**************************************************************************/
#ifndef __INCLUDED_BBS_COM_H__
#define __INCLUDED_BBS_COM_H__

void RestoreCurrentLine(const char *cl, const char *atr, const char *xl, const char *cc);
void dump();
bool CheckForHangup();
void makeansi(int attr, char *out_buffer, bool forceit);
void resetnsp();
char getkey();
bool yesno();
bool noyes();
char ynq();
char onek(const char *allowable_chars, bool auto_mpl = false);

#endif  // __INCLUDED_BBS_COM_H__
